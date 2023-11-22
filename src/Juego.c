/* Juego.c
 *
 * Copyright 2023 Diego Iván
 * Copyright 2023 Juan Pablo Alquicer
 * Copyright 2023 Mariana García
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Juego.h"
#include "Macros.h"
#include "Textura.h"
#include "Unicode.h"
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_VIDAS 5
#define DEFAULT_N_CATEGORIAS 12

/**
 * Enumeración que define los tipos de intento que puede realizar el usuario
 * dentro del rango (TIPO_0, N_TIPOS)
 */
typedef enum {
  TIPO_0,
  TIPO_CARACTER,
  TIPO_PALABRA,
  N_TIPOS
} TipoIntento;
const char *tipo_intento_to_string(TipoIntento);

void juego_realloc_categorias(Juego *);
void juego_solicitar_categoria(Juego *);
void juego_iniciar_adivinanzas(Juego *);
bool juego_revelar_caracter(Juego *, const char *, size_t, bool);
TipoIntento juego_solicitar_tipo_intento(void);
void juego_imprimir_menu(Juego *);
void juego_imprimir_partida(Juego *);
void juego_elegir_palabra(Juego *);
void juego_imprimir_palabra_adivinada (Juego *);
bool juego_preguntar_continuar(void);

/**
 * Estructura que almacena los datos relacionados al juego
 */
struct __Juego {
  int vidas;
  EstadoJuego estado_actual;

  Categoria **categorias;
  size_t n_categorias;
  size_t categorias_buffer_size;

  Categoria *categoria_actual;
  char *palabra_actual;
  size_t palabra_len;

  char *palabra_adivinada;
  bool adivinado;

  Textura *splash_textura;
  Textura *vida_textura;
  Textura *victoria_textura;
  Textura *derrota_textura;
};

/**
 * Crea una nueva instancia de un juego
 *
 * Returns: Nueva instancia de Juego o NULL en caso de haber fallado
 */
Juego *juego_nuevo(void) {
  Juego *nuevo = malloc(sizeof(Juego));

  nuevo->categorias = calloc(DEFAULT_N_CATEGORIAS, sizeof(Categoria *));
  nuevo->n_categorias = 0;
  nuevo->categorias_buffer_size = DEFAULT_N_CATEGORIAS;

  nuevo->vidas = DEFAULT_VIDAS;
  nuevo->estado_actual = MENU;

  nuevo->categoria_actual = NULL;
  nuevo->palabra_actual = NULL;
  nuevo->palabra_len = 0;
  nuevo->palabra_adivinada = NULL;
  nuevo->adivinado = false;

  nuevo->splash_textura = textura_nueva_desde_archivo("recursos/splash.txt");
  nuevo->vida_textura = textura_nueva_desde_archivo ("recursos/corazon.txt");
  nuevo->victoria_textura = textura_nueva_desde_archivo("recursos/victoria.txt");
  nuevo->derrota_textura = textura_nueva_desde_archivo("recursos/derrota.txt");

  return nuevo;
}

/**
 * Añade @categoria al listado de categíoras de @self para que pueda ser usada
 * en rondas
 *
 * @self La instancia a la que se le quiera agregar la categoría
 *
 * @ categoria: La categoria que se quiera añadir a @self
 */
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

/**
 * Procedimiento que aloja más espacio para el listado interno de categorías
 */
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

/**
 * Inicia el bucle de juego, que termina hasta que el usuario desea terminar
 * la ejecución del programa
 *
 * @self La instancia del juego que se quiera iniciar
 */
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

    clear_pantalla ();
    if (self->adivinado) {
      textura_imprimir (self->victoria_textura);
    } else {
      textura_imprimir (self->derrota_textura);
      printf ("La palabra era: %s\n", self->palabra_actual);
    }
  } while(juego_preguntar_continuar ());
}

