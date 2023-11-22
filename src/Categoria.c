/* Categoria.c
 *
 * Copyright 2023 Diego Iván
 * Copyright 2023 Juan Pablo Alquicer
 * Copyright 2023 Mariana García
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Categoria.h"
#include "Macros.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Este será el tamaño que tendrá el arreglo de palabras. Lo haremos un número
 * relativamente grande, porque si no tendríamos que realojar muchas veces,
 * y eso es una operación muy cara. Mejor nos ahorramos eso aunque tengamos
 * un poquillo de overhead.
 */
#define DEFAULT_N_PALABRAS 32

struct __Categoria {
  char *nombre;
  char **palabras;
  size_t n_palabras;
  size_t buffer_size;
};

void categoria_realloc(Categoria *);

/**
 * Función que crea una nueva categoría de nombre @nombre
 *
 * @nombre El nombre de la categoría
 *
 * Returns: una categoría nueva
 */
Categoria *categoria_nueva(const char *nombre) {
  Categoria *nueva;
  return_val_if_fail(nombre != NULL, NULL);

  nueva = malloc(sizeof(Categoria));
  nueva->nombre = strdup (nombre);

  nueva->palabras = calloc(DEFAULT_N_PALABRAS, sizeof(char *));
  nueva->n_palabras = 0;
  nueva->buffer_size = DEFAULT_N_PALABRAS;

  return nueva;
}

/**
 * Función que crea una nueva categoría de nombre @nombre a partir de las
 * palabras de @archivo

 * @nombre El nombre de la categoría
 *
 * @archivo El camino al archivo
 *
 * Returns: una categoría nueva
 */
Categoria *categoria_nueva_desde_archivo(const char *nombre,
                                         const char *archivo) {
  Categoria *nueva = NULL;
  char *palabra = NULL;
  long caracteres;
  size_t palabra_size;
  FILE *stream;

  return_val_if_fail(nombre != NULL, NULL);
  return_val_if_fail(archivo != NULL, NULL);

  stream = fopen(archivo, "r");

  return_val_if_fail(stream != NULL, NULL);
  nueva = categoria_nueva(nombre);

  while ((caracteres = getline(&palabra, &palabra_size, stream)) != -1) {
    // si el último caracter antes del nulo es \n, hay que quitarlo para
    // sanitizar la palabra y que nos sirva para el juego lets gooo
    if (palabra[caracteres - 1] == '\n') {
      palabra[caracteres - 1] = 0;
    }
    categoria_registrar_palabra(nueva, palabra, palabra_size);
  }

  fclose(stream);
  free(palabra);

  return nueva;
}

/**
 * Registra @palabra en @self
 *
 * @self La categoría
 *
 * @palabra La palabra a registrar
 *
 * @palabra_size La longitud de la palabra, o -1 si @palabra termina en NUL
 */
void categoria_registrar_palabra(Categoria *self, const char *palabra,
                                 int palabra_size) {
  char *copia_palabra;
  return_if_fail(self != NULL);
  return_if_fail(palabra != NULL);

  if (self->n_palabras >= self->buffer_size) {
    categoria_realloc(self);
  }

  if (palabra_size < 0) {
    palabra_size = strlen(palabra);
  }

  copia_palabra = calloc(palabra_size, sizeof(char));
  strcpy(copia_palabra, palabra);

  self->palabras[self->n_palabras] = copia_palabra;
  self->n_palabras++;
}

/**
 * Añade espacios al arreglo interno para que puedan haber más palabras
 */
void categoria_realloc(Categoria *self) {
  char **anterior, **nuevo;
  size_t anterior_size, nuevo_size;
  return_if_fail(self != NULL);

  anterior = self->palabras;
  anterior_size = self->buffer_size;
  nuevo_size = anterior_size + DEFAULT_N_PALABRAS;

  nuevo = calloc(nuevo_size, sizeof(char *));
  for (size_t i = 0; i < anterior_size; i++) {
    nuevo[i] = anterior[i];
  }

  self->palabras = nuevo;
  self->buffer_size = nuevo_size;

  free(anterior);
}

/**
 * Obtiene la palabra @indice dentro de @self
 *
 * @self La categoría
 *
 * @indice El índice de la palabra
 *
 * Returns: (transfer: None) La palabra @indice de @self ó -1 si @indice no es válido
 */
const char *categoria_get_palabra(Categoria *self, int indice) {
  return_val_if_fail(self != NULL, NULL);
  return_val_if_fail(indice >= 0, NULL);

  return self->palabras[indice];
}

/**
 * Obtiene el número de palabras de @self
 *
 * @self La categoría
 *
 * Returns: El numero de palabras en @self ó -1 si @self es NULL
 */
int categoria_get_n_palabras(Categoria *self) {
  return_val_if_fail(self != NULL, -1);
  return self->n_palabras;
}

/**
 * Obteiene el nombre de @self
 *
 * @self La categoría
 *
 * Returns: (transfer: none) El nombre de @self
 */
const char *categoria_get_nombre(Categoria *self) {
  return_val_if_fail(self != NULL, NULL);
  return self->nombre;
}

void categoria_destruir(Categoria *self) {
  return_if_fail(self != NULL);
  for (size_t i = 0; i < self->n_palabras; i++) {
    free(self->palabras[i]);
  }
  free(self->palabras);
  free(self->nombre);
  free(self);
}

