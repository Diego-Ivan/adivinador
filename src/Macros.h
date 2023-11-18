#include <stdio.h>
#include <stdlib.h>

#ifndef __MACROS_H__
#define __MACROS_H__

#define return_if_fail(expr)                                                   \
  {                                                                            \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Aserción Fallida en %s:%s:%d", __FILE__, __FUNCTION__,  \
              __LINE__);                                                       \
      return;                                                                  \
    }                                                                          \
  }

#define return_val_if_fail(expr, val)                                          \
  {                                                                            \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Aserción Fallida en %s:%s:%d", __FILE__, __FUNCTION__,  \
              __LINE__);                                                       \
      return val;                                                              \
    }                                                                          \
  }

#ifdef __unix__
#define clear_pantalla() system("clear")
#endif

#ifdef _WIN32
#define clear_pantalla() system("cls");
#endif

#endif
