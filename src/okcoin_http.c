#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"
#include "okcoin.h"
#include "okcoin_config.h"
#include "okcoin_http.h"
#include "clist.h"
#include "okcoin_delay.h"
#include "md5_sign.h"

typedef struct curl_resp_data {
	char *val;
	size_t size;
} CurlRespData;


static size_t
CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	CurlRespData * curl_resp_data = (CurlRespData *)userp;

	curl_resp_data->val = realloc(curl_resp_data->val, curl_resp_data->size + realsize + 1);
	if(curl_resp_data->val == NULL){
		fprintf(stderr, "realloc failed, not enough memory\n");
		return 0;
	}

	memcpy(&(curl_resp_data->val[curl_resp_data->size]), contents, realsize);
	curl_resp_data->size += realsize;
	curl_resp_data->val[curl_resp_data->size] = 0;

	return realsize;
}


CURL *
okcoin_http_init()
{
	CURL *curl;
	curl = curl_easy_init();
	return curl;
}

void
okcoin_http_cleanup(CURL *curl)
{
	curl_easy_cleanup(curl);
}

static char *
okcoin_http_api(CURL *curl,  const char* api_url)
{
	delay_in_ms(1000);
	CURLcode	res;
	CurlRespData curl_resp_data;
	curl_resp_data.val = malloc(1);
	curl_resp_data.size = 0;
	
	curl_easy_reset(curl);
	curl_easy_setopt(curl, CURLOPT_URL, api_url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&curl_resp_data);

	res = curl_easy_perform(curl);
	if(res != CURLE_OK){
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
		return NULL;
	}

	//fprintf(stdout, "%s\n", curl_resp_data.val);
	return curl_resp_data.val;
}

static char *
okcoin_http_tapi(CURL *curl, const char *api_url, const char *data)
{
	delay_in_ms(1000);
	CURLcode	res;
	struct curl_slist *headerlist=NULL;
	CurlRespData curl_resp_data;
	curl_resp_data.val = malloc(1);
	curl_resp_data.size = 0;
	
	//http headers
	static const char buf[] = "Content-type:application/x-www-form-urlencoded";
	headerlist = curl_slist_append(headerlist, buf);

	curl_easy_reset(curl);
	curl_easy_setopt(curl, CURLOPT_URL, api_url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&curl_resp_data);
	//post
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
 
	
	res = curl_easy_perform(curl); curl_slist_free_all (headerlist);
	if(res != CURLE_OK){
	  fprintf(stderr, "curl_easy_perform() failed: %s\n",
			  curl_easy_strerror(res));
	  return NULL;
	}

	return curl_resp_data.val;
}


int
okcoin_get_ticker(CURL *curl, Ticker *ticker)
{
	const char 	*TICKER_URL = "https://www.okcoin.cn/api/ticker.do?symbol=ltc_cny";

	char *resp_string = okcoin_http_api(curl, TICKER_URL);
	if (!resp_string){
		fprintf(stderr, "okcoin_http_api() failed.");
		return 1;
	}

	cJSON *json_root = cJSON_Parse(resp_string);
	if (!json_root) {
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return 2;
	}

	cJSON *json_ticker = cJSON_GetObjectItem(json_root, "ticker");
	ticker->high = atof(cJSON_GetObjectItem(json_ticker, "high")->valuestring);
	ticker->low = atof(cJSON_GetObjectItem(json_ticker, "low")->valuestring);
	ticker->buy = atof(cJSON_GetObjectItem(json_ticker, "buy")->valuestring);
	ticker->sell = atof(cJSON_GetObjectItem(json_ticker, "sell")->valuestring);
	ticker->last = atof(cJSON_GetObjectItem(json_ticker, "last")->valuestring);
	ticker->vol = atof(cJSON_GetObjectItem(json_ticker, "vol")->valuestring);

	cJSON_Delete(json_root);
	if (resp_string)
		free(resp_string);
	
	return 0;
}


