#include <unistd.h>


void delay_in_ms(int sleepMs)
{
    usleep(sleepMs * 1000);   // usleep takes sleep time in us (1 millionth of a second)
}