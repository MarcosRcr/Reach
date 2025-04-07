#include "../includes/json_value.h"
#include <stdio.h>
#include <stdlib.h>

void print_json_value(const JsonValue *value, int indent) {
  switch (value->type) {
  case JSON_NULL:
    printf("null");
    break;
  case JSON_BOOL:
    printf(value->value.boolean ? "true" : "false");
    break;
  case JSON_NUMBER:
    printf("%g", value->value.number);
    break;
  case JSON_STRING:
    printf("\"%s\"", value->value.string);
    break;
  case JSON_DATE:
    printf("\"%s\"", value->value.date_string);
    break;
  case JSON_HEX:
    printf("0x%X", value->value.hex_value);
    break;
  case JSON_ARRAY:
    printf("[\n");
    for (size_t i = 0; i < value->value.array.count; i++) {
      for (int j = 0; j < indent + 2; j++)
        printf(" ");
      print_json_value(&value->value.array.values[i], indent + 2);
      if (i < value->value.array.count - 1)
        printf(",");
      printf("\n");
    }
    for (int j = 0; j < indent; j++)
      printf(" ");
    printf("]");
    break;
  case JSON_OBJECT:
    printf("{\n");
    for (size_t i = 0; i < value->value.object.count; i++) {
      for (int j = 0; j < indent + 2; j++)
        printf(" ");
      printf("\"%s\": ", value->value.object.members[i].key);
      print_json_value(&value->value.object.members[i].value, indent + 2);
      if (i < value->value.object.count - 1)
        printf(",");
      printf("\n");
    }
    for (int j = 0; j < indent; j++)
      printf(" ");
    printf("}");
    break;
  }
}

void free_json_value(JsonValue *value) {
  switch (value->type) {
  case JSON_STRING:
    free(value->value.string);
    break;
  case JSON_DATE:
    free(value->value.date_string);
    break;
  case JSON_ARRAY:
    for (size_t i = 0; i < value->value.array.count; i++) {
      free_json_value(&value->value.array.values[i]);
    }
    free(value->value.array.values);
    break;
  case JSON_OBJECT:
    for (size_t i = 0; i < value->value.object.count; i++) {
      free(value->value.object.members[i].key);
      free_json_value(&value->value.object.members[i].value);
    }
    free(value->value.object.members);
    break;
  default:
    break;
  }
}