int
okcoin_get_userinfo(CURL *curl, UserInfo *userinfo)
{
	const char 	*USERINFO_URL = "https://www.okcoin.cn/api/userinfo.do";

	char partner[8], secret_key[33];
	read_config_value("partner", partner);
	read_config_value("secret_key", secret_key);
	
	char *to_sign = (char *)malloc(strlen("partner=") + strlen(partner) + strlen(secret_key) + 1);
	sprintf(to_sign, "%s%s%s", "partner=", partner, secret_key);
	// printf("%s\n", to_sign);

	char *sign = md5_sign(to_sign);
	char *post_data = (char *)malloc(strlen("partner=") + strlen(partner) + strlen(" sign=") + strlen(sign) + 1);
	sprintf(post_data, "%s%s%s%s", "partner=", partner, "&sign=", sign);
	// printf("%s\n", post_data);


	char *resp_string = okcoin_http_tapi(curl, USERINFO_URL, post_data);
	if (!resp_string){
		fprintf(stderr, "okcoin_http_tapi() failed.");
		return 1;
	}
	//printf("%s\n", resp_string);

	cJSON *json_root = cJSON_Parse(resp_string);
	if (!json_root) {
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return 2;
	}
	// printf("%s\n", cJSON_Print(json_root));

	int result = cJSON_GetObjectItem(json_root, "result")->valueint;
	if(result == 1){  //success
		cJSON *json_info = cJSON_GetObjectItem(json_root, "info");
		cJSON *json_funds = cJSON_GetObjectItem(json_info, "funds");
		// printf("%s\n", cJSON_Print(json_funds));


		cJSON *json_asset = cJSON_GetObjectItem(json_funds, "asset");
		userinfo->asset_net = atof(cJSON_GetObjectItem(json_asset, "net")->valuestring);
		userinfo->asset_net = atof(cJSON_GetObjectItem(json_asset, "total")->valuestring);

		cJSON *json_free = cJSON_GetObjectItem(json_funds, "free");
		userinfo->free_ltc = atof(cJSON_GetObjectItem(json_free, "ltc")->valuestring);
		userinfo->free_cny = atof(cJSON_GetObjectItem(json_free, "cny")->valuestring);


		cJSON *json_freezed = cJSON_GetObjectItem(json_funds, "freezed");
		userinfo->freezed_ltc = atof(cJSON_GetObjectItem(json_freezed, "ltc")->valuestring);
		userinfo->freezed_cny = atof(cJSON_GetObjectItem(json_freezed, "cny")->valuestring);


		return 0;
	}




	cJSON_Delete(json_root);
	if (resp_string)
		free(resp_string);
	if (to_sign)
		free(to_sign);
	if (post_data)
		free(post_data);

	return 3;
}

int
okcoin_get_unprocess_orders(CURL *curl, OrderList *order_list)
{
	const char 	*USERINFO_URL = "https://www.okcoin.cn/api/getorder.do";
	order_list->list = list_create();

	char partner[8], secret_key[33];
	read_config_value("partner", partner);
	read_config_value("secret_key", secret_key);
	
	char *to_sign = (char *)malloc(strlen("order_id=-1") + \
		strlen("&partner=") + \
		strlen(partner) + \
		strlen("&symbol=ltc_cny") + \
		strlen(secret_key) + 1);
	sprintf(to_sign, "%s%s%s%s%s", "order_id=-1", "&partner=", partner, "&symbol=ltc_cny",  secret_key);
	// printf("%s\n", to_sign);

	char *sign = md5_sign(to_sign);
	char *post_data = (char *)malloc(strlen("order_id=-1") + \
		strlen("&partner=") + \
		strlen(partner) + \
		strlen("&symbol=ltc_cny") + \
		strlen("&sign=") + \
		strlen(sign) + 1);
	sprintf(post_data, "%s%s%s%s%s%s", "order_id=-1", "&partner=", partner, "&sign=", sign, "&symbol=ltc_cny");
	// printf("%s\n", post_data);


	char *resp_string = okcoin_http_tapi(curl, USERINFO_URL, post_data);
	if (!resp_string){
		fprintf(stderr, "okcoin_http_tapi() failed.");
		return 1;
	}
	// printf("%s\n", resp_string);

	cJSON *json_root = cJSON_Parse(resp_string);
	if (!json_root) {
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return 2;
	}
	// printf("%s\n", cJSON_Print(json_root));

	int result = cJSON_GetObjectItem(json_root, "result")->valueint;
	if(result == 1){  //success
		cJSON *json_order_list = cJSON_GetObjectItem(json_root, "orders");
		// printf("%s\n", cJSON_Print(json_order_list));


		int order_qty = cJSON_GetArraySize(json_order_list);
		for (int i = 0; i < order_qty; ++i)
		{
			cJSON *json_order = cJSON_GetArrayItem(json_order_list, i);
			// printf("%s\n", cJSON_Print(json_order));

			Order *order = (Order *)malloc(sizeof(Order));

			order->order_id = cJSON_GetObjectItem(json_order, "orders_id")->valueint;
			order->amount = cJSON_GetObjectItem(json_order, "amount")->valuedouble;
			order->rate = cJSON_GetObjectItem(json_order, "rate")->valuedouble;
			order->symbol = cJSON_GetObjectItem(json_order, "symbol")->valuestring;
			order->type = cJSON_GetObjectItem(json_order, "type")->valuestring;
			
			list_append(order_list->list, order);
		}


		return 0;
	}



	cJSON_Delete(json_root);
	if (resp_string)
		free(resp_string);
	if (to_sign)
		free(to_sign);
	if (post_data)
		free(post_data);

	return 3;
}

