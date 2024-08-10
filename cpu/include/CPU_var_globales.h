#ifndef CPU_VAR_GLOBALES_H_
#define CPU_VAR_GLOBALES_H_

#include <semaphore.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

t_log* logger;
t_log* logger_debug;
t_log* logger_valores;
t_config* config;


char* ip_memoria;
char* puerto_memoria;
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;

int32_t socket_cpu_kernel_dispatch;
int32_t socket_cpu_kernel_interrupt;
int32_t socket_cpu_memoria;
int32_t socket_escucha_dispatch;
int32_t socket_escucha_interrupt;

uint32_t cant_entradas_TLB;
char* algoritmo_TLB;
uint32_t tamanio_de_pagina;
bool usa_TLB;
t_list* tabla_TLB;
bool detener_ejecucion=true;

uint32_t PID;
t_contexto_ejecucion contexto_interno;
sem_t hay_proceso_ejecutando;
sem_t espera_iterador;
pthread_mutex_t mutex_detenerEjecucion;

t_instruccion* ins_actual;
sem_t prox_instruccion;
sem_t respuesta_resize;
bool resize_ok;
sem_t respuesta_MOV_IN;
uint8_t respuesta_mov_in_8;
uint32_t respuesta_mov_in_32;
sem_t respuesta_copy_string;
char* string_leida_de_memoria;

sem_t respuesta_marco;
uint32_t marco_pedido;

bool int_consola;
bool int_quantum;
op_code motivo_desalojo;

pthread_t hilo_conexion_dispatch;
pthread_t hilo_conexion_interrupt;
pthread_t hilo_conexion_memoria;

#endif /*  CPU_VAR_GLOBALES_H */
