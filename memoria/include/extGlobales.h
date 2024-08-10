#ifndef TP_MEMORIA_EXT_H_
#define TP_MEMORIA_EXT_H_

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
#include <commons/bitarray.h>
#include <pthread.h>

#include <semaphore.h>
#include <commons/collections/queue.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"


extern t_bitarray* bitmap;
extern void* memoria_usuario; 
extern t_list* tablaDePaginas;
extern t_list* procesos;

//LOGGER
extern t_log* logger;

//CONFIG
extern char* puerto_escucha;
extern char* path_base;
extern int32_t tam_pagina;
extern int32_t tam_memoria;
extern int32_t retardo;
extern int32_t cant_frames;
extern t_log* logger_debug;

extern t_config* config;

extern int32_t socket_cpu_memoria;
extern int32_t socket_kernel_memoria;
extern int32_t socket_entradasalida_memoria;
extern int32_t socket_escucha;

t_list* leer_pseudocodigo(char* path);
// memoria de instrucciones
t_instruccion* parsear_instruccion(char* linea);

cod_ins hash_ins(char* ins);
char* path_completo(char* path_base, char* path_parcial);
t_instruccion* get_ins(t_list* lista_instrucciones, uint32_t PC);

///MUTEX 
extern pthread_mutex_t mutex_tablaDePaginas;
extern pthread_mutex_t mutex_procesos;
extern pthread_mutex_t mutex_listaDeinstrucciones;
extern pthread_mutex_t mutex_bitmap;
//extern pthread_mutex_t accediendo_a_memoria;
//Memoria Main
void liberar_array_de_comando(char** array_de_comando, int tamanio);

#endif //TP_MEMORIA_EXT_H_