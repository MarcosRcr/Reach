#include "../includes/json_parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_json_file(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Error: No se pudo abrir el archivo %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(length + 1);
  if (!buffer) {
    fclose(file);
    fprintf(stderr, "Error: No se pudo asignar memoria para el JSON\n");
    exit(EXIT_FAILURE);
  }

  size_t bytes_read = fread(buffer, 1, length, file);
  if (bytes_read != length) {
    fclose(file);
    free(buffer);
    fprintf(stderr, "Error: No se pudo leer el archivo completo\n");
    exit(EXIT_FAILURE);
  }

  buffer[length] = '\0';
  fclose(file);
  return buffer;
}

void skip_whitespace(const char **json) {
  while (isspace(**json)) {
    (*json)++;
  }
}

JsonValue parse_string(const char **json) {
  JsonValue value;
  value.type = JSON_STRING;
  value.is_null = false;
  value.value.string = NULL;

  if (**json != '"') {
    fprintf(stderr, "Error: Se esperaba comilla inicial de string\n");
    exit(EXIT_FAILURE);
  }

  (*json)++;

  const char *start = *json;
  size_t length = 0;

  while (**json != '"') {
    if (**json == '\0') {
      fprintf(stderr, "Error: String no terminado\n");
      exit(EXIT_FAILURE);
    }

    if (**json == '\\') {
      (*json)++;
      if (**json == '\0') {
        fprintf(stderr, "Error: Carácter de escape al final del string\n");
        exit(EXIT_FAILURE);
      }
      length++; // Contamos el carácter escapado
    }

    (*json)++;
    length++;
  }

  value.value.string = (char *)malloc(length + 1);
  if (!value.value.string) {
    fprintf(stderr, "Error: No se pudo asignar memoria para el string\n");
    exit(EXIT_FAILURE);
  }

  const char *src = start;
  char *dst = value.value.string;

  while (src < *json) {
    if (*src == '\\') {
      src++;
      switch (*src) {
      case '"':
        *dst++ = '"';
        break;
      case '\\':
        *dst++ = '\\';
        break;
      case '/':
        *dst++ = '/';
        break;
      case 'b':
        *dst++ = '\b';
        break;
      case 'f':
        *dst++ = '\f';
        break;
      case 'n':
        *dst++ = '\n';
        break;
      case 'r':
        *dst++ = '\r';
        break;
      case 't':
        *dst++ = '\t';
        break;
      case 'u': {
        // Manejo básico de Unicode (simplificado)
        char hex[5] = {src[1], src[2], src[3], src[4], '\0'};
        *dst++ = (char)strtol(hex, NULL, 16);
        src += 4;
        break;
      }
      default:
        *dst++ = *src;
        break;
      }
    } else {
      *dst++ = *src;
    }
    src++;
  }

  *dst = '\0';
  (*json)++;
  return value;
}

JsonValue parse_number(const char **json) {
  JsonValue value;
  value.type = JSON_NUMBER;
  value.is_null = false;

  char *end;
  value.value.number = strtod(*json, &end);

  if (end == *json) {
    fprintf(stderr, "Error: Número inválido\n");
    exit(EXIT_FAILURE);
  }

  *json = end;
  return value;
}

JsonValue parse_bool(const char **json) {
  JsonValue value;
  value.type = JSON_BOOL;
  value.is_null = false;

  if (strncmp(*json, "true", 4) == 0) {
    value.value.boolean = 1;
    *json += 4;
  } else if (strncmp(*json, "false", 5) == 0) {
    value.value.boolean = 0;
    *json += 5;
  } else {
    fprintf(stderr, "Error: Valor booleano inválido\n");
    exit(EXIT_FAILURE);
  }

  return value;
}

JsonValue parse_null(const char **json) {
  JsonValue value;
  value.type = JSON_NULL;
  value.is_null = true;

  if (strncmp(*json, "null", 4) != 0) {
    fprintf(stderr, "Error: Valor null inválido\n");
    exit(EXIT_FAILURE);
  }

  *json += 4;
  return value;
}

