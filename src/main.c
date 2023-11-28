/* main.c
 *
 * Copyright 2023 Diego Iván <diegoivan.mae@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Textura.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define MAX_CATEGORIAS 10
#define DEFAULT_VIDAS 5

#define clear_pantalla() system("clear")

/*
 * Nos ayudará a reconocer caracteres codificados en UTF-8 en vez de ASCII
 *
 * Macros implementadas gracias a: https://dev.to/rdentato/utf-8-strings-in-c-1-3-42a4
 */
#define PRIMER_U8(c) ((c & 0xC0) == 0xC0)
#define PARTE_U8(c) ((c & 0xC0) == 0x80)
#define ES_ASCII(c) (c >= 0)

int char_minuscula(int);
char *u8_construir_primer_caracter(const char *, size_t *);
const char *u8_get_caracter_equivalente_minuscula(const char *, size_t *);
const char *u8_get_caracter_equivalente_mayuscula(const char *, size_t *);
const char *u8_get_ascii_equivalente(const char *);

/* Inician declaraciones de las categorias */
struct __Categoria;
typedef struct __Categoria Categoria;

Categoria *categoria_nueva(const char *nombre);
Categoria *categoria_nueva_desde_archivo(const char *, const char *);
const char *categoria_get_nombre(Categoria *);
void categoria_registrar_palabra(Categoria *, const char *, int);
const char *categoria_get_palabra(Categoria *, unsigned int);
int categoria_get_n_palabras(Categoria *);
void categoria_destruir(Categoria *);

/* Terminan declaraciones de las categorías */

/* Inician declaraciones del juego */

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

int vidas, n_categorias, palabra_len;
Categoria *categorias[MAX_CATEGORIAS], *categoria_actual;
char *palabra_actual, *palabra_adivinada;
bool adivinado;
Textura *splash_textura, *vida_textura, *victoria_textura, *derrota_textura;

void inicializar (void);
void juego_finalizar(void);
void agregar_categoria (Categoria *);
void iniciar_bucle_juego (void);
bool juego_preguntar_continuar (void);
void juego_solicitar_categoria(void);
void juego_elegir_palabra(void);
void juego_imprimir_menu(void);
void juego_imprimir_partida(void);
const char *tipo_intento_to_string(TipoIntento);
TipoIntento juego_solicitar_tipo_intento(void);
void juego_imprimir_palabra_adivinada (void);
bool juego_revelar_caracter(const char *, size_t, bool);
void juego_iniciar_adivinanzas(void);

/* Terminan declaraciones del juego */

int main(int argc,
         char **argv)
{
  inicializar ();
  iniciar_bucle_juego ();
  juego_finalizar ();
  return EXIT_SUCCESS;
}

/* Inicia código del juego */
void inicializar (void)
{
  n_categorias = 0;

  vidas = DEFAULT_VIDAS;

  categoria_actual = NULL;
  palabra_actual = NULL;
  palabra_len = 0;
  palabra_adivinada = NULL;
  adivinado = false;

  splash_textura = textura_nueva_desde_archivo("recursos/splash.txt");
  vida_textura = textura_nueva_desde_archivo ("recursos/corazon.txt");
  victoria_textura = textura_nueva_desde_archivo("recursos/victoria.txt");
  derrota_textura = textura_nueva_desde_archivo("recursos/derrota.txt");

  agregar_categoria(categoria_nueva_desde_archivo("Animales", "recursos/animales.txt"));
  agregar_categoria(categoria_nueva_desde_archivo("Frutas", "recursos/frutas.txt"));
  agregar_categoria(categoria_nueva_desde_archivo("Países","recursos/paises.txt"));
  agregar_categoria(categoria_nueva_desde_archivo("Estados de México",
                                                  "recursos/estados.txt"));
}

void agregar_categoria (Categoria *nueva_categoria)
{
  if (nueva_categoria == NULL || n_categorias >= MAX_CATEGORIAS)
    {
      printf ("No se puede agregar categoria.\n");
      return;
    }
  categorias[n_categorias] = nueva_categoria;
  n_categorias++;
}

/**
 * Inicia el bucle de juego, que termina hasta que el usuario desea terminar
 * la ejecución del programa
 */
void iniciar_bucle_juego (void)
{
  clear_pantalla ();
  clear_pantalla ();
  juego_imprimir_menu ();

  // Esperar hasta que presione ENTER
  while(getchar() != '\n');
  clear_pantalla();

  do{
    adivinado = false;
    juego_solicitar_categoria ();
    juego_elegir_palabra ();
    vidas = DEFAULT_VIDAS;

    clear_pantalla();
    juego_iniciar_adivinanzas ();

    clear_pantalla ();
    if (adivinado) {
      textura_imprimir (victoria_textura);
    } else {
      textura_imprimir (derrota_textura);
      printf ("La palabra era: %s\n", palabra_actual);
    }
  } while(juego_preguntar_continuar ());
}

