OBJS = bin/okcoin_http.o bin/cJSON.o bin/md5_sign.o bin/clist.o \
	bin/okcoin_delay.o bin/okcoin_config.o bin/okcoin_utils.o
CCFLAGS = -lcurl -lm -lcrypto

ifneq ($(debug), true)
$(warning "use 'make debug=true' to build for gdb debug.")
CC = clang -Wall
else
CC = clang -g -Wall
endif

all: bin/okcoin

bin/okcoin: $(OBJS)  src/okcoin.c src/okcoin.h
	$(CC) -o $@  src/okcoin.c $(OBJS) $(CCFLAGS)

bin/okcoin_http.o: src/okcoin_http.c src/okcoin_http.h
	$(CC) -o $@ -c src/okcoin_http.c

bin/clist.o: src/clist.c src/clist.h
	$(CC) -o $@ -c src/clist.c

bin/okcoin_delay.o: src/okcoin_delay.c src/okcoin_delay.h
	$(CC) -o $@ -c src/okcoin_delay.c

bin/okcoin_config.o: src/okcoin_config.c src/okcoin_config.h
	$(CC) -o $@ -c src/okcoin_config.c

bin/okcoin_utils.o: src/okcoin_utils.c src/okcoin_utils.h
	$(CC) -o $@ -c src/okcoin_utils.c

bin/cJSON.o: src/cJSON.c src/cJSON.h
	$(CC) -o $@ -c src/cJSON.c

bin/md5_sign.o: src/md5_sign.c src/md5_sign.h
	$(CC) -o $@ -c src/md5_sign.c

.PHONY : clean
clean :
	-rm -f bin/*.o bin/okcoin
