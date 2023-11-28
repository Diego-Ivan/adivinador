/* main.c
 *
 * Copyright 2023 Diego Iván M.E
 * Copyright 2023 Juan Pablo Alquicer
 * Copyright 2023 Mariana García
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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

/* Inician declaraciones para funciones UTF-8 */
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
/* Terminan declaraciones para funciones UTF-8 */

/* Inician declaraciones de las categorias */
/*
 * Vamos a crear una estructura opaca para que no se puedan modificar
 * los campos de la categoría más que dentro del mismo código de la categoría
 */
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

/* Inician declaraciones de las texturas */
struct __Textura;
typedef struct __Textura Textura;

Textura *textura_nueva_desde_archivo(const char *);
int textura_get_rowstride(Textura *);
int textura_get_altura(Textura *);
const char *textura_get_linea(Textura *, size_t);
void textura_imprimir_linea (Textura *, size_t);
void textura_imprimir(Textura *);
void textura_liberar(Textura *);
/* Terminan las declaraciones de las texturas */

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
   * como  están alojadas en el heap, tenemos que liberarlas cuando ya
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

      // Usamos strcasecmp para ignorar si es mayuscula o minuscula
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
    /*
     * Solo vamos a imprimir el caracter si cumple con alguna de las siguientes
     * condiciones:
     *
     * 1. Si el caracter de la palabra actual es el primer byte de una
     * cadena UTF-8
     * 2. Si el caracter de la palabra actual es un caracter ASCII
     * 3. Si el caracter de la cadena a adivinar ya fue revelado
     */
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
    /* Iteramos sobre los tipos de intento válidos y los imprimimos */
    for (TipoIntento tipo = TIPO_0 + 1; tipo < N_TIPOS; tipo++) {
      printf ("%d. %s\n", tipo, tipo_intento_to_string (tipo));
    }
    scanf ("%d", &seleccion);
    // Si el índice seleccionado por el usuario es válido, salimos para
    // devolverlo
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

  /*
   * Nota importante:
   * En esta función no podemos iterar caracter por caracter,
   * recordemos que aquí las cadenas pueden tener caracteres especiales más
   * allá de los ASCII, y estos caracteres ocupan mas de un espacio en una
   * cadena de caracteres, entonces vamos a comparar secciones de cadenas que
   * puedan contener los caracteres
   *
   * Por eso como parametros pedimos una cadena de caracteres en vez de un
   * solo caracter, además de la longitud de la cadena para saber cuantos
   * espacio debemos de comparar
   */

  for (size_t i = 0; i < palabra_len; i++)
  {
    // Significa que el caracter ya fue adivinado
    // strncasecmp nos ayuda a comparar cierta cantidad de caracteres entre
    // dos cadenas

    /*
     * Aquí vamos a usar unos cuantos trucos de las cadenas de caracteres en C.
     * Las cadenas de caracteres técnicamente son solo punteros, y las funciones
     * que leen cadenas de caracteres leen desde la direccion de memoria a la
     * que el puntero apunta hasta que encuentran un 0 (el caracter nulo).
     *
     * Aplicando esta lógica, podemos hacer que una función solo lea desde una
     * posicion deseada en la cadena de caracteres. Para hacerlo, tenemos
     * que pasarle la direccion de memoria (osea, un puntero, y por tanto una
     * cadena) del caracter que esta en el indice en el que queremos que empiece:
     *
     * &mi_cadena[indice]
     *
     * El operador & da la direccion de memoria de la expresión que lo sigue.
     */
    if (strncasecmp (&palabra_adivinada[i], u8_c, c_len) == 0)
    {
      valido = false;
      break;
    }
    // El caracter que se pasó como parametro si está en la cadena
    if (strncasecmp (&palabra_actual[i], u8_c, c_len) == 0)
    {
      strncpy (&palabra_adivinada[i], &palabra_actual[i], c_len);
      valido = true;
    }
  }

  // Vamos a revelar los caracteres equivalentes, como caracteres cono acento/
  // sin acento, Ñ...
  // Aquí no tenemos de otra más que hacer la función recursiva. Solo no
  // ejecutaremos esta parte de la función cuando especifiquemos que estamos
  // revisando caracteres equivalentes
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