void juego_imprimir_menu(void)
{
  textura_imprimir(splash_textura);
  printf("\n\n\nPRESIONE ENTER PARA COMENZAR\n\n\n");
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
    if (seleccion == 's' || seleccion == 'n')
      {
        break;
      }
    printf("Opción inválida!\n");
  }
  return seleccion == 's';
}

/**
 * Solicita al usuario alguna de las categorías registradas
 */
void juego_solicitar_categoria(void)
{
  int seleccion;

  for (;;) {
    printf("Seleccione la categoría con la que quiera jugar:\n");
    for (size_t i = 0; i < n_categorias; i++) {
      Categoria *categoria = categorias[i];
      printf("%lu. %s\n", i + 1, categoria_get_nombre(categoria));
    }
    scanf("%d", &seleccion);
    if (seleccion > 0 && seleccion <= n_categorias) {
      break;
    }
    printf("Opción inválida!\n");
  }
  categoria_actual = categorias[seleccion - 1];
}

/**
 * Procedimiento que elije una palabra aleatoria para la categoria actual
 *
 * @self La instancia del juego
 */
void juego_elegir_palabra(void)
{
  const char *palabra_seleccionada = NULL;
  size_t n_palabras = 0, palabra_indice = 0;

  n_palabras = categoria_get_n_palabras (categoria_actual);
  srand (time(NULL));
  palabra_indice = rand() % n_palabras;
  palabra_seleccionada = categoria_get_palabra (categoria_actual,
                                                palabra_indice);
  palabra_len = strlen(palabra_seleccionada);

  /*
   * Vamos a hacer copias de las palabras que seleccionemos aleatoriamente,
   * como estas están alojadas en el heap, tenemos que liberarlas cuando ya
   * no las necesitamos.
   */
  if (palabra_actual != NULL) {
    free (palabra_actual);
    palabra_actual = NULL;
  }
  if (palabra_adivinada != NULL) {
    free(palabra_adivinada);
    palabra_adivinada = NULL;
  }

  palabra_actual = strdup (palabra_seleccionada);
  palabra_adivinada = calloc(palabra_len, sizeof(char));

  /* Ahora que ya alojamos espacio para la palabra seleccionada en el heap
   * vamos a reemplazar todos los caracteres por guiones bajos, menos si son
   * espacios. Así será más fácil imprimirlos
   */
  for (size_t i = 0; palabra_seleccionada[i] != 0; i++) {
    char c = palabra_seleccionada[i] == ' ' ? ' ' : '_';
    palabra_adivinada[i] = c;
  }
}

/**
 * Procedimiento que inicia el bucle de adivinanzas del usuario, que se
 * detiene hasta que el usuario haya perdido todas sus vidas o cuando
 * haya adivinado la palabra correcta
 *
 * @self La instancia del juego
 */
