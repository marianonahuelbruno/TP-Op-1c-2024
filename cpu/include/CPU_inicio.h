#ifndef CPU_INICIO_H_
#define CPU_INICIO_H_

#include <commons/log.h>
#include <commons/config.h>
#include <semaphore.h>
#include "../../utils/include/conexiones.h"
#include "../../utils/include/utils.h"

extern t_log* logger;
extern t_log* logger_debug;
extern t_log* logger_valores;
extern t_config* config;

extern char* ip_memoria;
extern char* puerto_memoria;
extern char* puerto_escucha_dispatch;
extern char* puerto_escucha_interrupt;
extern int32_t socket_cpu_kernel_dispatch;
extern int32_t socket_cpu_kernel_interrupt;
extern int32_t socket_cpu_memoria;
extern int32_t socket_escucha;

extern uint32_t cant_entradas_TLB;
extern char* algoritmo_TLB;

extern uint32_t PID;
extern bool int_consola;
extern bool int_quantum;
extern t_contexto_ejecucion contexto_interno;
extern sem_t hay_proceso_ejecutando;
extern sem_t espera_iterador;
extern sem_t prox_instruccion;
extern sem_t respuesta_resize;
extern sem_t respuesta_MOV_IN;
extern sem_t respuesta_copy_string;
extern pthread_mutex_t mutex_detenerEjecucion;

void iniciar_CPU();
void iniciar_logs();
void iniciar_config();

#endif /*  CPU_INICIO_H_ */
