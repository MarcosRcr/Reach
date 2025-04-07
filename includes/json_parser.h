#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "json_value.h"

char *read_json_file(const char *filename);
JsonValue parse_json(const char **json);
JsonValue parse_value(const char **json);
JsonValue parse_any_value(const char **json);
JsonValue parse_heterogeneous_array(const char **json);
JsonValue parse_object(const char **json);
JsonValue parse_array(const char **json);
JsonValue parse_string(const char **json);
JsonValue parse_number(const char **json);
JsonValue parse_bool(const char **json);
JsonValue parse_null(const char **json);
JsonValue parse_date(const char **json);
JsonValue parse_hex(const char **json);
void skip_whitespace(const char **json);
bool detect_special_value(const char **json, JsonValue *result);

#endif // JSON_PARSER_H
