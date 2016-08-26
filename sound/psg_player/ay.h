void ay_fill_samples(ay_context *ayc, int16_t *sound, uint32_t length); // length in samples = bytes/4
ay_context* ay_init(int samplerate, void(*frame_callback)(void));
void ay_write(ay_context *ayc, uint8_t reg, uint8_t val);
void ay_done(ay_context *ayc);

