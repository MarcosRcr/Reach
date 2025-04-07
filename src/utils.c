#include "../includes/utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *sanitize_identifier(const char *key) {
  char *sanitized = strdup(key);
  for (int i = 0; sanitized[i]; i++) {
    if (!isalnum(sanitized[i]) && sanitized[i] != '_') {
      sanitized[i] = '_';
    }
  }
  return sanitized;
}

void escape_string_for_c(char *dest, const char *src, size_t dest_size) {
  size_t j = 0;
  for (size_t i = 0; src[i] != '\0' && j < dest_size - 1; i++) {
    if (src[i] == '"' || src[i] == '\\') {
      if (j < dest_size - 2) {
        dest[j++] = '\\';
        dest[j++] = src[i];
      }
    } else {
      dest[j++] = src[i];
    }
  }
  dest[j] = '\0';
}
