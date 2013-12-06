#ifndef BEATFLOWER_H__
#define BEATFLOWER_H__ 1

void config_set_defaults(config_t *config);
void config_load(config_t *cfg);
void config_save(config_t *cfg);
void find_color(Sint16 data[2][256]);

void *beatflower_thread(void *blah);

#endif // BEATFLOWER_H__ 1
