#ifndef __OKCOIN_HTTP_H__
#define __OKCOIN_HTTP_H__

#include "okcoin.h"
#include <curl/curl.h>

CURL *okcoin_http_init();
void okcoin_http_cleanup(CURL *curl);

int okcoin_get_ticker(CURL *curl, Ticker *ticker);
int okcoin_get_userinfo(CURL *curl, UserInfo *userinfo);
int okcoin_get_unprocess_orders(CURL *curl, OrderList *order_list);
int okcoin_cancel_order(CURL *curl, Order *order);
int okcoin_place_order(CURL *curl, Order *order);

#endif /* __OKCOIN_HTTP_H__ */
