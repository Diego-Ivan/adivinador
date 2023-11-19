#include "Categoria.h"

#ifndef __JUEGO_H__
#define __JUEGO_H__

struct __Juego;
typedef struct __Juego Juego;

typedef enum {
  MENU,
  JUGANDO,
  FIN
} EstadoJuego;

Juego *juego_nuevo(void);
void juego_registrar_categoria(Juego *, Categoria *);
void juego_iniciar_bucle(Juego *);
void juego_liberar(Juego *);

int juego_get_vidas(Juego *);

EstadoJuego juego_get_estado(Juego *);

#endif
