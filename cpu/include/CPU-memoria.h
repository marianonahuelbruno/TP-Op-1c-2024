#ifndef CPU_MEMORIA_H_
#define CPU_MEMORIA_H_

#include <pthread.h>
#include <stdlib.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern t_log* logger;
extern t_log* logger_debug;

extern char* ip_memoria;
extern char* puerto_memoria;
extern int32_t socket_cpu_memoria;

extern uint32_t cant_entradas_TLB;
extern char* algoritmo_TLB;
extern uint32_t tamanio_de_pagina;

extern uint32_t PID;
extern t_contexto_ejecucion contexto_interno;
extern sem_t hay_proceso_ejecutando;
extern sem_t espera_iterador;

extern t_instruccion* ins_actual;
extern sem_t prox_instruccion;
extern sem_t respuesta_resize;
extern bool resize_ok;
extern sem_t respuesta_MOV_IN;
extern uint8_t respuesta_mov_in_8;
extern uint32_t respuesta_mov_in_32;
extern sem_t respuesta_copy_string;
extern char* string_leida_de_memoria;

extern sem_t respuesta_marco;
extern uint32_t marco_pedido;

void gestionar_conexion_memoria();

void fetch(uint32_t PID, uint32_t PC);
t_instruccion* recibir_instruccion();
void recibir_tamanio_de_pagina();
void pedir_rezise(uint32_t PID, uint32_t valor);

#endif //CPU_MEMORIA_H_