#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "json_value.h"

void generate_c_code(const JsonValue *value, const char *struct_name,
                     FILE *output);
void generate_nested_structs(const JsonValue *value, const char *parent_name,
                             FILE *output);

#endif // CODE_GENERATOR_H
