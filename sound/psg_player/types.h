#include <stdint.h>
typedef struct ay_struct
{
	uint8_t regs[16];
	uint8_t tone_wave[3];
	uint8_t envelope;
} ay_context;
