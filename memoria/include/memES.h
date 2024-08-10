#ifndef TP_MEMORIA_ES_H_
#define TP_MEMORIA_ES_H_

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
#include "memPaginacion.h"
#include "../include/extGlobales.h"

void conexion_con_es();
void read_es(int32_t socket, op_code motivo);
void write_es(int32_t socket);
void escuchar_nueva_Interfaz_mem(void* socket);

#endif //TP_MEMORIA_ES_H_