JsonValue parse_date(const char **json) {
  JsonValue value;
  value.type = JSON_DATE;
  value.is_null = false;

  if (**json != '"') {
    fprintf(stderr, "Error: Se esperaba comilla para fecha\n");
    exit(EXIT_FAILURE);
  }

  (*json)++;
  const char *start = *json;

  while (**json != '"' && **json != '\0')
    (*json)++;

  size_t length = *json - start;
  value.value.date_string = malloc(length + 1);
  strncpy(value.value.date_string, start, length);
  value.value.date_string[length] = '\0';

  if (**json == '"')
    (*json)++;

  // Validación básica de formato de fecha (YYYY-MM-DD)
  if (length != 10 || value.value.date_string[4] != '-' ||
      value.value.date_string[7] != '-') {
    fprintf(stderr,
            "Error: Formato de fecha inválido (se esperaba YYYY-MM-DD)\n");
    exit(EXIT_FAILURE);
  }

  return value;
}

JsonValue parse_hex(const char **json) {
  JsonValue value;
  value.type = JSON_HEX;
  value.is_null = false;

  if (strncmp(*json, "0x", 2) != 0 && strncmp(*json, "0X", 2) != 0) {
    fprintf(stderr, "Error: Valor hexadecimal inválido\n");
    exit(EXIT_FAILURE);
  }

  char *end;
  value.value.hex_value = strtoul(*json, &end, 16);

  if (end == *json) {
    fprintf(stderr, "Error: No se pudo convertir valor hexadecimal\n");
    exit(EXIT_FAILURE);
  }

  *json = end;
  return value;
}

bool detect_special_value(const char **json, JsonValue *result) {
  const char *start = *json;

  // Detectar fechas (formato ISO 8601 básico)
  if (**json == '"') {
    const char *date_ptr = start + 1;
    if (strlen(date_ptr) >= 10 && isdigit(date_ptr[0]) &&
        isdigit(date_ptr[1]) && isdigit(date_ptr[2]) && isdigit(date_ptr[3]) &&
        date_ptr[4] == '-' && isdigit(date_ptr[5]) && isdigit(date_ptr[6]) &&
        date_ptr[7] == '-' && isdigit(date_ptr[8]) && isdigit(date_ptr[9])) {
      *result = parse_date(json);
      return true;
    }
  }

  // Detectar hexadecimal
  if (strncmp(start, "0x", 2) == 0 || strncmp(start, "0X", 2) == 0) {
    *result = parse_hex(json);
    return true;
  }

  return false;
}

