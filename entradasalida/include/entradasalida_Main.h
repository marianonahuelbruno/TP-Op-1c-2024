#ifndef TP_ENTRADASALIDA_MAIN_H_
#define TP_ENTRADASALIDA_MAIN_H_
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
#include <pthread.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

#include "entradasalida_var_globales.h"


char* nombre_interfaz;
char* config_interfaz;


//lo que estaba en entrada salida memoria

int32_t socket_memoria_entradasalida;
sem_t respuesta_memoria;
char* string_leida_memoria;

// lo que estaba en entrada salida kernel


int32_t socket_kernel_entradasalida;

//lo que estaba en entra da salida inicio


 t_config* config;
 t_config* config_conexion;

 char* IP_KERNEL;
 char* PUERTO_KERNEL;
 char* IP_MEMORIA;
 char* PUERTO_MEMORIA;

 char* TIPO_INTERFAZ;
 uint32_t TIEMPO_UNIDAD_TRABAJO;



 // lo que estaba en entrada salida FS

t_log* logger;
t_log* logger_debug;
char* PATH_BASE_DIALFS;
uint32_t BLOCK_SIZE;
uint32_t BLOCK_COUNT;
uint32_t RETRASO_COMPACTACION;
char* path_bloques;
char* bloques;
char* path_bitmap;
void* array_bitmap;
t_bitarray* bitmap_bloques;
char* path_metadata;

#endif //TP_ENTRADASALIDA_MAIN_H_