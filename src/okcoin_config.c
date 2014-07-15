#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "okcoin_utils.h"

static char *okcoin_conf_file = "/etc/okcoin/okcoin.conf";

int
read_config_value_by_file(FILE *fp, const char *key, char *val)
{
	if (fp == NULL)
	   exit(EXIT_FAILURE);

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != -1) {
	    //printf("Retrieved line of length %zu :\n", read);
		//printf("%s", line);
		line = trim_space(line);
		if(line[0] == '#')
			continue;
		
		if(!strncmp(line, key, strlen(key))){
			sprintf(val, "%s", line + strlen(key) + 1);
			return 0;
		}
	}

	free(line);
	return 0;
}

int
read_config_value(const char *key, char *val)
{
	FILE *fp;
	fp = fopen(okcoin_conf_file, "r");
	if (fp){
		if(read_config_value_by_file(fp, key, val)){
			fprintf(stderr, "read config file failed");	
			return 1;
		}
		fclose(fp);
	} else {
		fprintf(stderr, "read config file failed, due to %s\n", strerror(errno));
		return 2;
	}

	return 0;
}