/**
 * Procedimiento que elije una palabra aleatoria para la categoria actual
 *
 * @self La instancia del juego
 */
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
  palabra_len = self->palabra_len = strlen(palabra_seleccionada);

  if (self->palabra_actual != NULL) {
    free (self->palabra_actual);
    self->palabra_actual = NULL;
  }
  if (self->palabra_adivinada != NULL) {
    free(self->palabra_adivinada);
    self->palabra_adivinada = NULL;
  }

  self->palabra_actual = strdup (palabra_seleccionada);
  self->palabra_adivinada = calloc(palabra_len, sizeof(char));

  for (size_t i = 0; palabra_seleccionada[i] != 0; i++) {
    char c = palabra_seleccionada[i] == ' ' ? ' ' : '_';
    self->palabra_adivinada[i] = c;
  }
}

/**
 * Procedimiento que inicia el bucle de adivinanzas del usuario, que se
 * detiene hasta que el usuario haya perdido todas sus vidas o cuando
 * haya adivinado la palabra correcta
 *
 * @self La instancia del juego
 */
void juego_iniciar_adivinanzas(Juego *self)
{
  char str[100];
  char *primer_caracter = NULL;
  size_t c_len = 0;
  TipoIntento tipo_intento;
  return_if_fail(self != NULL);
  do {
    clear_pantalla ();
    juego_imprimir_partida (self);

    tipo_intento = juego_solicitar_tipo_intento ();
    switch(tipo_intento)
    {
    case TIPO_PALABRA:
      printf ("Ingrese la palabra: ");
      // https://stackoverflow.com/questions/1247989/how-do-you-allow-spaces-to-be-entered-using-scanf
      // Esta sola línea de código acabó con mi paciencia, y su único propósito
      // es permitir espacio en scanf...
      scanf(" %99[^\n]", str);

      self->adivinado = strcasecmp (self->palabra_actual, str) == 0;
      if (!self->adivinado) {
        self->vidas--;
      }
      break;
    case TIPO_CARACTER:
      printf ("Ingrese el caracter: ");
      scanf(" %99s", str);

      /**
       * Desafortunadamente, no podemos utilizar caracteres ASCII para español,
       * ya que palabras con acento y la ñ no se revelarán correctamente si es
       * que el usuario la adivina. Tenemos que utilizar la codificación
       * UTF-8 de las cadenas en C para que funcione
       *
       * https://dev.to/rdentato/utf-8-strings-in-c-1-3-42a4
       */
      primer_caracter = u8_construir_primer_caracter (str, &c_len);

      if (juego_revelar_caracter (self, primer_caracter, c_len, false)) {
        self->adivinado = strcasecmp (self->palabra_actual,
                                      self->palabra_adivinada) == 0;
      } else {
        self->vidas--;
      }
      free (primer_caracter);
      primer_caracter = NULL;
      break;
    case TIPO_0:
    case N_TIPOS:
    default:
      break;
    }
  }while(self->vidas > 0 && !self->adivinado);
}

/**
 * Función que solicita un tipo de intento al usuario
 *
 * Returns: Un tipo de intento válido de la enumeración TipoIntento.
 */
TipoIntento juego_solicitar_tipo_intento(void)
{
  int seleccion = 0;
  for (;;)
  {
    printf ("Ingrese el tipo de intento que quiere realizar:\n");
    for (TipoIntento tipo = TIPO_0 + 1; tipo < N_TIPOS; tipo++) {
      printf ("%d. %s\n", tipo, tipo_intento_to_string (tipo));
    }
    scanf ("%d", &seleccion);
    if (seleccion > TIPO_0 && seleccion < N_TIPOS)
      break;
    printf ("Opción Inválida!\n");
  }
  return (TipoIntento)seleccion;
}

/**
 * Función que intenta revelar @u8_c en la palabra a adivinar
 *
 * @self La instancia del juego
 * @u8_c Un caracter UTF-8 válido
 * @c_len La longitud de @u8_c
 */
