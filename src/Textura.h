#include <stdlib.h>

#ifndef __TEXTURA_H__
#define __TEXTURA_H__

struct __Textura;
typedef struct __Textura Textura;

Textura *textura_nueva_desde_archivo(const char *);
int textura_get_rowstride(Textura *);
int textura_get_altura(Textura *);
const char *textura_get_linea(Textura *, size_t);
void textura_dibujar_linea(Textura *, size_t);
void textura_imprimir_linea (Textura *, size_t);
void textura_imprimir(Textura *);
void textura_liberar(Textura *);

#endif
