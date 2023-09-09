#ifndef __UTILS_H__
#define __UTILS_H__

#define BUFSIZE 256
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

void print_num(unsigned long value);
void print_h(unsigned long value);
int  atoi(const char *s, unsigned int size);
int  strcmp(const char *a, const char *b);
int  strncmp(const char *a, const char *b, unsigned int size);
unsigned int strlen(const char *s);
int readcmd(char *buf, int len);
char* strncpy(char *dst, const char *src, uint32_t len);



#endif