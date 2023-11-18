/* main.c
 *
 * Copyright 2023 Diego Iván <diegoivan.mae@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Categoria.h"
#include "Macros.h"
#include "Textura.h"
#include <stdio.h>
#include <stdlib.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int main(int argc,
         char **argv)
{
  Textura *textura = textura_nueva_desde_archivo("recursos/corazon.txt");
  if (textura == NULL) {
    printf("No se pudo abrir la textura");
    return EXIT_FAILURE;
  }
  textura_imprimir(textura);
  return EXIT_SUCCESS;
}