bool juego_revelar_caracter(Juego      *self,
                            const char *u8_c,
                            size_t      c_len,
                            bool        es_alt)
{
  int valido = false;
  size_t alt_len;
  const char *alt;
  return_val_if_fail (self != NULL, false);

  for (size_t i = 0; i < self->palabra_len; i++)
  {
    // Significa que el caracter ya fue adivinado
    if (strncasecmp (&self->palabra_adivinada[i], u8_c, c_len) == 0)
    {
      valido = false;
      break;
    }
    // u8_c está en el string
    if (strncasecmp (&self->palabra_actual[i], u8_c, c_len) == 0)
    {
      strncpy (&self->palabra_adivinada[i], &self->palabra_actual[i], c_len);
      valido = true;
    }
  }

  // Vamos a revelar los caracteres equivalentes, como caracteres cono acento/
  // sin acento, Ñ...
  if (!es_alt)
  {
    alt = u8_get_caracter_equivalente_minuscula (u8_c, &alt_len);
    if (alt != NULL) {
      valido += juego_revelar_caracter (self, alt, alt_len, true);
    }
    alt = u8_get_caracter_equivalente_mayuscula (u8_c, &alt_len);
    if (alt != NULL) {
      valido += juego_revelar_caracter (self, alt, alt_len, true);
    }
    if (!ES_ASCII (u8_c[0])) {
      alt = u8_get_ascii_equivalente (u8_c);
      if (alt != NULL) {
        valido += juego_revelar_caracter (self, alt, 1, true);
      }
    }
  }

  return valido;
}

/**
 * Imprime el menú de @self
 *
 * @self - La instancia de Juego
 */
void juego_imprimir_menu(Juego *self)
{
  return_if_fail(self != NULL);
  textura_imprimir(self->splash_textura);
  printf("\n\n\nPRESIONE ENTER PARA COMENZAR\n\n\n");
}

/**
 * Solicita al usuario alguna de las categorías registradas en @self
 *
 * @self - La instancia del juego
 */
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

/**
 * Imprime el status actual de la partida, con número de vidas y el progreso
 * para adivinar la palabra
 *
 * @self La instancia del juego
 */
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
  juego_imprimir_palabra_adivinada (self);
}

/**
 * Imprime el progreso del usuario para adivinar la palabra seleccionada
 * Desfortunadamente, por la codificación de las cadenas en C, no podemos
 * confiar en solo imprimir los guiones que sustituyen a las letras
 * pendientes, ya que estas pueden estar codificadas en UTF-8 y por tanto
 * ocupar más de un caracter. Solo imprimiremos el guión en los siguientes
 * casos:
 *
 * 1. El caracter en la palabra seleccionada es ASCII
 *
 * 2. El caracter de la palabra adivinada es ASCII
 *
 * 3. El caracter de la palabra adivinada es el primer caracter de un caracter
 * UTF-8
 *
 * 4. El caracter es UTF-8 pero ya fue adivinado por el usuario
 */
void juego_imprimir_palabra_adivinada (Juego *self)
{
  return_if_fail (self != NULL);
  for (size_t i = 0; i < self->palabra_len; i++) {
    char c_adivinado = self->palabra_adivinada[i];
    char c_actual = self->palabra_actual[i];
    if ((PRIMER_U8 (c_actual) || ES_ASCII(c_actual)) || PARTE_U8 (c_adivinado)) {
      putchar (c_adivinado);
    }
  }
  putchar('\n');
}

/**
 * Pregunta al usuario si desea continuar jugando
 */
bool juego_preguntar_continuar(void)
{
  char seleccion;
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

/**
 * Retorna el número de vidas que tiene el usuario
 *
 * @self El número de vidas de @self
 */
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

/**
 * Libera la memoria utilizada por el juego
 *
 * @self La instancia que se desea liberar de memoria
 */
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

/**
 * Retorna la representación en cadena de caracteres de @tipo
 *
 * @tipo El tipo de intento que se quiere convertir a cadena de caracteres
 *
 * Returns: (transfer: none) La representación en cadena de caracteres de @tipo
 */
const char *tipo_intento_to_string(TipoIntento tipo)
{
  switch(tipo){
  case TIPO_PALABRA:
    return "Adivinar Palabra";
  case TIPO_CARACTER:
    return "Adivinar Carácter";
  case TIPO_0:
  case N_TIPOS:
  default:
    return NULL;
  }
}
