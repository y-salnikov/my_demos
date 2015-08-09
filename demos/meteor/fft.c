//
// Complex forward and inverse FFT routines
// Written by Alexey Lukin
//

#include <math.h>
#include "fft.h"

#define Round(x) ((int)(x+0.5))





// Complex FFT
// Input: N complex points in time domain
// Output: N complex points in frequency domain
void FFTC(Cmplx *X, int FFTSize)
{
	int	N, M, i, j, L, LE, LE2, ip, k, s;
	Cmplx t,z;
	RealData	UR, UI, SR, SI, TR, TI;

	N = FFTSize;
	M = Round(log(N)/log(2));
	// Bit-reverse
	i = 0;
	for (s=0;s<N-1;s++) {
		if (s<i) {
			t = *(X+i); *(X+i) = *(X+s); *(X+s) = t;
		}
		k = N >> 1;
		while (i&k) k >>= 1;
		i += k;
		k <<= 1;
		while (k<N) {
			i -= k;
			k <<= 1;
		}
	}
	// First pass
	for (i=0;i<N;i+=2) {
		t = *(X+i);
		X[i].Re = t.Re + X[i+1].Re;
		X[i].Im = t.Im + X[i+1].Im;
		X[i+1].Re = t.Re - X[i+1].Re;
		X[i+1].Im = t.Im - X[i+1].Im;
	}
	// Second pass
	for (i=0;i<N;i+=4) {
		t = X[i];
		X[i].Re = t.Re + X[i+2].Re;
		X[i].Im = t.Im + X[i+2].Im;
		X[i+2].Re = t.Re - X[i+2].Re;
		X[i+2].Im = t.Im - X[i+2].Im;
		t = X[i+1];
		z = X[i+3];
		X[i+1].Re = t.Re + z.Im;
		X[i+1].Im = t.Im - z.Re;
		X[i+3].Re = t.Re - z.Im;
		X[i+3].Im = t.Im + z.Re;
	}
	// Remaining passes
	for (L=3;L<=M;L++) {
		LE = 1 << L;
		LE2 = LE >> 1;
		UR = 1; UI = 0;
		SR = cos(M_PI/LE2);
		SI = -sin(M_PI/LE2);
		for (j=0;j<LE2;j++) {
			for (i=j;i<N;i+=LE) {
				ip = i + LE2;
				TR = X[ip].Re*UR - X[ip].Im*UI;
				TI = X[ip].Re*UI + X[ip].Im*UR;
				X[ip].Re = X[i].Re - TR;
				X[ip].Im = X[i].Im - TI;
				X[i].Re = X[i].Re + TR;
				X[i].Im = X[i].Im + TI;
			}
			TR = UR;
			UR = TR*SR - UI*SI;
			UI = TR*SI + UI*SR;
		}
	}
}


// Inverse complex FFT
// Input: N complex points in frequency domain
// Output: N complex points in time domain
void IFFTC(Cmplx *X, int FFTSize)
{
	int	i;
	RealData	DivFFTSize = 1.0 / FFTSize;

	// Change the sign of ImX
	for (i=0;i<FFTSize;i++) X[i].Im = -X[i].Im;
	// Calculate N-point complex FFT
	FFTC(X,FFTSize);
	// Adjust the result data
	for (i=0;i<FFTSize;i++) {
		X[i].Re *= DivFFTSize;
		X[i].Im = -X[i].Im * DivFFTSize;
	}
}


