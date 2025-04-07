#ifndef JSON_VALUE_H
#define JSON_VALUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef enum {
  JSON_NULL,
  JSON_BOOL,
  JSON_NUMBER,
  JSON_STRING,
  JSON_ARRAY,
  JSON_OBJECT,
  JSON_DATE,
  JSON_HEX
} JsonType;

typedef struct JsonValue JsonValue;
typedef struct JsonMember JsonMember;

struct JsonValue {
  JsonType type;
  bool is_null;
  union {
    int boolean;
    double number;
    char *string;
    struct {
      JsonValue *values;
      size_t count;
    } array;
    struct {
      JsonMember *members;
      size_t count;
    } object;
    char *date_string;
    unsigned int hex_value;
  } value;
};

struct JsonMember {
  char *key;
  JsonValue value;
};

void print_json_value(const JsonValue *value, int indent);
void free_json_value(JsonValue *value);

#endif // JSON_VALUE_H
