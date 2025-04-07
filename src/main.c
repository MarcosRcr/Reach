#include "../includes/code_generator.h"
#include "../includes/json_parser.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Uso: %s archivo.json nombre_estructura\n", argv[0]);
    printf("Ejemplo: %s datos.json Persona\n", argv[0]);
    return 1;
  }

  const char *filename = argv[1];
  const char *struct_name = argv[2];

  char *json_str = read_json_file(filename);
  const char *ptr = json_str;

  printf("Analizando archivo JSON: %s\n", filename);
  JsonValue json = parse_json(&ptr);

  printf("\nJSON parseado:\n");
  print_json_value(&json, 0);
  printf("\n");

  char output_filename[256];
  snprintf(output_filename, sizeof(output_filename), "%s_generated.c",
           struct_name);

  FILE *output = fopen(output_filename, "w");
  if (!output) {
    fprintf(stderr, "Error: No se pudo crear el archivo de salida\n");
    free(json_str);
    free_json_value(&json);
    return 1;
  }

  generate_c_code(&json, struct_name, output);
  fclose(output);

  printf("\nCódigo C generado en: %s\n", output_filename);

  free(json_str);
  free_json_value(&json);

  return 0;
}