JsonValue parse_array(const char **json) {
  JsonValue value;
  value.type = JSON_ARRAY;
  value.is_null = false;
  value.value.array.values = NULL;
  value.value.array.count = 0;

  if (**json != '[') {
    fprintf(stderr, "Error: Se esperaba '[' para array\n");
    exit(EXIT_FAILURE);
  }

  (*json)++;
  skip_whitespace(json);

  if (**json == ']') {
    (*json)++;
    return value;
  }

  size_t capacity = 10;
  value.value.array.values = (JsonValue *)malloc(capacity * sizeof(JsonValue));
  if (!value.value.array.values) {
    fprintf(stderr, "Error: No se pudo asignar memoria para el array\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    skip_whitespace(json);

    if (value.value.array.count >= capacity) {
      capacity *= 2;
      JsonValue *temp = (JsonValue *)realloc(value.value.array.values,
                                             capacity * sizeof(JsonValue));
      if (!temp) {
        fprintf(stderr, "Error: No se pudo redimensionar el array\n");
        exit(EXIT_FAILURE);
      }
      value.value.array.values = temp;
    }

    value.value.array.values[value.value.array.count++] = parse_any_value(json);
    skip_whitespace(json);

    if (**json == ']') {
      break;
    }

    if (**json != ',') {
      fprintf(stderr, "Error: Se esperaba ',' o ']' en array\n");
      exit(EXIT_FAILURE);
    }

    (*json)++;
  }

  (*json)++;
  return value;
}

JsonValue parse_heterogeneous_array(const char **json) {
  JsonValue value;
  value.type = JSON_ARRAY;
  value.is_null = false;
  value.value.array.values = NULL;
  value.value.array.count = 0;

  (*json)++; // Saltar '['
  skip_whitespace(json);

  size_t capacity = 10;
  value.value.array.values = malloc(capacity * sizeof(JsonValue));

  while (**json != ']' && **json != '\0') {
    if (value.value.array.count >= capacity) {
      capacity *= 2;
      value.value.array.values =
          realloc(value.value.array.values, capacity * sizeof(JsonValue));
    }

    value.value.array.values[value.value.array.count++] = parse_any_value(json);
    skip_whitespace(json);

    if (**json == ',') {
      (*json)++;
      skip_whitespace(json);
    }
  }

  if (**json == ']')
    (*json)++;
  return value;
}

JsonValue parse_object(const char **json) {
  JsonValue value;
  value.type = JSON_OBJECT;
  value.is_null = false;
  value.value.object.members = NULL;
  value.value.object.count = 0;

  if (**json != '{') {
    fprintf(stderr, "Error: Se esperaba '{' para objeto\n");
    exit(EXIT_FAILURE);
  }

  (*json)++;
  skip_whitespace(json);

  if (**json == '}') {
    (*json)++;
    return value;
  }

  size_t capacity = 10;
  value.value.object.members =
      (JsonMember *)malloc(capacity * sizeof(JsonMember));
  if (!value.value.object.members) {
    fprintf(stderr, "Error: No se pudo asignar memoria para el objeto\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    skip_whitespace(json);

    if (value.value.object.count >= capacity) {
      capacity *= 2;
      JsonMember *temp = (JsonMember *)realloc(value.value.object.members,
                                               capacity * sizeof(JsonMember));
      if (!temp) {
        fprintf(stderr, "Error: No se pudo redimensionar el objeto\n");
        exit(EXIT_FAILURE);
      }
      value.value.object.members = temp;
    }

    if (**json != '"') {
      fprintf(stderr,
              "Error: Se esperaba comilla inicial para clave de objeto\n");
      exit(EXIT_FAILURE);
    }

    JsonValue key_value = parse_string(json);
    value.value.object.members[value.value.object.count].key =
        key_value.value.string;

    skip_whitespace(json);

    if (**json != ':') {
      fprintf(stderr, "Error: Se esperaba ':' después de clave de objeto\n");
      exit(EXIT_FAILURE);
    }

    (*json)++;
    skip_whitespace(json);

    value.value.object.members[value.value.object.count].value =
        parse_any_value(json);
    value.value.object.count++;

    skip_whitespace(json);

    if (**json == '}') {
      break;
    }

    if (**json != ',') {
      fprintf(stderr, "Error: Se esperaba ',' o '}' en objeto\n");
      exit(EXIT_FAILURE);
    }

    (*json)++;
  }

  (*json)++;
  return value;
}

JsonValue parse_any_value(const char **json) {
  JsonValue special_value;
  if (detect_special_value(json, &special_value)) {
    return special_value;
  }

  skip_whitespace(json);

  switch (**json) {
  case '{':
    return parse_object(json);
  case '[':
    return parse_heterogeneous_array(json);
  case '"':
    return parse_string(json);
  case 't':
  case 'f':
    return parse_bool(json);
  case 'n':
    return parse_null(json);
  default:
    if (**json == '-' || isdigit(**json)) {
      return parse_number(json);
    } else {
      fprintf(stderr, "Error: Valor JSON inválido\n");
      exit(EXIT_FAILURE);
    }
  }
}

JsonValue parse_json(const char **json) {
  skip_whitespace(json);
  return parse_any_value(json);
}
