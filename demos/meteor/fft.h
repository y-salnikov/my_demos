typedef double	RealData;
typedef struct C {
	RealData	Re;
	RealData	Im;
} Cmplx;
void FFTC(Cmplx *X, int FFTSize);
void IFFTC(Cmplx *X, int FFTSize);
