#ifndef KERNEL_ENTRADASALIDA_H_
#define KERNEL_ENTRADASALIDA_H_


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
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"

#include "extern_globales.h"

 IO_type* crear_nodo_interfaz (IO_type* nueva_interfaz);
void escuchar_a_Nueva_Interfaz(void* interfaz);
void gestionar_envio_cola_nueva_interfaz(void* interfaz);
void cambiar_proceso_de_block_a_ready(uint32_t PID);
pthread_t hilo_gestion_Cola_interfaz;
pthread_t hilo_escucha_ENTRADASALIDA_KERNEL;







#endif /*  KERNEL_ENTRADASALIDA_H_ */
