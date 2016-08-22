#include <stdint.h>
typedef struct ay_struct
{
	uint8_t regs[16];
	uint8_t tone_wave[3];
	uint16_t tone_counters[3];
	uint16_t noise_counter;
	uint16_t envelope_counter;
	uint8_t envelope;
	float 	tick_fraction;
	float 	tps;			// ticks per sample
} ay_context;
