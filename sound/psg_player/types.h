#include <stdint.h>
typedef struct ay_struct
{
	uint8_t regs[16];
	uint8_t tone_wave[3];
	uint8_t noise_mod;
	uint16_t tone_counters[3];
	uint16_t noise_counter;
	uint32_t envelope_counter;
	uint8_t envelope_phase;
	uint8_t envelope_state;
	float 	tick_fraction;
	float 	tps;			// ticks per sample
	uint8_t a,b,c;
	void (*fcb)(void);
	uint32_t tpf;
	uint32_t frame_counter;
} ay_context;
