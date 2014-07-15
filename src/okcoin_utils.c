#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>

int 
okcoin_get_execution_dirname(char *path)
{
	char proc_buf[255];
	char exe_path_buf[255];

	sprintf(proc_buf, "/proc/%d/exe", getpid());
	int nb = readlink(proc_buf, exe_path_buf, 255 + 1);
	if(nb == -1){
		perror("read process's symbol link fail");
		return nb;
	}

	exe_path_buf[nb] = '\0';  //string terminating NUL


	char *dirpath = dirname(exe_path_buf);
	strncpy(path, dirpath, strlen(dirpath)+1);
	return 0;
}

// void 
// rtrim(char* str, char ch)
// {
// 	int len = strlen(str);
// 	if(str[len-1] == ch)
// 		str[len-1] = '\0';
// }

/*
Caution: change the str value by str++ will not affect caller's side pointer.
that's if I change the str to sencond character by moving str, it will not affect caller's pointer.
that's we only can modify the contents of pionter points, with char *p;
so, we return new pointer(address of certain characters in str)
*/
char *
trim_space(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}




static void get_last_file_path(char *path)
{
	char *last_file_name = "last_price.txt";
	char exe_path[255];

	
	if(okcoin_get_execution_dirname(exe_path)){
		perror("okcoin_get_execution_dirname failed.");
		return;
	}
	// printf("%s\n", exe_path);

	sprintf(path, "%s%s%s",
		exe_path,
		(path[strlen(path)-1] == '/' ? "" : "/"),
		last_file_name);
}

void 
okcoin_save_last_price(double price)
{
	FILE *fp;
	char last_file_full_path[255];
	get_last_file_path(last_file_full_path);

	
	fp = fopen(last_file_full_path, "w");
	if (fp){
		fprintf(fp, "%.2f", price);
		fclose(fp);
		// printf("write last price successed\n");
	} else {
		fprintf(stderr, "read %s failed:%s\n", last_file_full_path, strerror(errno));
		return;
	}
}

double 
okcoin_read_last_price()
{
	FILE *fp;
	char last_file_full_path[255];
	char last[8];

	get_last_file_path(last_file_full_path);

	
	fp = fopen(last_file_full_path, "r");
	if (fp){
		fscanf(fp, "%s", last);
		fclose(fp);
		// printf("read last %s\n", last);
		return atof(last);
	} else {
		fprintf(stderr, "read %s failed:%s\n", last_file_full_path, strerror(errno));
		return 0.0;
	}
}