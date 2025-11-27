#ifndef STRINGS_H
#define STRINGS_H

#include "drivers/type.h"  

void* memset(void* s, int c, u32int n);
int strcmp(const char* s1, const char* s2);
u32int strlen(const char* str);

#endif
