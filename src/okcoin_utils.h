#ifndef __OKCOIN_UTILS_H__
#define __OKCOIN_UTILS_H__

// void rtrim(char *str, char ch);
char *trim_space(char *str);
/* return path lengin in bytes, without last '/', return 0 if success*/
int okcoin_get_execution_dirname(char *path);

double okcoin_read_last_price();
void okcoin_save_last_price(double price);

#endif /* __OKCOIN_UTILS_H__ */