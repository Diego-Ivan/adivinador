#include "Juego.h"
#include "Macros.h"
#include "Textura.h"
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_VIDAS 4
#define DEFAULT_N_CATEGORIAS 12

void juego_realloc_categorias(Juego *);
void juego_solicitar_categoria(Juego *);
void juego_imprimir_menu(Juego *);
void juego_imprimir_partida(Juego *);
void juego_set_palabra_seleccionada(Juego *, const char *);

struct __Juego {
  int vidas;
  EstadoJuego estado_actual;

  Categoria **categorias;
  size_t n_categorias;
  size_t categorias_buffer_size;

  Categoria *categoria_actual;
  char *palabra_actual;

  char *caracteres_encontrados;
  size_t n_caracteres_encontrados;

  Textura *splash_textura;
  Textura *vida_textura;
};

Juego *juego_nuevo(void) {
  Juego *nuevo = malloc(sizeof(Juego));

  nuevo->categorias = calloc(DEFAULT_N_CATEGORIAS, sizeof(Categoria *));
  nuevo->n_categorias = 0;
  nuevo->categorias_buffer_size = DEFAULT_N_CATEGORIAS;

  nuevo->vidas = DEFAULT_VIDAS;
  nuevo->estado_actual = MENU;

  nuevo->categoria_actual = NULL;
  nuevo->palabra_actual = NULL;
  nuevo->caracteres_encontrados = NULL;
  nuevo->n_caracteres_encontrados = 0;


  nuevo->splash_textura = textura_nueva_desde_archivo("recursos/splash.txt");
  nuevo->vida_textura = textura_nueva_desde_archivo ("recursos/corazon.txt");

  return nuevo;
}

void juego_registrar_categoria(Juego     *self,
                               Categoria *categoria)
{
  return_if_fail(self != NULL);
  return_if_fail(categoria != NULL);
  if (self->n_categorias >= self->categorias_buffer_size) {
    juego_realloc_categorias(self);
  }
  self->categorias[self->n_categorias] = categoria;
  self->n_categorias++;
}

void juego_realloc_categorias(Juego *self)
{
  size_t nuevo_size;
  Categoria **nuevo_arreglo = NULL;
  return_if_fail(self != NULL);
  nuevo_size = self->categorias_buffer_size + DEFAULT_N_CATEGORIAS;
  nuevo_arreglo = calloc(nuevo_size, sizeof(Categoria *));

  for (size_t i = 0; i < self->n_categorias; i++) {
    nuevo_arreglo[i] = self->categorias[i];
  }
  free(self->categorias);
  self->categorias = nuevo_arreglo;
  self->categorias_buffer_size = nuevo_size;
}

void juego_empezar(Juego *self)
{
  return_if_fail(self != NULL);
  clear_pantalla ();
  juego_imprimir_menu (self);

  // Esperar hasta que presione ENTER
  while(getchar() != '\n');
  clear_pantalla();

  juego_solicitar_categoria (self);
  clear_pantalla();
  juego_imprimir_partida(self);
}

void juego_imprimir_menu(Juego *self)
{
  return_if_fail(self != NULL);
  textura_imprimir(self->splash_textura);
  printf("\n\n\nPRESIONE ENTER PARA COMENZAR\n\n\n");
}

void juego_solicitar_categoria(Juego *self) {
  int seleccion;
  return_if_fail(self != NULL);

  for (;;) {
    printf("Seleccione la categoría con la que quiera jugar:\n");
    for (size_t i = 0; i < self->n_categorias; i++) {
      Categoria *categoria = self->categorias[i];
      printf("%lu. %s\n", i + 1, categoria_get_nombre(categoria));
    }
    scanf("%d", &seleccion);
    if (seleccion > 0 && seleccion <= self->n_categorias) {
      break;
    }
    printf("Opción inválida!\n");
  }
  self->categoria_actual = self->categorias[seleccion - 1];
}

void juego_imprimir_partida(Juego *self)
{
  size_t altura_textura;
  return_if_fail(self != NULL);

  altura_textura = textura_get_altura (self->vida_textura);
  printf ("Tus vidas:\n\n");
  for (size_t linea = 0; linea < altura_textura; linea++)
  {
    for (size_t i = 0; i < self->vidas; i++) {
      textura_imprimir_linea(self->vida_textura, linea);
    }
    putchar('\n');
  }
}

int juego_get_vidas(Juego *self)
{
  return_val_if_fail (self != NULL, -1);
  return self->vidas;
}

EstadoJuego juego_get_estado(Juego *self)
{
  return_val_if_fail(self != NULL, -1);
  return self->estado_actual;
}