/* Termina código de las categorías */
/* Inicia código de las texturas */

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
  if (self == NULL) {
    return;
  }
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

  if (camino == NULL) {
    return NULL;
  }

  stream = fopen(camino, "r");
  if (stream == NULL) {
    printf ("No se pudo abrir el archivo %s para crear una textura", camino);
  }

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
  free (linea);
  fclose (stream);

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
  if (self == NULL) {
    return -1;
  }
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
  if (self == NULL) {
    return -1;
  }
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
  if (self == NULL) {
    return NULL;
  }
  if (indice >= self->altura) {
    printf ("Índice %lu no válido!\n", indice);
    return NULL;
  }
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
  if (self == NULL) {
    return;
  }
  if (indice >= self->altura) {
    printf ("Índice %lu no válido!\n", indice);
    return;
  }
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
  if (self == NULL) {
    return;
  }
  for (; fila < self->altura; fila++) {
    textura_imprimir_linea_unsafe(self, fila);
    putchar('\n');
  }
}

void textura_agregar_linea(Textura *self,
                           const char *linea)
{
  if (self == NULL) {
    return;
  }
  if (linea == NULL) {
    return;
  }
  if (self->altura >= self->buffer_size) {
    // Signfica que ya hemos superado el espacio que tenemos reservado, alojamos
    // más
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
  if (self == NULL) {
    return;
  }
  for (size_t i = 0; i < self->altura; i++) {
    free(self->datos[i]);
  }
  free(self->datos);
  free(self);
}

/* Termina código de las texturas */

/* Inicia código de las funciones UTF-8 */
/**
 * Retorna la minuscula de @c
 *
 * @c - El caracter que se quiere convertir a minusculas
 *
 * Returns: La minuscula de @c, si es que tiene
 */
int char_minuscula(int c)
{
  if (c >= 65 && c <= 90) {
    return c + 32;
  }
  return c;
}

/**
 * Obtiene el primer caracter en codificación UTF-8 de @str.
 *
 * Esta función es necesaria para poder implementar adivinanzas de caracteres
 * UTF-8, como letras acentudas o la ñ.
 *
 * Se utilizó https://dev.to/rdentato/utf-8-strings-in-c-1-3-42a4 como recurso
 * principal para implementar la función.
 *
 * Esta función NO hace validación de ningún tipo, solo retorna el primer
 * caracter, por lo que se espera que @str sea válido desde un inicio
 *
 * @str La cadena de la que se quiere obtener el caracter. Debe ser UTF-8 valida
 *
 * @charlen Una dirección de memoria válida a una variable size_t para
 * almacenar la longitud del primer caracter
 *
 * Returns: (transfer: ownership) El primer caracter UTF-8 de @str
 */
char *u8_construir_primer_caracter(const char *str, size_t *charlen)
{
  /*
   * Los caracteres codificados en UTF-8 tienen unas caracteristicas particulares
   * que nos pueden ayudar a identificarlos.
   *
   * 1. Los dos bits más significantes del primer byte de un caracter UTF-8
   * son 11. Podemos saber si en un byte sus primeros dos bits son 11
   * con la siguiente operacion: (byte & 0xC0) == 0xC0. Esta operacion está
   * implementada en la macro PRIMER_U8
   *
   * 2. Los dos bits más significativos de los demás bytes de un caracter UTF-8
   * son 10. Podemos saber si en un byte sus primeros dos bits son 10 con la
   * operacion (byte & 0xC0) == 0x80, esta operación está implementada
   * en la macro PARTE_U8
   */
  char *retval = NULL;
  bool inicio_u8 = 0;
  if (str == NULL) {
    return NULL;
  }
  if (charlen == NULL) {
    return NULL;
  }

  *charlen = 0;
  // Alojamos memoria en para el caracter de retorno
  retval = calloc (strlen (str), sizeof (char));

  /*
   * Si el primer caracter de la cadena es ASCII, lo retornamos
   */
  if (ES_ASCII (str[0]))
  {
    retval[0] = str[0];
    *charlen = 1;
    return retval;
  }

  /* Si no, vamos a iterar sobre la cadena para armar el caracter que queremos */
  for (; str[*charlen] != 0; (*charlen)++)
  {
    char c = str[*charlen];
    if (PRIMER_U8 (c))
    {
      /*
       * Si el caracter en el que estamos es el primer byte de un caracter
       * UTF-8, pero ya habíamos encontrado uno antes, significa que ya
       * estamos empezando a leer otro caracter. Salimos del bucle
       */
      if (inicio_u8) {
        break;
      }
      /*
       * Si no, signifca qu es el primer byte de un caracter uTF-8 que nos
       * encontramos, así que lo asignamos al valor de retorno y continuamos
       */
      inicio_u8 = true;
      retval[*charlen] = c;
      continue;
    }
    /*
     * Si el caracter que nos encontramos es un byte de un caracter U8,
     * lo añadimos
     */
    if (PARTE_U8 (c) && inicio_u8) {
      retval[*charlen] = c;
      continue;
    }

    /* Si es ASCII, significa que ya estamos leyendo otro caracter, salimos */
    if (ES_ASCII (c)) {
      if (inicio_u8) break;
      retval[*charlen] = c;
      break;
    }
  }

  return retval;
}

/**
 * Retorna el caracter minúscula equivalente de @c, o NULL en caso de que no
 * tenga
 *
 * @c El caracter del cual se quiere obtener la minuscula
 *
 * @size Una direccion de memoria valida para almacenar la longitud de @c del
 * caracter equivalente
 *
 * Returns: (transfer: none) La minuscula de @c o NULL, en caso de que no tenga
 */
const char *u8_get_caracter_equivalente_minuscula(const char *c,
                                                  size_t     *size)
{
  const char *retval = NULL;
  if (c == NULL) {
    return NULL;
  }
  if (size == NULL) {
    return NULL;
  }

  if (ES_ASCII (c[0]))
  {
    switch (char_minuscula (c[0]))
    {
    case 'a':
      retval = "á";
      break;
    case 'e':
      retval = "é";
      break;
    case 'i':
      retval = "í";
      break;
    case 'o':
      retval = "ó";
      break;
    case 'u':
      retval = "ú";
      break;
    default:
      break;
    }
  }
  else
  {
    if (strcmp (c, "Á") == 0) {
      retval = "á";
    }
    if (strcmp (c, "É") == 0) {
      retval = "é";
    }
    if (strcmp (c, "Í") == 0) {
      retval = "í";
    }
    if (strcmp (c, "Ó") == 0) {
      retval = "ó";
    }
    if (strcmp (c, "Ú") == 0) {
      retval = "ú";
    }
    if (strcmp (c, "Ñ") == 0) {
      retval = "ñ";
    }
  }
  if (retval != NULL) {
    *size = strlen (retval);
  } else {
    *size = 0;
  }
  return retval;
}

/**
 * Retorna el caracter mayúscula equivalente de @c, o NULL en caso de que no
 * tenga
 *
 * @c El caracter del cual se quiere obtener la mayúscula
 *
 * @size Una direccion de memoria valida para almacenar la longitud del caracter
 * equivalente
 *
 * Returns: (transfer: none) La mayúscula de @c o NULL, en caso de que no tenga
 */
const char *u8_get_caracter_equivalente_mayuscula(const char *c,
                                                  size_t     *size)
{
  const char *retval = NULL;
  if (c == NULL) {
    return NULL;
  }
  if (size == NULL) {
    return NULL;
  }

  if (ES_ASCII (c[0]))
  {
    switch (char_minuscula (c[0]))
    {
    case 'a':
      retval = "Á";
      break;
    case 'e':
      retval = "É";
      break;
    case 'i':
      retval = "Í";
      break;
    case 'o':
      retval = "Ó";
      break;
    case 'u':
      retval = "Ú";
      break;
    default:
      break;
    }
  }
  else
  {
    if (strcmp (c, "a") == 0) {
      retval = "Á";
    }
    if (strcmp (c, "e") == 0) {
      retval = "É";
    }
    if (strcmp (c, "i") == 0) {
      retval = "Í";
    }
    if (strcmp (c, "o") == 0) {
      retval = "Ó";
    }
    if (strcmp (c, "u") == 0) {
      retval = "Ú";
    }
    if (strcmp (c, "ñ") == 0) {
      retval = "Ñ";
    }
  }
  if (retval != NULL) {
    *size = strlen (retval);
  } else {
    *size = 0;
  }
  return retval;
}

/**
 * Retorna el caracter ASCII equivalente de @c
 *
 * @c El caracter del cual se quiere obtener el ASCII equivalente
 *
 * Returns: (transfer: none) El ASCII equivalente de @c, o NULL, en caso de que
 * no tenga
 */
const char *u8_get_ascii_equivalente(const char *c)
{
  const char *retval = NULL;
  if (c == NULL) {
    return NULL;
  }

  if (strcmp (c, "á") == 0 || strcmp (c, "Á") == 0) {
    retval = "a";
  }
  if (strcmp (c, "é") == 0 || strcmp (c, "É") == 0) {
    retval = "e";
  }
  if (strcmp (c, "í") == 0 || strcmp (c, "Í") == 0) {
    retval = "i";
  }
  if (strcmp (c, "ó") == 0 || strcmp (c, "Ó") == 0) {
    retval = "o";
  }
  if (strcmp (c, "ú") == 0 || strcmp (c, "Ú") == 0) {
    retval = "u";
  }

  return retval;
}
