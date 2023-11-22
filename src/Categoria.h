/* Categoria.h
 *
 * Copyright 2023 Diego Iván
 * Copyright 2023 Juan Pablo Alquicer
 * Copyright 2023 Mariana García
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef __CATEGORIA_H__
#define __CATEGORIA_H__

struct __Categoria;

/**
 * Tipo que representa a una categoría del ahorcado
 */
typedef struct __Categoria Categoria;

Categoria *categoria_nueva(const char *nombre);
Categoria *categoria_nueva_desde_archivo(const char *, const char *);
const char *categoria_get_nombre(Categoria *);
void categoria_registrar_palabra(Categoria *, const char *, int);
const char *categoria_get_palabra(Categoria *, int);
int categoria_get_n_palabras(Categoria *);
void categoria_destruir(Categoria *);

#endif
