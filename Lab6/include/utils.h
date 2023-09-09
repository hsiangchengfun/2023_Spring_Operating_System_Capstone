#ifndef	_BOOT_H
#define	_BOOT_H

extern void delay ( unsigned long );
extern void put32 ( unsigned long, unsigned int );
extern unsigned int get32 ( unsigned long );

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;


#endif  /*_BOOT_H */