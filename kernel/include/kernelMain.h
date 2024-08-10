#ifndef TP_KERNEL_MAIN_H_
#define TP_KERNEL_MAIN_H_

#include <commons/log.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include "../../utils/include/utils.h"
#include "../../utils/include/conexiones.h"


#include "extern_globales.h"







    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;
    char* puerto_escucha;
    char* algoritmo_planificacion;
	int64_t quantum;
	char** recursos;
	char** instancias_recursos;
	int32_t grado_multiprogramacion;
    char*path_de_comandos_base;
    int32_t* instancias_recursos_int=NULL;




    int32_t socket_kernel_cpu_dispatch;
    int32_t socket_kernel_cpu_interrupt; 
    int32_t socket_memoria_kernel;
    int32_t socket_entradasalida_kernel;
    int32_t socket_escucha;



    t_log* logger;
    t_log* logger_debug;
    t_config* config;

    t_list *lista_new;
    t_list *lista_ready;
    t_list *lista_exit;
    t_list *lista_bloqueado;
    t_list *lista_ready_prioridad;
    t_list *lista_bloqueado_prioritario;
    t_list *lista_de_interfaces;

    pthread_t hilo_CPU_dispatch;


#endif //TP_KERNEL_MAIN_H_