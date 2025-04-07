#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdio.h>

char *sanitize_identifier(const char *key);
void escape_string_for_c(char *dest, const char *src, size_t dest_size);

#endif // UTILS_H
