#ifndef TP_MEMORIA_INICIAR_H_
#define TP_MEMORIA_INICIAR_H_

#include <stdlib.h>
#include <stdio.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "extGlobales.h"

void inciarlogsYsemaforos();
void cargarConfig();
void inicializarEspacioMem();

#endif //TP_MEMORIA_INICIAR_H_