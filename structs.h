#include <stdlib.h>

static struct EthHeader {
  char d_mac[6];
  char s_mac[6];
  char type[2];
};

/*
unsigned int test;
FILE *fp1;

fp1 = fopen("/Users/matt/Desktop/test.txt", "r");

 char buffer[14];

 while (fgets(buffer, sizeof(buffer), fp1) != NULL) {
   struct EthHeader ping = GetHeader(buffer);
  */
