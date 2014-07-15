#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clist.h"
#include "okcoin_http.h"
#include "okcoin_utils.h"
#include "okcoin.h"

#define WINDOW_SIZE	 3
#define ORDER_AMOUNT 3

void place_orders(CURL *curl, double pivot)
{
	double sell_buf[WINDOW_SIZE] = {pivot+1, pivot+2, pivot+3};
	double buy_buf[WINDOW_SIZE] = {pivot-1, pivot-2, pivot-3};

	for (int i = 0; i < WINDOW_SIZE; ++i){
		Order *sell = (Order *)malloc(sizeof(Order));
		sell->amount = ORDER_AMOUNT;
		sell->rate = sell_buf[i];
		sell->symbol = "ltc_cny";
		sell->type = "sell";

		Order *buy = (Order *)malloc(sizeof(Order));
		buy->amount = ORDER_AMOUNT;
		buy->rate = buy_buf[i];
		buy->symbol = "ltc_cny";
		buy->type = "buy";

		okcoin_place_order(curl, sell);
		okcoin_place_order(curl, buy);

		free(sell);
		free(buy);
	}
}

void cancel_orders(CURL *curl, OrderList * order_list)
{
	ListNode *cur = (ListNode *)order_list->list->head;
	while(cur){
		Order *o = (Order *)cur->data;
		okcoin_cancel_order(curl, o);

		// dump_order(o);
		cur = cur->next;
	}
}

void dump_ticker(Ticker *t)
{
	printf("\t%s\t%f\n\t%s\t%f\n\t%s\t%f\n\t%s\t%f\n\t%s\t%f\n\t%s\t%f\n", 
		"high", t->high,
		"low",  t->low,
		"buy",  t->buy,
		"sell",  t->sell,
		"last",  t->last,
		"vol",  t->vol);
}

void dump_userinfo(UserInfo *u)
{
	printf("\t%s\t%f\n\t%s\t%f\n\t%s\t%f\n\t%s\t%f\n\t%s\t%f\n\t%s\t%f\n", 
		"asset_net", u->asset_net,
		"asset_total",  u->asset_total,
		"free_ltc",  u->free_ltc,
		"free_cny",  u->free_cny,
		"freezed_ltc",  u->freezed_ltc,
		"freezed_cny",  u->freezed_cny);
}

void dump_order(Order *o)
{
	printf("\t%s\t%d\n\t%s\t%f\n\t%s\t%f\n\t%s\t%s\n\t%s\t%s\n", 
		"orders_id", o->order_id,
		"\tamount",  o->amount,
		"\trate",  o->rate,
		"\ttype",  o->type,
		"\tsymbol",  o->symbol);
}

int 
main(int argc, char **argv)
{
	Ticker 	ticker;
	UserInfo userinfo;
	OrderList order_list;
	CURL	*curl;


	curl = okcoin_http_init();
	if(!curl){
		perror("okcoin_http_init failed.");
		return 0;
	}


	printf("[get user info]\n");
	if(okcoin_get_userinfo(curl, &userinfo)){
		perror("get userinfo failed");
		return 0;
	}
	dump_userinfo(&userinfo);


	printf("[get last ticker]\n");
	if(okcoin_get_ticker(curl, &ticker)){
		perror("get ticker failed");
		return 0;
	}
	dump_ticker(&ticker);

	double saved_last = okcoin_read_last_price();
	printf("cur last:%f, saved last:%f\n", ticker.last, saved_last);
	okcoin_save_last_price(ticker.last);

	printf("[get unprocess orders: ");
	if(okcoin_get_unprocess_orders(curl, &order_list)){
		perror("get order list error.");
		return 0;
	}
	printf("%d orders found]\n", order_list.list->size);

	if(abs(saved_last - ticker.last) < 0.5 && order_list.list->size > 4){
		printf("changes less than 0.5, so exit;\n");
		return 0;
	}

	printf("[cancel orders]\n");		
	cancel_orders(curl, &order_list);


	printf("place new order based on last pivot\n");
	place_orders(curl, ticker.last);

	
	okcoin_http_cleanup(curl);
	list_cleanup(order_list.list);
}
