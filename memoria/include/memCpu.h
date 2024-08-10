#ifndef TP_MEMORIA_CPU_H_
#define TP_MEMORIA_CPU_H_

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
#include "memPaginacion.h"

void enviar_instruccion(int socket_cpu_memoria, t_instruccion* instruccion);
void recibir_fetch(int socket_cpu_memoria, uint32_t* PID, uint32_t* PC);
void fetch(int socket_cpu_memoria);
void conexion_con_cpu();
void frame(int socket_cpu_memoria);
void movIn();
void movOut();
void copiar_string_read(int socket_cpu_memoria);
void copiar_string_write(int socket_cpu_memoria);
void ins_resize(int socket_cpu_memoria);



#endif //TP_MEMORIA_CPU_H_