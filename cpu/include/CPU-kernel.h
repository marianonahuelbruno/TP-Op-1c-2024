#ifndef CPU_KERNEL_H_
#define CPU_KERNEL_H_

#include <pthread.h>
#include <stdlib.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

extern t_log* logger;
extern t_log* logger_debug;
extern int32_t socket_cpu_kernel_dispatch;
extern int32_t socket_cpu_kernel_interrupt;

extern uint32_t PID;
extern t_contexto_ejecucion contexto_interno;
extern sem_t hay_proceso_ejecutando;
extern sem_t espera_iterador;
extern pthread_mutex_t mutex_detenerEjecucion;

extern bool int_consola;
extern bool int_quantum;
extern bool detener_ejecucion;

void gestionar_conexion_dispatch(void);
void desalojar_proceso(op_code motivo_desalojo);
void enviar_CE_con_1_arg(op_code motivo_desalojo, char* arg1);
void solicitar_IO_GEN_SLEEP(op_code motivo_desalojo, char* nombre_interfaz, uint32_t unidades_trabajo);
void enviar_CE_con_2_arg(op_code motivo_desalojo, char* arg1, char* arg2);
void enviar_CE_con_5_arg(op_code motivo_desalojo, char* arg1, char* arg2, char* arg3, char* arg4, char* arg5);
bool esperar_respuesta_recurso();

void gestionar_conexion_interrupt();

#endif //CPU_KERNEL_H_