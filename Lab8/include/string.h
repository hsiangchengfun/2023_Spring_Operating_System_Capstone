#ifndef _STRING_H
#define _STRING_H

int strcmp (const char *, const char *);
int strncmp (const char *, const char *, unsigned int);
unsigned int strlen(const char *);
void strcpy(char *dest, const char *src);
void strncpy(char *dest, const char *src, int n);

#endif