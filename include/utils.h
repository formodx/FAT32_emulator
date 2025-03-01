#ifndef UTILS_H
#define UTILS_H


#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "deque.h"
#include "fat32.h"


char **tokenize(const char *string, const char *delimiter, size_t *number_of_tokens);
void solve(const char *, struct deque *);


#endif