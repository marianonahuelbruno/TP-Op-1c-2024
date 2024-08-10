#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
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

#include "extern_globales.h"


#ifndef  ATENDER_OPERACION_H
#define ATENDER_OPERACION_H

void imprimir_listas_de_estados(t_list* lista,char* estado);
bool detener_planificacion=false;
void leer_path_comandos(char* path);


#endif //ATENDER_OPERACION_H
