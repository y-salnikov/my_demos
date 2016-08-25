void ay_fill_samples(ay_context *ayc, int16_t *sound, uint32_t length); // length in samples = bytes/4
ay_context* ay_init(int samplerate, void(*frame_callback)(void));
