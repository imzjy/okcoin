#ifndef __OKCOIN_H__
#define __OKCOIN_H__

#include "clist.h"


typedef struct ticker
{
	double	high;
	double	low;
	double	buy;
	double 	sell;
	double	last;
	double	vol;	
} Ticker;

typedef struct trade {
	char	*type;
	double 	price;
	double 	qty;
} Trade;



typedef struct order{
	int 	order_id;
	double 	amount;
	double 	rate;
	char 	*type;
	char 	*symbol;
} Order;

typedef struct orderList{
	List *list;
} OrderList;

typedef struct userinfo {
	double 	asset_net;
	double 	asset_total;
	double 	free_ltc;
	double 	free_cny;	
	double 	freezed_ltc;
	double 	freezed_cny;

	/*unused now*/
	double	last_pivot;
} UserInfo;


void dump_ticker(Ticker *t);
void dump_userinfo(UserInfo *u);
void dump_order(Order *o);

#endif /* __OKCOIN_H__ */
