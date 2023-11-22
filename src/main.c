/* main.c
 *
 * Copyright 2023 Diego Iván <diegoivan.mae@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Juego.h"
#include <stdio.h>
#include <stdlib.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int main(int argc,
         char **argv)
{
  Juego *juego = juego_nuevo();
  if (juego == NULL) {
    printf ("No se ha podido iniciar el juego\n");
    return EXIT_FAILURE;
  }
  juego_registrar_categoria(juego,
                            categoria_nueva_desde_archivo("Animales",
                                                          "recursos/animales.txt"));
  juego_registrar_categoria(juego,
                            categoria_nueva_desde_archivo("Frutas",
                                                          "recursos/frutas.txt"));
  juego_registrar_categoria(juego,
                            categoria_nueva_desde_archivo("Países",
                                                          "recursos/paises.txt"));
  juego_registrar_categoria(juego,
                            categoria_nueva_desde_archivo("Estados de México",
                                                          "recursos/estados.txt"));
  juego_iniciar_bucle (juego);
  juego_liberar(juego);
  return EXIT_SUCCESS;
}
