#include "Recursos.h"
#include <stdlib.h>

char *get_carpeta_recursos(void) {
  char *datadir = getenv ("XDG_DATA_HOME");
  if(datadir != NULL) {
    return datadir;
  }
  return NULL;
}
