#ifndef UTILS_H
#define UTILS_H


#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>
#include "deque.h"
#include "fat32.h"


wchar_t *convert_to_wide(const char16_t *, size_t);
size_t utf16_strlen(const char16_t *);
wchar_t **tokenize(wchar_t *, wchar_t *, size_t *);
void solve(wchar_t *, struct deque *);


#endif