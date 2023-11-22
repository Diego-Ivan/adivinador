/* Unicode.c
 *
 * Copyright 2023 Diego Iván
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Macros.h"
#include "Unicode.h"
#include <stdbool.h>
#include <string.h>

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
  char *retval = NULL;
  bool inicio_u8 = 0;
  return_val_if_fail (str != NULL, NULL);
  return_val_if_fail (charlen != NULL, NULL);

  *charlen = 0;
  retval = calloc (strlen (str), sizeof (char));
  if (ES_ASCII (str[0]))
  {
    retval[0] = str[0];
    *charlen = 1;
    return retval;
  }

  for (; str[*charlen] != 0; (*charlen)++)
  {
    char c = str[*charlen];
    if (PRIMER_U8 (c))
    {
      if (inicio_u8) {
        break;
      }
      inicio_u8 = true;
      retval[*charlen] = c;
      continue;
    }
    if (PARTE_U8 (c) && inicio_u8) {
      retval[*charlen] = c;
      continue;
    }
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
  return_val_if_fail (c != NULL, NULL);
  return_val_if_fail (size != NULL, NULL);

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
  *size = retval != NULL ? strlen (retval) : 0;
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
  return_val_if_fail (c != NULL, NULL);
  return_val_if_fail (size != NULL, NULL);

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
  *size = retval != NULL ? strlen (retval) : 0;
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
  return_val_if_fail (c != NULL, NULL);

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
