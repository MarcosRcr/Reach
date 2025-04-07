#include "../includes/code_generator.h"
#include "../includes/json_value.h"
#include "../includes/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void generate_nested_structs(const JsonValue *value, const char *parent_name,
                             FILE *output) {
  if (value->type == JSON_OBJECT) {
    for (size_t i = 0; i < value->value.object.count; i++) {
      const JsonMember *member = &value->value.object.members[i];
      if (member->value.type == JSON_OBJECT) {
        char nested_name[256];
        snprintf(nested_name, sizeof(nested_name), "%s_%s", parent_name,
                 member->key);

        fprintf(output, "typedef struct %s {\n", nested_name);
        for (size_t j = 0; j < member->value.value.object.count; j++) {
          const JsonMember *nested_member =
              &member->value.value.object.members[j];
          char *sanitized_key = sanitize_identifier(nested_member->key);

          switch (nested_member->value.type) {
          case JSON_STRING:
            fprintf(output, "    char %s[%zu];\n", sanitized_key,
                    strlen(nested_member->value.value.string) + 1);
            break;
          case JSON_NUMBER:
            if (nested_member->value.value.number ==
                (int)nested_member->value.value.number) {
              fprintf(output, "    int %s;\n", sanitized_key);
            } else {
              fprintf(output, "    double %s;\n", sanitized_key);
            }
            break;
          case JSON_BOOL:
            fprintf(output, "    bool %s;\n", sanitized_key);
            break;
          case JSON_DATE:
            fprintf(output, "    char %s[11]; // YYYY-MM-DD\n", sanitized_key);
            break;
          case JSON_HEX:
            fprintf(output, "    unsigned int %s;\n", sanitized_key);
            break;
          default:
            fprintf(output, "    // Tipo no soportado para %s\n",
                    sanitized_key);
          }
          free(sanitized_key);
        }
        fprintf(output, "} %s;\n\n", nested_name);

        // Generar funciones para la estructura anidada
        fprintf(output, "void init_%s(%s *data) {\n", nested_name, nested_name);
        for (size_t j = 0; j < member->value.value.object.count; j++) {
          const JsonMember *nested_member =
              &member->value.value.object.members[j];
          char *sanitized_key = sanitize_identifier(nested_member->key);

          switch (nested_member->value.type) {
          case JSON_STRING: {
            char escaped_str[256];
            escape_string_for_c(escaped_str, nested_member->value.value.string,
                                sizeof(escaped_str));
            fprintf(output, "    strcpy(data->%s, \"%s\");\n", sanitized_key,
                    escaped_str);
            break;
          }
          case JSON_NUMBER:
            if (nested_member->value.value.number ==
                (int)nested_member->value.value.number) {
              fprintf(output, "    data->%s = %d;\n", sanitized_key,
                      (int)nested_member->value.value.number);
            } else {
              fprintf(output, "    data->%s = %f;\n", sanitized_key,
                      nested_member->value.value.number);
            }
            break;
          case JSON_BOOL:
            fprintf(output, "    data->%s = %s;\n", sanitized_key,
                    nested_member->value.value.boolean ? "true" : "false");
            break;
          case JSON_DATE:
            fprintf(output, "    strcpy(data->%s, \"%s\");\n", sanitized_key,
                    nested_member->value.value.date_string);
            break;
          case JSON_HEX:
            fprintf(output, "    data->%s = 0x%X;\n", sanitized_key,
                    nested_member->value.value.hex_value);
            break;
          }
          free(sanitized_key);
        }
        fprintf(output, "}\n\n");

        fprintf(output, "void print_%s(const %s *data) {\n", nested_name,
                nested_name);
        for (size_t j = 0; j < member->value.value.object.count; j++) {
          const JsonMember *nested_member =
              &member->value.value.object.members[j];
          char *sanitized_key = sanitize_identifier(nested_member->key);

          fprintf(output, "    printf(\"%s: ", nested_member->key);
          switch (nested_member->value.type) {
          case JSON_STRING:
            fprintf(output, "%%s\\n\", data->%s);\n", sanitized_key);
            break;
          case JSON_NUMBER:
            if (nested_member->value.value.number ==
                (int)nested_member->value.value.number) {
              fprintf(output, "%%d\\n\", data->%s);\n", sanitized_key);
            } else {
              fprintf(output, "%%f\\n\", data->%s);\n", sanitized_key);
            }
            break;
          case JSON_BOOL:
            fprintf(output, "%%s\\n\", data->%s ? \"true\" : \"false\");\n",
                    sanitized_key);
            break;
          case JSON_DATE:
            fprintf(output, "%%s\\n\", data->%s);\n", sanitized_key);
            break;
          case JSON_HEX:
            fprintf(output, "0x%%X\\n\", data->%s);\n", sanitized_key);
            break;
          }
          free(sanitized_key);
        }
        fprintf(output, "}\n\n");

        generate_nested_structs(&member->value, nested_name, output);
      }
    }
  }
}

