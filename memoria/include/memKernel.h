#ifndef TP_MEMORIA_KERNEL_H_
#define TP_MEMORIA_KERNEL_H_

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
#include "../include/memPaginacion.h"

void atender_conexion_KERNEL_MEMORIA();
void conexion_con_kernel();
void crear_proceso();
void eliminar_proceso();

#endif //TP_MEMORIA_KERNEL_H_