void juego_iniciar_adivinanzas(void)
{
  char str[100];
  char *primer_caracter = NULL;
  size_t c_len = 0;
  TipoIntento tipo_intento;
  do {
    clear_pantalla ();
    juego_imprimir_partida ();

    tipo_intento = juego_solicitar_tipo_intento ();
    switch(tipo_intento)
    {
    case TIPO_PALABRA:
      printf ("Ingrese la palabra: ");
      // https://stackoverflow.com/questions/1247989/how-do-you-allow-spaces-to-be-entered-using-scanf
      // Esta sola línea de código acabó con mi paciencia, y su único propósito
      // es permitir espacio en scanf...
      scanf(" %99[^\n]", str);

      adivinado = strcasecmp (palabra_actual, str) == 0;
      if (!adivinado) {
        vidas--;
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

      if (juego_revelar_caracter (primer_caracter, c_len, false)) {
        adivinado = strcasecmp (palabra_actual,
                                      palabra_adivinada) == 0;
      } else {
        adivinado = false;
        vidas--;
      }
      free (primer_caracter);
      primer_caracter = NULL;
      break;
    case TIPO_0:
    case N_TIPOS:
    default:
      break;
    }
  }while(vidas > 0 && !adivinado);
}

/**
 * Imprime el status actual de la partida, con número de vidas y el progreso
 * para adivinar la palabra
 *
 * @self La instancia del juego
 */
void juego_imprimir_partida(void)
{
  size_t altura_textura;

  altura_textura = textura_get_altura (vida_textura);
  printf ("Tus vidas:\n\n");
  for (size_t linea = 0; linea < altura_textura; linea++)
  {
    for (size_t i = 0; i < vidas; i++) {
      textura_imprimir_linea(vida_textura, linea);
    }
    putchar('\n');
  }
  printf("\n\n");
  juego_imprimir_palabra_adivinada ();
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
void juego_imprimir_palabra_adivinada (void)
{
  for (size_t i = 0; i < palabra_len; i++) {
    char c_adivinado = palabra_adivinada[i];
    char c_actual = palabra_actual[i];
    if ((PRIMER_U8 (c_actual) || ES_ASCII(c_actual)) || PARTE_U8 (c_adivinado)) {
      putchar (c_adivinado);
    }
  }
  putchar('\n');
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

/**
 * Función que intenta revelar @u8_c en la palabra a adivinar
 *
 * @self La instancia del juego
 * @u8_c Un caracter UTF-8 válido
 * @c_len La longitud de @u8_c
 */
bool juego_revelar_caracter(const char *u8_c,
                            size_t      c_len,
                            bool        es_alt)
{
  int valido = 0;
  size_t alt_len;
  const char *alt;

  for (size_t i = 0; i < palabra_len; i++)
  {
    // Significa que el caracter ya fue adivinado
    if (strncasecmp (&palabra_adivinada[i], u8_c, c_len) == 0)
    {
      valido = false;
      break;
    }
    // u8_c está en el string
    if (strncasecmp (&palabra_actual[i], u8_c, c_len) == 0)
    {
      strncpy (&palabra_adivinada[i], &palabra_actual[i], c_len);
      valido = true;
    }
  }

  // Vamos a revelar los caracteres equivalentes, como caracteres cono acento/
  // sin acento, Ñ...
  if (!es_alt)
  {
    alt = u8_get_caracter_equivalente_minuscula (u8_c, &alt_len);
    if (alt != NULL) {
      valido += juego_revelar_caracter (alt, alt_len, true);
    }
    alt = u8_get_caracter_equivalente_mayuscula (u8_c, &alt_len);
    if (alt != NULL) {
      valido += juego_revelar_caracter (alt, alt_len, true);
    }
    if (!ES_ASCII (u8_c[0])) {
      alt = u8_get_ascii_equivalente (u8_c);
      if (alt != NULL) {
        valido += juego_revelar_caracter (alt, 1, true);
      }
    }
  }

  return valido;
}

/**
 * Libera la memoria utilizada por el juego
 */
void juego_finalizar(void)
{
  for (size_t i = 0; i < n_categorias; i++)
  {
    categoria_destruir (categorias[i]);
  }

  if (palabra_actual != NULL)
  {
    free(palabra_actual);
  }

  if (palabra_adivinada != NULL)
  {
    free(palabra_adivinada);
  }

  if (splash_textura != NULL) {
    textura_liberar(splash_textura);
  }

  if (vida_textura != NULL) {
    textura_liberar(vida_textura);
  }

  if (derrota_textura != NULL) {
    textura_liberar(derrota_textura);
  }

  if (victoria_textura != NULL) {
    textura_liberar(victoria_textura);
  }
}

/* Termina código del juego, inicia código de las categorías */

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
Categoria *categoria_nueva(const char *nombre)
{
  Categoria *nueva;
  if (nombre == NULL) {
    return NULL;
  }

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
                                         const char *archivo)
{
  Categoria *nueva = NULL;
  char *palabra = NULL;
  long caracteres;
  size_t palabra_size;
  FILE *stream;

  if (nombre == NULL) {
    return NULL;
  }
  if (archivo == NULL) {
    return NULL;
  }

  stream = fopen(archivo, "r");

  if (stream == NULL) {
    printf ("No se pudo abrir el archivo %s para la categoría %s\n",
            nombre, archivo);
    return NULL;
  }
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
                                 int palabra_size)
{
  char *copia_palabra;
  if (self == NULL) {
    return;
  }
  if (palabra == NULL) {
    return;
  }

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
  if (self == NULL) {
    return;
  }

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
const char *categoria_get_palabra(Categoria *self, unsigned int indice) {
  if (self == NULL) {
    return NULL;
  }
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
  if (self == NULL) {
    return -1;
  }
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
  if (self == NULL) {
    return NULL;
  }
  return self->nombre;
}

void categoria_destruir(Categoria *self) {
  if (self == NULL) {
    return;
  }
  for (size_t i = 0; i < self->n_palabras; i++) {
    free(self->palabras[i]);
  }
  free(self->palabras);
  free(self->nombre);
  free(self);
}
