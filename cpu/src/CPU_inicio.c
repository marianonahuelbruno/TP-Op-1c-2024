#include "../include/CPU_inicio.h"

void iniciar_CPU(){
    iniciar_logs();
    iniciar_config();

    PID = 0;
    int_consola = false;
    int_quantum = false;
    
    contexto_interno.PC = 0;
    contexto_interno.AX = 0;
    contexto_interno.BX = 0;
    contexto_interno.CX = 0;
    contexto_interno.DX = 0;
    contexto_interno.EAX = 0;
    contexto_interno.EBX = 0;
    contexto_interno.ECX = 0;
    contexto_interno.EDX = 0;
    contexto_interno.DI = 0;
    contexto_interno.SI = 0;

    sem_init(&hay_proceso_ejecutando, 0, 0);
    sem_init(&espera_iterador, 0, 0);
    
    sem_init(&prox_instruccion, 0, 0);
    sem_init(&respuesta_resize, 0, 0);
    sem_init(&respuesta_copy_string, 0, 0);
    sem_init(&respuesta_MOV_IN, 0, 0);
    pthread_mutex_init(&mutex_detenerEjecucion, NULL);

}

void iniciar_logs(){
    logger = start_logger("log_cpu.log", "LOG CPU", LOG_LEVEL_TRACE);
	if(logger==NULL){
		perror("No se pudo crear el logger");
		exit(EXIT_FAILURE);
	}
    logger_debug = start_logger("log_cpu_debug.log", "LOG CPU DB", LOG_LEVEL_TRACE);
	if(logger_debug==NULL){
		perror("No se pudo crear el logger debug");
		exit(EXIT_FAILURE);
	}
    logger_valores = start_logger("log_valores.log", "LOG valores de registros", LOG_LEVEL_TRACE);
	if(logger_debug==NULL){
		perror("No se pudo crear el logger de valores");
		exit(EXIT_FAILURE);
	}
}

void iniciar_config(){
    config = start_config("./cpu.config");
	if(config==NULL){
		perror("No se pudo crear la config");
		exit(EXIT_FAILURE);
	}


    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    log_info(logger, "IP leido: %s", ip_memoria);

    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    log_info(logger, "PUERTO MEMORIA leido: %s", puerto_memoria);

    puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    log_info(logger, "PUERTO DISPATCH leido: %s", puerto_escucha_dispatch);

    puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    log_info(logger, "PUERTO INTERRUPT leido: %s", puerto_escucha_interrupt);
    
    cant_entradas_TLB = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    log_info(logger, "CANTIDAD ENTRADAS TLB leido: %u", cant_entradas_TLB);

    algoritmo_TLB = config_get_string_value(config, "ALGORITMO_TLB");
    log_info(logger, "ALGORITMO TLB leido: %s", algoritmo_TLB);
}
