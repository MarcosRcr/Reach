#include "../includes/code_generator.h"
#include "../includes/json_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prototipos para las nuevas funciones
void compile_for_linux(const char *filename);
void compile_for_windows(const char *filename);
void show_help();

int main(int argc, char *argv[]) {
  // Modo compilación (-cl o -cx)
  if (argc == 3 &&
      (strcmp(argv[1], "-cl") == 0 || strcmp(argv[1], "-cx") == 0)) {
    if (strcmp(argv[1], "-cl") == 0) {
      compile_for_linux(argv[2]);
    } else {
      compile_for_windows(argv[2]);
    }
    return 0;
  }

  // Modo conversión JSON a C (comportamiento original)
  if (argc == 3) {
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
    printf("Puedes compilarlo con:\n");
    printf("  reach -cl %s  # Para Linux\n", output_filename);
    printf("  reach -cx %s  # Para Windows\n", output_filename);

    free(json_str);
    free_json_value(&json);

    return 0;
  }

  // Si no coinciden los parámetros, mostrar ayuda
  show_help();
  return 1;
}

void compile_for_linux(const char *filename) {
  char command[512];
  // Extraer nombre base sin extensión .c
  char basename[256];
  strncpy(basename, filename, strlen(filename) - 2);
  basename[strlen(filename) - 2] = '\0';

  snprintf(command, sizeof(command), "gcc %s -o %s && ./%s", filename, basename,
           basename);

  printf("Compilando para Linux: %s\n", command);
  int result = system(command);
  if (result != 0) {
    fprintf(stderr, "Error al compilar para Linux\n");
    exit(EXIT_FAILURE);
  }
}

void compile_for_windows(const char *filename) {
  char command[512];
  char basename[256];
  strncpy(basename, filename, strlen(filename) - 2);
  basename[strlen(filename) - 2] = '\0';

  snprintf(command, sizeof(command),
           "x86_64-w64-mingw32-gcc %s -o %s.exe && wine %s.exe", filename,
           basename, basename);

  printf("Compilando para Windows: %s\n", command);
  int result = system(command);
  if (result != 0) {
    fprintf(stderr, "Error al compilar para Windows\n");
    exit(EXIT_FAILURE);
  }
}

void show_help() {
  printf("Uso:\n");
  printf("  Modo conversión JSON a C:\n");
  printf("    reach archivo.json nombre_estructura\n");
  printf("    Ejemplo: reach datos.json Persona\n\n");
  printf("  Modo compilación:\n");
  printf("    reach -cl archivo.c  # Compilar y ejecutar para Linux\n");
  printf("    reach -cx archivo.c  # Compilar y ejecutar para Windows\n");
}
