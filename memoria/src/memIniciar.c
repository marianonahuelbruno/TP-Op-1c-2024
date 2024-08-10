#include "../include/memIniciar.h"

void inciarlogsYsemaforos(){
    //INICIALIZO LOGGER
    logger = start_logger("log_memoria.log", "LOG MEMORIA", LOG_LEVEL_TRACE);
    logger_debug = start_logger("log_memoria_debug.log", "LOG MEMORIA DEBUG", LOG_LEVEL_TRACE);

     pthread_mutex_init(&mutex_tablaDePaginas, NULL);
     pthread_mutex_init(&mutex_procesos, NULL);
     pthread_mutex_init(&mutex_listaDeinstrucciones, NULL);
     pthread_mutex_init(& mutex_bitmap,NULL);
     //pthread_mutex_init(& accediendo_a_memoria,NULL);
     
}

void cargarConfig(){

    //INICIALIZO config
    config = start_config("./memoria.config");

    if(config==NULL){
        perror("Fallo al crear el archivo config");
        exit(EXIT_FAILURE);
    }

    //OBTENER VALORES CONFIG
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    log_info(logger, "PUERTO leido: %s", puerto_escucha);

    path_base = config_get_string_value(config, "PATH_INSTRUCCIONES");
    log_info(logger, "PATH: %s", path_base);

    tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    log_info(logger, "TAMANIO MEMORIA: %d", tam_memoria);

    tam_pagina = config_get_int_value(config, "TAM_PAGINA");
    log_info(logger, "TAMANIO PAGINA: %d", tam_pagina);

    retardo = config_get_int_value(config, "RETARDO_RESPUESTA");
    log_info(logger, "RETARDO RESPUESTA: %d", retardo);

    cant_frames = tam_memoria/tam_pagina;
}

void inicializarEspacioMem(){
    memoria_usuario = malloc(tam_memoria);

    if(memoria_usuario==NULL){
        perror("Fallo al inicializar memoria de usuario");
        exit(EXIT_FAILURE);
    }

    memset(memoria_usuario, 0, tam_memoria); //INCIALIZA TODA LA MEMORIA EN 0

    log_info(logger, "Memoria de usuario inicializada con %d frames de %d bytes cada uno", cant_frames, tam_pagina);

    int tam_bitarray = (cant_frames + 7) / 8; //TAMAÑO EN BYTES redondeando hacia arriba
    char* bitarray_mem = malloc(tam_bitarray);
    if (bitarray_mem == NULL) {
        log_error(logger_debug, "Fallo asignacion memoria al bitarray");
        exit(EXIT_FAILURE);
    }
    memset(bitarray_mem, 0, tam_bitarray);  // Inicializa el bit array a 0 (todos los marcos libres)
    bitmap = bitarray_create_with_mode(bitarray_mem, tam_bitarray, MSB_FIRST);

    tablaDePaginas = list_create();
    procesos = list_create();
    
    //log_info(logger, "BITMAP inicializado correctamente");
}