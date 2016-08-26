#define FREQ 44100
#define CHANNELS 2
#define CHUNKSIZE 4096
#define N_CHANNELS 4

void sound_init(void(*sound_cb)(void *udata, Uint8 *stream, int len));
void sound_done(void);
