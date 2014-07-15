#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include "md5_sign.h"

char *
md5_sign(const char* data)
{
 // unsigned char *MD5(const unsigned char *d, unsigned long n,
 //                  unsigned char *md);
 // int MD5_Init(MD5_CTX *c);
 // int MD5_Update(MD5_CTX *c, const void *data,
 //                  unsigned long len);
 // int MD5_Final(unsigned char *md, MD5_CTX *c);

	MD5_CTX ctx;
	unsigned char digest[16];
	char *hexOut =(char *)malloc(33);

	MD5_Init(&ctx);
	MD5_Update(&ctx, data, strlen(data));
	MD5_Final(digest, &ctx);
	

	//HEX string
 	for (int i = 0; i < 16; i++)
        sprintf(&hexOut[i*2], "%02X", (unsigned int)digest[i]);

    return hexOut;
}