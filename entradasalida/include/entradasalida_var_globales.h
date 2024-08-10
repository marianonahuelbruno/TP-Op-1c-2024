#ifndef ENTRADASALIDA_VAR_GLOBALES_H_
#define ENTRADASALIDA_VAR_GLOBALES_H_

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
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include "../../utils/include/conexiones.h"
#include "../../utils/include/utils.h"

 // Entrada salida memoria ---------------------------------------------------

 extern sem_t respuesta_memoria;
 extern char* string_leida_memoria;

//Entrada salida file system ---------------------------------------------------

extern char* PATH_BASE_DIALFS;
extern uint32_t BLOCK_SIZE;
extern uint32_t BLOCK_COUNT;
extern uint32_t RETRASO_COMPACTACION;

extern char* path_bloques;
extern char* bloques;

extern char* path_bitmap;
extern void* array_bitmap;
extern t_bitarray* bitmap_bloques;

extern char* path_metadata;

extern sem_t respuesta_memoria;

void inicializar_FS();
void inicializar_bloques();
void inicializar_bitmap();

bool crear_archivo(char* nombre_archivo);
int32_t buscar_bloque_libre();
void crear_metadata(int32_t bloque, char* nombre_archivo);

bool eliminar_archivo(char* nombre_archivo);
bool existe_archivo(char* nombre_archivo);
void liberar_bloques(char* path_archivo_metadata);

bool truncar_archivo(uint32_t PID, char* nombre_archivo, uint32_t nuevo_tamanio);
int32_t cantidad_de_bloques(int32_t tamanio_archivo);
void liberar_n_bloques(int32_t bloque_inicial, int32_t bloques_a_liberar);
bool intentar_asignar_bloques(uint32_t PID, char* nombre_archivo, t_config* metadata, int32_t bloque_inicial, int32_t cant_bloques, int32_t nueva_cant_bloques);
bool asignar_n_bloques(int32_t bloque_inicial, int32_t bloques_a_asignar);
bool reasignar_bloques(t_config* metadata, int32_t cant_bloques, int32_t nueva_cant_bloques);

int32_t contar_bloques_libres();
int32_t compactacion(uint32_t PID, char* nombre_archivo, uint32_t nueva_cant_bloques);
void limpiar_bitmap();
int32_t compactar_archivo(char* nombre_archivo, void* nuevos_bloques);
void asignar_bloques_compactacion(int32_t nuevo_inicio, int32_t cant_bloques);

void FS_WRITE(void* bloques, uint32_t bloque_inicial, uint32_t puntero, uint32_t tamanio_total, char* datos_a_escribir);
void FS_READ(void* bloques, uint32_t bloque_inicial, uint32_t puntero, uint32_t tamanio_total, void* datos_leidos);

//entrada salida inicio ---------------------------------------------------

extern int32_t socket_memoria_entradasalida;
  
extern char* PATH_BASE_DIALFS;
extern uint32_t BLOCK_SIZE;
extern uint32_t BLOCK_COUNT;
extern uint32_t RETRASO_COMPACTACION;
 
extern char* path_bloques;
extern char* bloques;
 
extern char* path_bitmap;
extern void* array_bitmap;
extern t_bitarray* bitmap_bloques;

extern t_log* logger;
extern t_log* logger_debug;
extern t_config* config;
extern t_config* config_conexion;

extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;

extern char* TIPO_INTERFAZ;
extern uint32_t TIEMPO_UNIDAD_TRABAJO;

extern char* PATH_BASE_DIALFS;
extern uint32_t BLOCK_SIZE;
extern uint32_t BLOCK_COUNT;
extern uint32_t RETRASO_COMPACTACION;

extern sem_t respuesta_memoria;

void iniciar_entradasalida(char* nombre_interfaz);
void iniciar_logs(char* nombre_interfaz);
void iniciar_config(char* config_interfaz);
void iniciar_config_conexion();
cod_interfaz get_tipo_interfaz(char* TIPO_INTERFAZ);


// entrada salida Main ---------------------------------------------------

extern char* nombre_interfaz;
extern char* config_interfaz;

void validar_argumentos(char* nombre_interfaz);
void notificar_kernel(bool exito);
char* leer_de_teclado(uint32_t tamanio_a_leer);


//entrada salida kernel ---------------------------------------------------

extern int32_t socket_kernel_entradasalida;

void atender_conexion_entradasalida_KERNEL();

//entrada salida memoria ---------------------------------------------------

extern t_log* logger;
extern t_log* logger_debug;
extern sem_t respuesta_memoria;
extern char* string_leida_memoria;

void gestionar_conexion_memoria();

#endif /*  ENTRADASALIDA_INICIO_H_ */