void generate_c_code(const JsonValue *value, const char *struct_name,
                     FILE *output) {
  fprintf(output, "// Código generado automáticamente a partir de JSON\n");
  fprintf(output, "#include <stdio.h>\n");
  fprintf(output, "#include <stdbool.h>\n");
  fprintf(output, "#include <string.h>\n");
  fprintf(output, "#include <time.h>\n\n");

  // Primero generar todas las estructuras anidadas
  generate_nested_structs(value, struct_name, output);

  // Generar la estructura principal
  if (value->type == JSON_OBJECT) {
    fprintf(output, "typedef struct %s {\n", struct_name);

    for (size_t i = 0; i < value->value.object.count; i++) {
      const JsonMember *member = &value->value.object.members[i];
      char *sanitized_key = sanitize_identifier(member->key);

      switch (member->value.type) {
      case JSON_NULL:
        fprintf(output, "    void *%s;\n", sanitized_key);
        break;
      case JSON_BOOL:
        fprintf(output, "    bool %s;\n", sanitized_key);
        break;
      case JSON_NUMBER:
        if (member->value.value.number == (int)member->value.value.number) {
          fprintf(output, "    int %s;\n", sanitized_key);
        } else {
          fprintf(output, "    double %s;\n", sanitized_key);
        }
        break;
      case JSON_STRING:
        fprintf(output, "    char %s[%zu];\n", sanitized_key,
                strlen(member->value.value.string) + 1);
        break;
      case JSON_DATE:
        fprintf(output, "    char %s[11]; // YYYY-MM-DD\n", sanitized_key);
        break;
      case JSON_HEX:
        fprintf(output, "    unsigned int %s;\n", sanitized_key);
        break;
      case JSON_ARRAY:
        if (member->value.value.array.count > 0) {
          bool homogeneous = true;
          JsonType first_type = member->value.value.array.values[0].type;

          for (size_t j = 1; j < member->value.value.array.count; j++) {
            if (member->value.value.array.values[j].type != first_type) {
              homogeneous = false;
              break;
            }
          }

          if (homogeneous) {
            if (first_type == JSON_STRING) {
              fprintf(output, "    char %s[%zu][64]; // Array de strings\n",
                      sanitized_key, member->value.value.array.count);
            } else if (first_type == JSON_NUMBER) {
              fprintf(output, "    double %s[%zu]; // Array de números\n",
                      sanitized_key, member->value.value.array.count);
            } else if (first_type == JSON_BOOL) {
              fprintf(output, "    bool %s[%zu]; // Array de booleanos\n",
                      sanitized_key, member->value.value.array.count);
            }
          } else {
            fprintf(output, "    // Array heterogéneo para %s\n", member->key);
            fprintf(output, "    struct {\n");
            fprintf(output, "        JsonType type;\n");
            fprintf(output, "        union {\n");
            fprintf(output, "            char str_val[64];\n");
            fprintf(output, "            double num_val;\n");
            fprintf(output, "            bool bool_val;\n");
            fprintf(output, "        } value;\n");
            fprintf(output, "    } %s[%zu];\n", sanitized_key,
                    member->value.value.array.count);
          }
        }
        break;
      case JSON_OBJECT: {
        char nested_name[256];
        snprintf(nested_name, sizeof(nested_name), "%s_%s", struct_name,
                 member->key);
        fprintf(output, "    struct %s %s;\n", nested_name, sanitized_key);
        break;
      }
      }
      free(sanitized_key);
    }

    fprintf(output, "} %s;\n\n", struct_name);

    // Función para inicializar la estructura
    fprintf(output, "void init_%s(%s *data) {\n", struct_name, struct_name);
    for (size_t i = 0; i < value->value.object.count; i++) {
      const JsonMember *member = &value->value.object.members[i];
      char *sanitized_key = sanitize_identifier(member->key);

      switch (member->value.type) {
      case JSON_NULL:
        fprintf(output, "    data->%s = NULL;\n", sanitized_key);
        break;
      case JSON_BOOL:
        fprintf(output, "    data->%s = %s;\n", sanitized_key,
                member->value.value.boolean ? "true" : "false");
        break;
      case JSON_NUMBER:
        if (member->value.value.number == (int)member->value.value.number) {
          fprintf(output, "    data->%s = %d;\n", sanitized_key,
                  (int)member->value.value.number);
        } else {
          fprintf(output, "    data->%s = %f;\n", sanitized_key,
                  member->value.value.number);
        }
        break;
      case JSON_STRING: {
        char escaped_str[256];
        escape_string_for_c(escaped_str, member->value.value.string,
                            sizeof(escaped_str));
        fprintf(output, "    strcpy(data->%s, \"%s\");\n", sanitized_key,
                escaped_str);
        break;
      }
      case JSON_DATE:
        fprintf(output, "    strcpy(data->%s, \"%s\");\n", sanitized_key,
                member->value.value.date_string);
        break;
      case JSON_HEX:
        fprintf(output, "    data->%s = 0x%X;\n", sanitized_key,
                member->value.value.hex_value);
        break;
      case JSON_ARRAY:
        if (member->value.value.array.count > 0) {
          bool homogeneous = true;
          JsonType first_type = member->value.value.array.values[0].type;

          for (size_t j = 1; j < member->value.value.array.count; j++) {
            if (member->value.value.array.values[j].type != first_type) {
              homogeneous = false;
              break;
            }
          }

          if (homogeneous) {
            if (first_type == JSON_STRING) {
              for (size_t j = 0; j < member->value.value.array.count; j++) {
                char escaped_str[256];
                escape_string_for_c(
                    escaped_str,
                    member->value.value.array.values[j].value.string,
                    sizeof(escaped_str));
                fprintf(output, "    strcpy(data->%s[%zu], \"%s\");\n",
                        sanitized_key, j, escaped_str);
              }
            } else if (first_type == JSON_NUMBER) {
              for (size_t j = 0; j < member->value.value.array.count; j++) {
                fprintf(output, "    data->%s[%zu] = %f;\n", sanitized_key, j,
                        member->value.value.array.values[j].value.number);
              }
            } else if (first_type == JSON_BOOL) {
              for (size_t j = 0; j < member->value.value.array.count; j++) {
                fprintf(output, "    data->%s[%zu] = %s;\n", sanitized_key, j,
                        member->value.value.array.values[j].value.boolean
                            ? "true"
                            : "false");
              }
            }
          } else {
            fprintf(output,
                    "    // Inicialización manual necesaria para array "
                    "heterogéneo %s\n",
                    sanitized_key);
          }
        }
        break;
      case JSON_OBJECT: {
        char nested_name[256];
        snprintf(nested_name, sizeof(nested_name), "%s_%s", struct_name,
                 member->key);
        fprintf(output, "    init_%s(&data->%s);\n", nested_name,
                sanitized_key);
        break;
      }
      }
      free(sanitized_key);
    }
    fprintf(output, "}\n\n");

    // Función para imprimir la estructura
    fprintf(output, "void print_%s(const %s *data) {\n", struct_name,
            struct_name);
    for (size_t i = 0; i < value->value.object.count; i++) {
      const JsonMember *member = &value->value.object.members[i];
      char *sanitized_key = sanitize_identifier(member->key);

      switch (member->value.type) {
      case JSON_NULL:
        fprintf(
            output,
            "    printf(\"%s: %%s\\n\", data->%s ? \"(null)\" : \"NULL\");\n",
            member->key, sanitized_key);
        break;
      case JSON_BOOL:
        fprintf(
            output,
            "    printf(\"%s: %%s\\n\", data->%s ? \"true\" : \"false\");\n",
            member->key, sanitized_key);
        break;
      case JSON_NUMBER:
        if (member->value.value.number == (int)member->value.value.number) {
          fprintf(output, "    printf(\"%s: %%d\\n\", data->%s);\n",
                  member->key, sanitized_key);
        } else {
          fprintf(output, "    printf(\"%s: %%f\\n\", data->%s);\n",
                  member->key, sanitized_key);
        }
        break;
      case JSON_STRING:
        fprintf(output, "    printf(\"%s: %%s\\n\", data->%s);\n", member->key,
                sanitized_key);
        break;
      case JSON_DATE:
        fprintf(output, "    printf(\"%s: %%s\\n\", data->%s);\n", member->key,
                sanitized_key);
        break;
      case JSON_HEX:
        fprintf(output, "    printf(\"%s: 0x%%X\\n\", data->%s);\n",
                member->key, sanitized_key);
        break;
      case JSON_ARRAY:
        fprintf(output, "    printf(\"%s: [\");\n", member->key);
        if (member->value.value.array.count > 0) {
          bool homogeneous = true;
          JsonType first_type = member->value.value.array.values[0].type;

          for (size_t j = 1; j < member->value.value.array.count; j++) {
            if (member->value.value.array.values[j].type != first_type) {
              homogeneous = false;
              break;
            }
          }

          if (homogeneous) {
            if (first_type == JSON_STRING) {
              fprintf(output, "    for (size_t i = 0; i < %zu; i++) {\n",
                      member->value.value.array.count);
              fprintf(output,
                      "        printf(i == 0 ? \"\\\"%%s\\\"\" : \", "
                      "\\\"%%s\\\"\", data->%s[i]);\n",
                      sanitized_key);
              fprintf(output, "    }\n");
            } else if (first_type == JSON_NUMBER) {
              fprintf(output, "    for (size_t i = 0; i < %zu; i++) {\n",
                      member->value.value.array.count);
              fprintf(output,
                      "        printf(i == 0 ? \"%%f\" : \", %%f\", "
                      "data->%s[i]);\n",
                      sanitized_key);
              fprintf(output, "    }\n");
            } else if (first_type == JSON_BOOL) {
              fprintf(output, "    for (size_t i = 0; i < %zu; i++) {\n",
                      member->value.value.array.count);
              fprintf(output,
                      "        printf(i == 0 ? \"%%s\" : \", %%s\", "
                      "data->%s[i] ? \"true\" : \"false\");\n",
                      sanitized_key);
              fprintf(output, "    }\n");
            }
          } else {
            fprintf(output,
                    "    // Imprimir manualmente array heterogéneo %s\n",
                    sanitized_key);
          }
        }
        fprintf(output, "    printf(\"]\\n\");\n");
        break;
      case JSON_OBJECT: {
        char nested_name[256];
        snprintf(nested_name, sizeof(nested_name), "%s_%s", struct_name,
                 member->key);
        fprintf(output, "    printf(\"%s: {\\n\");\n", member->key);
        fprintf(output, "    print_%s(&data->%s);\n", nested_name,
                sanitized_key);
        fprintf(output, "    printf(\"}\\n\");\n");
        break;
      }
      }
      free(sanitized_key);
    }
    fprintf(output, "}\n\n");

    // Ejemplo de uso
    fprintf(output, "int main() {\n");
    fprintf(output, "    %s data;\n", struct_name);
    fprintf(output, "    init_%s(&data);\n", struct_name);
    fprintf(output, "    print_%s(&data);\n", struct_name);
    fprintf(output, "    return 0;\n");
    fprintf(output, "}\n");
  }
}
