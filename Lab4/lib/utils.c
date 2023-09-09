#include "utils.h"
#include "uart.h"


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

void print_num(unsigned long value) {
    if (value == 0) {
        mini_uart_puts("0"); return;
    }

    char nums[20]; unsigned int len = 0;

    while (value) {
        unsigned int x = value % 10;
        nums[len++] = '0' + x;
        value /= 10;
    }

    for (int i = len - 1; i >= 0; i--) {
        mini_uart_putc(nums[i]);
    }
}
char* strncpy(char *dst, const char *src, uint32_t len) {
  for (uint32_t i = 0; i < len; i++) {
    dst[i] = src[i];
    if (src[i] == 0) break;
  }
  return dst;
}

void print_h(unsigned long value) {
    char nums[9]; nums[8] = '\0';

    for (int i = 7; i >= 0; i--) {
        unsigned int x = value % 16; value >>= 4;
        switch (x) {
            case 10: nums[i] = 'A';     break;
            case 11: nums[i] = 'B';     break;
            case 12: nums[i] = 'C';     break;
            case 13: nums[i] = 'D';     break;
            case 14: nums[i] = 'E';     break;
            case 15: nums[i] = 'F';     break;
            default: nums[i] = '0' + x; break;
        }
    }

    mini_uart_puts("0x");
    mini_uart_puts(nums);
}

int atoi(const char *s, unsigned int size) {
    int num = 0;

    for (unsigned int i = 0; i < size && s[i] != '\0'; i++) {
        if ('0' <= s[i] && s[i] <= '9') {
            num = (num * 16) + (s[i] - '0');
        } else if ('A' <= s[i] && s[i] <= 'F') {
            num = (num * 16) + (s[i] - 'A' + 10);
        } else if ('a' <= s[i] && s[i] <= 'f') {
            num = (num * 16) + (s[i] - 'a' + 10);
        }
    }

    return num;
}

int strcmp(const char *l, const char *r) {
    while (*l && (*l == *r)) {
        l++; r++;
    }

    return (const char) *l - (const char) *r;
}

int strncmp(const char *a, const char *b, unsigned int size) {
    
    for (unsigned int i = 0; i < size; i++) {
        if (a[i] != b[i]) {
            return (const char) a[i] - (const char) b[i];
        }
    }

    return 0;
}

unsigned int strlen(const char *s) {
    unsigned int len = 0;

    while (*s != '\0') {
        len++; s++;
    }

    return len;
}


int readcmd(char *buf, int len){
    


    char c;
    int i;
    for (i = 0; i < len; i++) {
        c = mini_uart_recv();
        if (c == 127) { i--; continue; }
        mini_uart_putc(c);
        // print_num((int)c);
        if (c == '\r') {
        c = '\n';
        mini_uart_putc('\n');
        break;
        }
        buf[i] = c;
    }
    buf[i] = '\0';
    return i;



}

int pow(int base, int power) {
    int ans = 1;
    for (int i = 0; i < power; i++) {
        ans *= base;
    }
    return ans;
}

void print_string(char* s){
    mini_uart_puts(s);
}