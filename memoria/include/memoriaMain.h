#ifndef TP_MEMORIA_MAIN_H_
#define TP_MEMORIA_MAIN_H_

#include <readline/readline.h>
#include <commons/bitarray.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"
#include "../include/memPaginacion.h"
#include "../include/memIniciar.h"
#include "../include/memCpu.h"
#include "../include/extGlobales.h"
#include "../include/memES.h"
#include "../include/memKernel.h"

t_log* logger;
t_log* logger_debug;
char* path_base='\0';
int tam_pagina;
int tam_memoria;
int retardo;
t_config* config;
char* puerto_escucha;
int cant_frames;
void* memoria_usuario; 

int32_t socket_cpu_memoria;
int32_t socket_kernel_memoria;
int32_t socket_entradasalida_memoria;
int32_t socket_escucha;

void enviar_tam_pag();

//MUTEX
pthread_mutex_t mutex_tablaDePaginas;
pthread_mutex_t mutex_procesos;
pthread_mutex_t mutex_listaDeinstrucciones;
pthread_mutex_t mutex_bitmap;
//pthread_mutex_t accediendo_a_memoria;
#endif //TP_MEMORIA_MAIN_H_