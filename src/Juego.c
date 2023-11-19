#include "Juego.h"
#include "Macros.h"
#include "Textura.h"
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_VIDAS 4
#define DEFAULT_N_CATEGORIAS 12

void juego_realloc_categorias(Juego *);
void juego_solicitar_categoria(Juego *);
void juego_iniciar_adivinanzas(Juego *);
bool juego_revelar_caracter(Juego *, char);
void juego_imprimir_menu(Juego *);
void juego_imprimir_partida(Juego *);
void juego_elegir_palabra(Juego *);
char char_minuscula(char);
bool juego_preguntar_continuar(Juego *);

struct __Juego {
  int vidas;
  EstadoJuego estado_actual;

  Categoria **categorias;
  size_t n_categorias;
  size_t categorias_buffer_size;

  Categoria *categoria_actual;
  char *palabra_actual;

  char *palabra_adivinada;
  bool adivinado;

  Textura *splash_textura;
  Textura *vida_textura;
  Textura *victoria_textura;
  Textura *derrota_textura;
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
  nuevo->palabra_adivinada = NULL;
  nuevo->adivinado = false;

  nuevo->splash_textura = textura_nueva_desde_archivo("recursos/splash.txt");
  nuevo->vida_textura = textura_nueva_desde_archivo ("recursos/corazon.txt");
  nuevo->victoria_textura = textura_nueva_desde_archivo("recursos/victoria.txt");
  nuevo->derrota_textura = textura_nueva_desde_archivo("recursos/derrota.txt");

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

void juego_iniciar_bucle(Juego *self)
{
  return_if_fail(self != NULL);
  clear_pantalla ();
  juego_imprimir_menu (self);

  // Esperar hasta que presione ENTER
  while(getchar() != '\n');
  clear_pantalla();

  do{
    juego_solicitar_categoria (self);
    juego_elegir_palabra (self);
    self->vidas = DEFAULT_VIDAS;

    clear_pantalla();
    juego_iniciar_adivinanzas (self);

    if (self->adivinado) {
      textura_imprimir (self->victoria_textura);
    } else {
      textura_imprimir (self->derrota_textura);
    }
  } while(juego_preguntar_continuar (self));
}

void juego_elegir_palabra(Juego *self)
{
  const char *palabra_seleccionada = NULL;
  size_t n_palabras = 0, palabra_indice = 0, palabra_len = 0;
  return_if_fail(self != NULL);

  n_palabras = categoria_get_n_palabras (self->categoria_actual);
  srand (time(NULL));
  palabra_indice = rand() % n_palabras;
  palabra_seleccionada = categoria_get_palabra (self->categoria_actual,
                                                palabra_indice);
  palabra_len = strlen(palabra_seleccionada);

  if (self->palabra_actual != NULL) {
    free (self->palabra_actual);
  }
  if (self->palabra_adivinada != NULL) {
    free(self->palabra_adivinada);
  }

  self->palabra_actual = calloc (palabra_len, sizeof(char));
  self->palabra_adivinada = calloc(palabra_len, sizeof(char));

  strcpy (self->palabra_actual, palabra_seleccionada);
  for (size_t i = 0; palabra_seleccionada[i] != 0; i++) {
    char c = palabra_seleccionada[i] == ' ' ? ' ' : '_';
    self->palabra_adivinada[i] = c;
  }
}

void juego_iniciar_adivinanzas(Juego *self)
{
  char c = 0;
  bool intento_exitoso = false;
  return_if_fail(self != NULL);
  do {
    clear_pantalla ();
    juego_imprimir_partida (self);

    printf ("Ingrese un caracter para revelar: ");
    scanf(" %c", &c);

    intento_exitoso = juego_revelar_caracter (self, c);
    if (intento_exitoso) {
      self->adivinado = strcmp(self->palabra_actual,
                               self->palabra_adivinada) == 0;
    } else {
      self->vidas--;
    }
  }while(self->vidas > 0 && !self->adivinado);
}

bool juego_revelar_caracter(Juego *self, char c)
{
  bool valido = false;
  return_val_if_fail (self != NULL, false);
  for (size_t i = 0; self->palabra_actual[i] != 0; i++)
  {
    // Significa que el usuario ya ha adivinado esta letra
    if (c == self->palabra_adivinada[i])
    {
      valido = false;
      break;
    }
    if (c == self->palabra_actual[i])
    {
      self->palabra_adivinada[i] = c;
      valido = true;
    }
  }

  return valido;
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
  printf("\n\n");
  printf("%s\n", self->palabra_adivinada);
}

bool juego_preguntar_continuar(Juego *self)
{
  char seleccion;
  return_val_if_fail (self != NULL, false);

  for (;;) {
    printf ("¿Desea iniciar una nueva partida? (s/n): ");
    scanf(" %c", &seleccion);

    seleccion = char_minuscula (seleccion);
    if (seleccion == 's' || seleccion == 'n') {
      break;
    }
    printf("Opción inválida!\n");
  }
  return seleccion == 's';
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

void juego_liberar(Juego *self)
{
  return_if_fail(self != NULL);
  if (self->categorias != NULL)
  {
    for (size_t i = 0; i < self->n_categorias; i++)
    {
      categoria_destruir (self->categorias[i]);
    }
    free(self->categorias);
  }

  if (self->palabra_actual != NULL)
  {
    free(self->palabra_actual);
  }

  if (self->palabra_adivinada != NULL)
  {
    free(self->palabra_adivinada);
  }

  if (self->splash_textura != NULL) {
    textura_liberar(self->splash_textura);
  }

  if (self->vida_textura != NULL) {
    textura_liberar(self->vida_textura);
  }

  if (self->derrota_textura != NULL) {
    textura_liberar(self->derrota_textura);
  }

  if (self->victoria_textura != NULL) {
    textura_liberar(self->victoria_textura);
  }

  free(self);
}

char char_minuscula(char c)
{
  if (c >= 65 && c <= 90) {
    return c + 32;
  }
  return c;
}