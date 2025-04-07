UNAME_S := $(shell uname -s)

CC=gcc
CC_WIN=x86_64-w64-mingw32-gcc
SRC=src
INC=includes
OBJS=$(SRC)/main.c $(SRC)/json_parser.c $(SRC)/json_value.c $(SRC)/code_generator.c $(SRC)/utils.c
CFLAGS=-I$(INC) -Wall -Wextra

ifeq ($(OS),Windows_NT)
	IS_WINDOWS := 1
else
	IS_WINDOWS := 0
endif

# Targets por defecto
all: reach
ifeq ($(IS_WINDOWS),1)
	$(MAKE) reach.exe
else
	$(MAKE) reach
endif

# Construir el ejecutable principal
reach: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o reach

# Construir el ejecutable de Windows
reach.exe: $(OBJS)
	$(CC_WIN) $(CFLAGS) $(OBJS) -o reach.exe

# Instalación para Linux
install:
ifeq ($(IS_WINDOWS),0)
	cp reach /usr/local/bin/
	chmod +x /usr/local/bin/reach
	@echo "Instalado en /usr/local/bin. Puedes usar 'reach' directamente."
else
	@echo "En Windows, copia manualmente 'reach.exe' a una carpeta en tu PATH."
endif

# Limpiar los archivos generados
clean:
	rm -f reach reach.exe
	rm -f *.exe
	rm -f *_generated.c

