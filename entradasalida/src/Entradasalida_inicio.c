#include "../include/entradasalida_inicio.h"

void iniciar_entradasalida(char* nombre_interfaz)
{
    iniciar_logs(nombre_interfaz);
    iniciar_config(nombre_interfaz);
    iniciar_config_conexion();

    log_debug(logger_debug,"IP KERNEL: %s",IP_KERNEL);
    log_debug(logger_debug,"PUERTO KERNEL: %s",PUERTO_KERNEL);
    log_debug(logger_debug,"IP MEMORIA: %s",IP_MEMORIA);
    log_debug(logger_debug,"PUERTO MEMORIA: %s",PUERTO_MEMORIA);

    sem_init(&respuesta_memoria, 0, 0);
}

void iniciar_logs( char* nombre_interfaz) {
    char* path_log = string_duplicate("log_");
    string_append(&path_log, nombre_interfaz);
    char* nombre_log = string_duplicate(path_log);
    string_append(&path_log, ".log");

    logger = start_logger(path_log, nombre_log, LOG_LEVEL_TRACE);

    if (logger == NULL) {
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }

    string_append(&nombre_log, "_debug.log");
    logger_debug = start_logger(nombre_log, "log_debug_IO", LOG_LEVEL_TRACE);

    if (logger == NULL) {
        perror("No se pudo crear el logger debug");
        exit(EXIT_FAILURE);
    }
    free(path_log);
    free(nombre_log);
}


void iniciar_config_conexion(){
    config_conexion = start_config("./configs/conection.config");
    	if(config_conexion==NULL){
    		perror("No se pudo crear la config");
    		exit(EXIT_FAILURE);
    }

    IP_KERNEL = config_get_string_value(config_conexion, "IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(config_conexion, "PUERTO_KERNEL");

   // if (TIPO_INTERFAZ!=GENERICA)
    //{
        IP_MEMORIA = config_get_string_value(config_conexion, "IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(config_conexion, "PUERTO_MEMORIA");
    //}
    


}


void iniciar_config(char* config_interfaz)
{   
    char* path_config = string_duplicate("./configs/");
    string_append(&path_config, config_interfaz);
    string_append(&path_config, ".config");
    log_debug(logger,"PATH CONFIG: %s",path_config);
    config = start_config(path_config);
	if(config==NULL){
		perror("No se pudo crear la config");
		exit(EXIT_FAILURE);
        
	}

    TIPO_INTERFAZ = config_get_string_value(config, "TIPO_INTERFAZ");
    cod_interfaz interfaz = get_tipo_interfaz(TIPO_INTERFAZ);

    switch (interfaz)
    {
    case GENERICA:
        TIEMPO_UNIDAD_TRABAJO = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");    
        //IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
        //PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
        break;
    case STDIN:
    case STDOUT:
        //IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
        //PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
        //IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
        //PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
        break;
    case DIALFS:
        //IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
        //PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
        //IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
        //PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
        TIEMPO_UNIDAD_TRABAJO = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
        PATH_BASE_DIALFS = config_get_string_value(config, "PATH_BASE_DIALFS");
        BLOCK_SIZE = config_get_int_value(config, "BLOCK_SIZE");
        BLOCK_COUNT = config_get_int_value(config, "BLOCK_COUNT");
        RETRASO_COMPACTACION = config_get_int_value(config, "RETRASO_COMPACTACION");
        break;
    default:
		perror("No se pudo cargar datos de la config");
		exit(EXIT_FAILURE);
        break;
    }
    free(path_config);
}

cod_interfaz get_tipo_interfaz(char* TIPO_INTERFAZ){
    if (string_equals_ignore_case(TIPO_INTERFAZ, "GENERICA")) {return GENERICA;}
    else if (string_equals_ignore_case(TIPO_INTERFAZ, "STDIN")) {return STDIN;}
    else if (string_equals_ignore_case(TIPO_INTERFAZ, "STDOUT")) {return STDOUT;}
    else if (string_equals_ignore_case(TIPO_INTERFAZ, "DIALFS")) {return DIALFS;}
    else {
        log_error(logger, "Se intento crear una interfaz de un tipo no valido");
        return -1;
    }
}