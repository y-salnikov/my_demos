#include<stdint.h>

typedef struct drostatstr
{
	uint8_t	playing;
	uint8_t loop;
	uint32_t position;
	uint32_t length;
} dro_status;

//void dro_poll(void);
uint8_t dro_load(void *data, uint32_t size);
void dro_play(int8_t loop);
void dro_stop();
void dro_get_status(dro_status *stat);

