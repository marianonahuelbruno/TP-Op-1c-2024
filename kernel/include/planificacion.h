#ifndef PLANIFICACION_H
#define PLANIFICACION_H

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
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <sys/time.h>
#include <commons/temporal.h>
#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"




#include "extern_globales.h"

t_temporal * temporizador=NULL;
int64_t tiempo_recien_ejecutado=0;
pthread_t hilo_de_desalojo_por_quantum;
uint32_t pcb_actual_en_cpu=0;
int64_t backup_de_quantum_ejecutado=0;
bool gestionando_dispatch=false;
bool vengode_gestionarDispatch=false;
bool ocupacion_cpu=true;

void interruptor_de_QUANTUM(void* quantum_de_pcb);
pthread_t hilo_de_desalojo_por_quantum; 
op_code cod_op_dispatch;

void enviar_nuevamente_proceso_a_ejecucion(t_pcb* pcb_a_reenviar);
void gestionar_solicitud_IO(t_pcb* pcb_dispatch, char* nombre_interfaz, op_code cod_op_dispatch, t_paquete* paquete);






#endif //PLANIFICACION_H