int okcoin_cancel_order(CURL *curl, Order *order)
{
	const char 	*USERINFO_URL = "https://www.okcoin.cn/api/cancelorder.do";

	char partner[8], secret_key[33];
	read_config_value("partner", partner);
	read_config_value("secret_key", secret_key);

	char str_order_id[15];
	sprintf(str_order_id, "%d", order->order_id);

	char *to_sign = (char *)malloc(strlen("order_id=") + strlen(str_order_id) + \
		strlen("&partner=") + strlen(partner) + \
		strlen("&symbol=") + strlen(order->symbol) + \
		strlen(secret_key) + 1);
	sprintf(to_sign, "%s%d%s%s%s%s%s", "order_id=", order->order_id, 
		"&partner=", partner, 
		"&symbol=", order->symbol, 
		secret_key);
	// printf("%s\n", to_sign);

	char *sign = md5_sign(to_sign);
	char *post_data = (char *)malloc(strlen("order_id=") + strlen(str_order_id) + \
		strlen("&partner=") + strlen(partner) + \
		strlen("&symbol=") + strlen(order->symbol) + \
		strlen("&sign=") + strlen(sign) + 1);
	sprintf(post_data, "%s%d%s%s%s%s%s%s", "order_id=", order->order_id, 
		"&partner=", partner, 
		"&symbol=", order->symbol, 
		"&sign=", sign); 
	// printf("%s\n", post_data);


	char *resp_string = okcoin_http_tapi(curl, USERINFO_URL, post_data);
	if (!resp_string){
		fprintf(stderr, "okcoin_http_tapi() failed.");
		return -1;
	}
	printf("%s\n", resp_string);

	cJSON *json_root = cJSON_Parse(resp_string);
	if (!json_root) {
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	}
	// // printf("%s\n", cJSON_Print(json_root));


	cJSON_Delete(json_root);
	if (resp_string)
		free(resp_string);
	if (to_sign)
		free(to_sign);
	if (post_data)
		free(post_data);

	return 0;
}

int okcoin_place_order(CURL *curl, Order *order)
{
	const char 	*USERINFO_URL = "https://www.okcoin.cn/api/trade.do";

	char partner[8], secret_key[33];
	read_config_value("partner", partner);
	read_config_value("secret_key", secret_key);
	
	char *to_sign = (char *)malloc(strlen("amount=1.00") + \
		strlen("&partner=") + strlen(partner) + \
		strlen("&rate=1.00") + \
		strlen("&symbol=") + strlen(order->symbol) + \
		strlen("&type=") + strlen(order->type) + \
		strlen(secret_key) + 1);
	sprintf(to_sign, "%s%.2f%s%s%s%.2f%s%s%s%s%s", "amount=", order->amount, 
		"&partner=", partner, 
		"&rate=", order->rate, 
		"&symbol=", order->symbol, 
		"&type=", order->type, 
		secret_key);
	// printf("%s\n", to_sign);

	char *sign = md5_sign(to_sign);
	char *post_data = (char *)malloc(strlen("amount=1.00") + \
		strlen("&partner=") + strlen(partner) + \
		strlen("&rate=1.00") + \
		strlen("&symbol=") + strlen(order->symbol) + \
		strlen("&type=") + strlen(order->type) + \
		strlen("&sign=") + strlen(sign) + 1);
	sprintf(post_data, "%s%.2f%s%s%s%.2f%s%s%s%s%s%s", "amount=", order->amount, 
		"&partner=", partner, 
		"&rate=", order->rate, 
		"&symbol=", order->symbol, 
		"&type=", order->type, 
		"&sign=", sign); 
	// printf("%s\n", post_data);


	char *resp_string = okcoin_http_tapi(curl, USERINFO_URL, post_data);
	if (!resp_string){
		fprintf(stderr, "okcoin_http_tapi() failed.");
		return -1;
	}
	printf("%s\n", resp_string);

	cJSON *json_root = cJSON_Parse(resp_string);
	if (!json_root) {
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	}
	// // printf("%s\n", cJSON_Print(json_root));


	cJSON_Delete(json_root);
	if (resp_string)
		free(resp_string);
	if (to_sign)
		free(to_sign);
	if (post_data)
		free(post_data);

	return 0;
}