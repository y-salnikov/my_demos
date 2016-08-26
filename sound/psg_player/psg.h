
psg_context* psg_init(void(*write_ay_cb)(uint8_t reg, uint8_t val));
void psg_load(psg_context *pc, const char *filename);
void psg_next_frame(psg_context *pc);
void psg_play(psg_context *pc);
void psg_stop(psg_context *pc);
void psg_done(psg_context *pc);
