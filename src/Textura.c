/* Textura.c
 *
 * Copyright 2023 Diego Iván
 * Copyright 2023 Juan Pablo Alquicer
 * Copyright 2023 Mariana García
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Textura.h"
#include "Macros.h"
#include <string.h>

#define BUFFER_DEFAULT 20

/**
 * Una textura es una estructura que representa a una imagen creada a partir de
 * caracteres ASCII
 */
struct __Textura {
  size_t rowstride;
  size_t altura;
  char   **datos;
  size_t buffer_size;
};

void textura_realloc(Textura *);
void textura_imprimir_linea_unsafe(Textura *, size_t);
void textura_agregar_linea(Textura *, const char *);

/**
 * Aloja espacio para más lineas en @self
 *
 * @self - La instancia a la que se le quiere alojar más espacio
 */
void textura_realloc(Textura *self)
{
  size_t nuevo_buffer_size;
  char **nuevo;
  return_if_fail(self != NULL);
  nuevo_buffer_size = self->buffer_size + BUFFER_DEFAULT;
  nuevo = calloc(nuevo_buffer_size, sizeof(char *));

  for (size_t i = 0; i < self->altura; i++) {
    nuevo[i] = self->datos[i];
  }
  free(self->datos);
  self->datos = nuevo;
  self->buffer_size = nuevo_buffer_size;
}

/**
 * Crea una textura nueva a partir de @camino, un archivo de texto plano
 * válido
 *
 * @camino Un camino válido a un archivo de texto plano válido
 *
 * Returns: La nueva textura creada a partir del archivo, o NULL en caso de
 * haber fallado
 */
Textura *textura_nueva_desde_archivo(const char *camino)
{
  FILE *stream = NULL;
  char *linea = NULL;
  size_t caracteres = 0, size = 0;
  Textura *self = NULL;
  return_val_if_fail(camino != NULL, NULL);

  stream = fopen(camino, "r");
  return_val_if_fail(stream != NULL, NULL);

  self = malloc(sizeof(Textura));
  self->altura = 0;
  self->rowstride = 0;
  self->buffer_size = BUFFER_DEFAULT;
  self->datos = calloc(BUFFER_DEFAULT, sizeof(char *));

  while ((caracteres = getline(&linea, &size, stream)) != -1)
  {
    if (caracteres > self->rowstride) {
      self->rowstride = caracteres;
    }

    if (caracteres > 0 && linea[caracteres - 1] == '\n') {
      linea[caracteres - 1] = 0;
    }
    textura_agregar_linea(self, linea);
  }
  free(linea);

  return self;
}

/**
 * Retorna el número de caracteres de @self por linea
 *
 * @self La instancia de una textura
 *
 * Returns: El numero de caracteres por linea de @self
 */
int textura_get_rowstride(Textura *self)
{
  return_val_if_fail(self != NULL, -1);
  return self->rowstride;
}

/**
 * Retorna la altura de @self
 *
 * @self La instancia de una textura
 *
 * Returns: La altura de @self
 */
int textura_get_altura(Textura *self)
{
  return_val_if_fail(self != NULL, -1);
  return self->altura;
}

/**
 * Retorna la linea numero @indice de @self
 *
 * @self La instancia de una textura
 * @indice La posicion de la linea que se quiere obtener
 *
 * Returns: La linea @indice de @self o NULL en caso de @indice invalido
 */
const char *textura_get_linea(Textura *self,
                              size_t indice)
{
  return_val_if_fail(self != NULL, NULL);
  return_val_if_fail(indice < self->altura, NULL);
  return self->datos[indice];
}

/**
 * Imprime la linea numero @indice de @self
 *
 * @self La instancia de una textura
 * @indice La posicion de la linea que se quiere imprimir
 */
void textura_imprimir_linea(Textura *self,
                            size_t   indice)
{
  return_if_fail (self != NULL);
  return_if_fail (indice < self->altura);
  textura_imprimir_linea_unsafe(self, indice);
}

void textura_imprimir_linea_unsafe(Textura *self,
                                   size_t   indice)
{
  size_t i = 0;
  for (; self->datos[indice][i] != '\0'; i++) {
    putchar(self->datos[indice][i]);
  }
  for (; i < self->rowstride; i++) {
    putchar(' ');
  }
}

/**
 * Imprime la imagen contenida en @self
 *
 * @self - La instancia que se desea imprimir
 */
void textura_imprimir(Textura *self)
{
  size_t fila = 0;
  return_if_fail(self != NULL);
  for (; fila < self->altura; fila++) {
    textura_imprimir_linea_unsafe(self, fila);
    putchar('\n');
  }
}

void textura_agregar_linea(Textura *self,
                           const char *linea)
{
  return_if_fail(self != NULL);
  return_if_fail (linea != NULL);
  if (self->altura >= self->buffer_size) {
    textura_realloc(self);
  }
  self->datos[self->altura] = strdup (linea);
  self->altura++;
}

/**
 * Libera la información contenida en @self
 *
 * @self La instancia que se quiera liberar
 */
void textura_liberar(Textura *self)
{
  return_if_fail (self != NULL);
  for (size_t i = 0; i < self->altura; i++) {
    free(self->datos[i]);
  }
  free(self->datos);
  free(self);
}
