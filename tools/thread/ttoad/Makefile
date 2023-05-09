CC = gcc
DEFS = -DHAVE_CONFIG_H
DINCS = -I. -I..
INCS = -I../examples -I../include -I../include/coap -isystem../include/coap
WARNINGS = -pedantic -Wall -Wextra -Wformat-security -Winline -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wunused  -Wlogical-op -Wunused-result
CFLAGS = -g -O2 -std=c99 -fdiagnostics-color -D_GNU_SOURCE -DWITH_POSIX
OBJS = coap_list.o ttoad-server.o
SRC_DIR = ../examples
LIBS_DIR = ../.libs
LIBS = $(LIBS_DIR)/libcoap-1.la -lrt

ttoad-server: $(OBJS)
	    /bin/bash ../libtool --tag=CC --mode=link $(CC) $(INCS) $(WARNINGS) $(CFLAGS) -o $@ $^ $(LIBS)

coap_list.o: $(SRC_DIR)/coap_list.c $(SRC_DIR)/coap_list.h
	  
	    $(CC) $(DEFS) $(DINCS) $(INCS) $(WARNINGS) $(CFLAGS) -c -o $@ $<
	      
ttoad-server.o: ttoad-server.c $(SRC_DIR)/coap_list.h
	  
	    $(CC) $(DEFS) $(DINCS) $(INCS) $(WARNINGS) $(CFLAGS) -c -o $@ $<
	      
.PHONY: clean

clean:
	  rm -rf *.o ttoad-server .libs

