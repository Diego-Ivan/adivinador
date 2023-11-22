/* Unicode.h
 *
 * Copyright 2023 Diego Iván
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>

#ifndef __UNICODE_H__
#define __UNICODE_H__

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

#endif
