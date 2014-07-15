#ifndef __OKCOIN_CONFIG_H__
#define __OKCOIN_CONFIG_H__

int read_config_value(const char *key, char *val);
int read_config_value_by_file(FILE *fp, const char *key, char *val);
void okcoin_config_save_last(double price);
double okcoin_config_read_last();

#endif /* __OKCOIN_CONFIG_H__ */