#include "../include/CPU_Main.h"

int main(int argc, char* argv[]) 
{
    //printf("Argumento : %s\n", argv[1]);   
      
    //VALIDO ARGUMENTOS
    //validar_argumento(argv[1]);
    
    //char* parametros = argv[1];

//INICIO DE CPU
    iniciar_CPU();

// INICIAR SERVIDOR
    socket_escucha_dispatch = iniciar_servidor(puerto_escucha_dispatch, logger_debug);
    socket_escucha_interrupt = iniciar_servidor(puerto_escucha_interrupt, logger_debug);

//CREAR CONEXION CON MEMORIA
    socket_cpu_memoria = crear_conexion(ip_memoria, puerto_memoria);
    log_info(logger, "Conectado a MEMORIA");
    recibir_tamanio_de_pagina();
    inicializar_TLB();
 
// ESPERAR CONEXION CON KERNEL
    socket_cpu_kernel_dispatch = esperar_cliente(socket_escucha_dispatch, logger_debug);
    log_info(logger_debug, "Conectado a KERNEL dispatch");

    socket_cpu_kernel_interrupt = esperar_cliente(socket_escucha_interrupt, logger_debug);
    log_info(logger_debug, "Conectado a KERNEL interrupt");

// CREACION DE HILOS
    pthread_create(&hilo_conexion_dispatch, NULL, (void*) gestionar_conexion_dispatch, NULL);
    pthread_detach(hilo_conexion_dispatch);

    pthread_create(&hilo_conexion_interrupt, NULL, (void*) gestionar_conexion_interrupt, NULL);
    pthread_detach(hilo_conexion_interrupt);

    pthread_create(&hilo_conexion_memoria, NULL, (void*) gestionar_conexion_memoria, NULL);
    pthread_detach(hilo_conexion_memoria);
        
          
            
    
    while(true){
        pthread_mutex_lock(&mutex_detenerEjecucion);

        if (detener_ejecucion)
        {   
            pthread_mutex_unlock(&mutex_detenerEjecucion);
            log_trace(logger,"Esperando un proceso");
            sem_post(&espera_iterador);
            sem_wait(&hay_proceso_ejecutando);
        }
        else
        {
            pthread_mutex_unlock(&mutex_detenerEjecucion);
        }
        
        
        fetch(PID, contexto_interno.PC);
        sem_wait(&prox_instruccion);
        ejecutar_instruccion(PID, &contexto_interno, ins_actual);
        
        // loggear_valores();
        
        if ((int_consola || int_quantum) && !instruccion_de_IO_o_exit(ins_actual->ins) && motivo_desalojo!= OUT_OF_MEMORY) 
        {
            if (int_consola) 
            {
                log_info(logger, "Gestionando desalojo por consola");
                motivo_desalojo = DESALOJO_POR_CONSOLA;
            }
            else if (int_quantum) 
            {
                log_info(logger, "Gestionando desalojo por quantum");
                motivo_desalojo = DESALOJO_POR_QUANTUM;
            }
            desalojar_proceso(motivo_desalojo);
        }else{
            motivo_desalojo=DESALOJO_POR_QUANTUM;  //esto lo pongo para resetear el motivo desalojo
        };
        destruir_instruccion(ins_actual);
    }


    if (socket_cpu_kernel_dispatch) {liberar_conexion(socket_cpu_kernel_dispatch);}
    if (socket_cpu_kernel_interrupt) {liberar_conexion(socket_cpu_kernel_interrupt);}
    if (socket_cpu_memoria) {liberar_conexion(socket_cpu_memoria);}
    if (socket_escucha_dispatch) {liberar_conexion(socket_escucha_dispatch);}
    if (socket_escucha_interrupt) {liberar_conexion(socket_escucha_interrupt);}

    config_destroy(config);
    log_destroy(logger);
    log_destroy(logger_valores);
    log_destroy(logger_debug);

    return 0;
}





void ejecutar_instruccion(uint32_t PID, t_contexto_ejecucion* contexto_interno, t_instruccion* ins_actual){
    cod_ins codigo = ins_actual->ins;
    int* registro_1;
    int* registro_2;
    int* registro_3;

    uint8_t valorchico1;
    uint8_t valorchico2;
    uint32_t valorgrande1;
    uint32_t valorgrande2;

    uint32_t direccion_logica;

    switch (codigo)
    {
    
    case VOLVER:

    break;

    case SET:
        log_info(logger,"PID: %u - Ejecutando: SET - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        registro_1 = direccion_registro(contexto_interno, ins_actual->arg1);
        if(registro_chico(ins_actual->arg1))
        {
            valorchico1 = atoi(ins_actual->arg2);
            memcpy(registro_1, &valorchico1, sizeof(uint8_t));
        }
        else
        {
            valorgrande1 = atoi(ins_actual->arg2); 
            memcpy(registro_1, &valorgrande1, sizeof(uint32_t));
        }
        break;

    case SUM:
        log_info(logger,"PID: %u - Ejecutando: SUM - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        registro_1 = direccion_registro(contexto_interno, ins_actual->arg1);
        registro_2 = direccion_registro(contexto_interno, ins_actual->arg2);

        if(registro_chico(ins_actual->arg1))
        {
            valorchico1 = *registro_1;
            valorgrande1=valorchico1;
        }
        else
            {valorgrande1=*registro_1;}
        if(registro_chico(ins_actual->arg2))
        {
            valorchico2 = *registro_2;
            valorgrande2=valorchico2;
        }
        else
            {valorgrande2=*registro_2;}

        uint32_t suma = valorgrande1 + valorgrande2;
        // log_debug(logger_debug, "a= %u, b= %u, suma: %u", valorgrande1, valorgrande2, suma);

        if(registro_chico(ins_actual->arg1))
        {
            valorchico1 = suma;// Convertirlo a uint8 si hace falta
            // log_debug(logger_valores, "Resultado suma: %u", valorchico1);
            memcpy(registro_1, &valorchico1, sizeof(uint8_t));
        }
        else
        {
            // log_debug(logger_valores, "Resultado suma: %u", suma);
            memcpy(registro_1, &suma, sizeof(uint32_t));
        }
        break;

    case SUB:
        log_info(logger,"PID: %u - Ejecutando: SUB - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        registro_1 = direccion_registro(contexto_interno, ins_actual->arg1);
        registro_2 = direccion_registro(contexto_interno, ins_actual->arg2);
        
        if(registro_chico(ins_actual->arg1))
        {
            valorchico1 = *registro_1;
            valorgrande1=valorchico1;
        }
        else
            {valorgrande1=*registro_1;}
        if(registro_chico(ins_actual->arg2))
        {
            valorchico2 = *registro_2;
            valorgrande2=valorchico2;
        }
        else
            {valorgrande2=*registro_2;}

        uint32_t resta = valorgrande1 + valorgrande2;
        // log_debug(logger_debug, "a= %u, b= %u, resta: %u", valorgrande1, valorgrande2, resta);

        if(registro_chico(ins_actual->arg1))
        {
            valorchico1 = resta;// Convertirlo a uint8 si hace falta
            // log_debug(logger_valores, "Resultado resta: %u", valorchico1);
            memcpy(registro_1, &valorchico1, sizeof(uint8_t));
        }
        else
        {
            // log_debug(logger_valores, "Resultado resta: %u", resta);
            memcpy(registro_1, &resta, sizeof(uint32_t));
        }
        break;
        
    case JNZ:
        log_info(logger,"PID: %u - Ejecutando: JNZ - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro_1 = direccion_registro(contexto_interno, ins_actual->arg1);
        valorgrande1 = (uint32_t) *registro_1;
        uint32_t nuevo_pc = atoi(ins_actual->arg2);

        if (valorgrande1 != 0) {memcpy(&contexto_interno->PC, &nuevo_pc, sizeof(uint32_t));}
        else {contexto_interno->PC++;}
        break;

    case MOV_IN:
        log_info(logger,"PID: %u - Ejecutando: MOV_IN - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro_1 = direccion_registro(contexto_interno, ins_actual->arg1); //puntero donde se guarda el dato
        registro_2 = direccion_registro(contexto_interno ,ins_actual->arg2); //puntero que contiene la direccion logica de memoria
        uint32_t dir_fisica_read;

        if (registro_chico(ins_actual->arg2))
        {
            memcpy(&valorchico1, registro_2, sizeof(uint8_t));
            direccion_logica = valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_2, sizeof(uint32_t));
            direccion_logica = valorgrande1;
        }

        if (registro_chico(ins_actual->arg1))
        {
            dir_fisica_read = solicitar_MOV_IN(direccion_logica, sizeof(uint8_t));
            sem_wait(&respuesta_MOV_IN);
            valorchico1 = respuesta_mov_in_8;
            memcpy(registro_1, &valorchico1, sizeof(uint8_t));
            log_info(logger, "PID: %u - Acción: LEER - Dirección Física: %u - Valor: %u", PID, dir_fisica_read, valorchico1);
        }
        else
        {
            dir_fisica_read = solicitar_MOV_IN(direccion_logica, sizeof(uint32_t));
            sem_wait(&respuesta_MOV_IN);
            valorgrande1 = respuesta_mov_in_32;
            memcpy(registro_1, &valorgrande1, sizeof(uint32_t));
            log_info(logger, "PID: %u - Acción: LEER - Dirección Física: %u - Valor: %u", PID, dir_fisica_read, valorgrande1);
        }

        contexto_interno->PC++;        
        break;

    case MOV_OUT:
        log_info(logger,"PID: %u - Ejecutando: MOV_OUT - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        registro_2 = direccion_registro(contexto_interno, ins_actual->arg2); //puntero donde esta almacenado el valor a escribir
        registro_1 = direccion_registro(contexto_interno, ins_actual->arg1);//puntero que contiene la direccion logica de memoria 
        uint32_t dir_fisica_write;

        if (registro_chico(ins_actual->arg1))
        {
            memcpy(&valorchico1, registro_1, sizeof(uint8_t));
            direccion_logica = valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_1, sizeof(uint32_t));
            direccion_logica = valorgrande1;
        }
        // log_debug(logger_debug, "MOV_OUT direccion logica: %u", direccion_logica);
        
        if (registro_chico(ins_actual->arg2))
        {
            memcpy(&valorchico2, registro_2, sizeof(uint8_t));
            dir_fisica_write = solicitar_MOV_OUT(direccion_logica, sizeof(uint8_t), valorchico2);
            log_info(logger, "PID: %u - Acción: ESCRIBIR - Dirección Física: %u - Valor: %u", PID, dir_fisica_write, valorchico2);
        }
        else
        {
            memcpy(&valorgrande2, registro_2, sizeof(uint32_t));
            dir_fisica_write = solicitar_MOV_OUT(direccion_logica, sizeof(uint32_t), valorgrande2);
            log_info(logger, "PID: %u - Acción: ESCRIBIR - Dirección Física: %u - Valor: %u", PID, dir_fisica_write, valorgrande2);
        }
        contexto_interno->PC++;
        break;

    case RESIZE:
        log_info(logger,"PID: %u - Ejecutando: RESIZE - %s", PID, ins_actual->arg1);
        contexto_interno->PC++;
        uint32_t new_size = atoi(ins_actual->arg1);
        pedir_rezise(PID, new_size);
        sem_wait(&respuesta_resize);
        if (!resize_ok)
        {
            motivo_desalojo = OUT_OF_MEMORY;
            desalojar_proceso(motivo_desalojo);
            log_warning(logger, "PID: %u, es desaojado por OUT_OF_MEMORY", PID);
        }        
        break;

    case COPY_STRING:
        log_info(logger,"PID: %u - Ejecutando: COPY_STRING - %s", PID, ins_actual->arg1);
        uint32_t bytes_a_copiar = atoi(ins_actual->arg1);
        uint32_t direccion_logica_READ;
        memcpy(&direccion_logica_READ, &contexto_interno->SI, sizeof(uint32_t));
        uint32_t direccion_logica_WRITE;
        memcpy(&direccion_logica_WRITE, &contexto_interno->DI, sizeof(uint32_t));
        // log_debug(logger_debug, "COPY_STRING: Direcciones leidas - READ: %u, WRITE: %u", direccion_logica_READ, direccion_logica_WRITE);

        solicitar_lectura_string(direccion_logica_READ, bytes_a_copiar);
        sem_wait(&respuesta_copy_string);
        char* str_leida = string_leida_de_memoria;
        escribir_en_memoria_string(str_leida, direccion_logica_WRITE, bytes_a_copiar);
        free(string_leida_de_memoria);
        contexto_interno->PC++;
        break;
        
    case IO_GEN_SLEEP:
        log_info(logger,"PID: %u - Ejecutando: IO_GEN_SLEEP - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_GEN_SLEEP;
        // arg1: nombre_interfaz, arg2: unidades_trabajo
        uint32_t unidades = atoi(ins_actual->arg2);
        solicitar_IO_GEN_SLEEP(motivo_desalojo, ins_actual->arg1, unidades);
        break;

    case IO_STDIN_READ:
        log_info(logger,"PID: %u - Ejecutando: IO_STDIN_READ - %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_STDIN;
        
        registro_1 = direccion_registro(contexto_interno, ins_actual->arg2);
        if (registro_chico(ins_actual->arg2))
        {
            memcpy(&valorchico1, registro_1, sizeof(uint8_t));
            direccion_logica = valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_1, sizeof(uint32_t));
            direccion_logica = valorgrande1;
        }

        registro_2 = direccion_registro(contexto_interno, ins_actual->arg3);
        uint32_t tamanio_a_leer;
        if (registro_chico(ins_actual->arg3))
        {
            memcpy(&valorchico2, registro_2, sizeof(uint8_t));
            tamanio_a_leer =  valorchico2;
        }
        else
        {
            memcpy(&valorgrande2, registro_2, sizeof(uint32_t));
            tamanio_a_leer =  valorgrande2;
        }
        // log_debug(logger_debug, "IO_STDIN parametros - direccion logica: %u, tamanio leido: %u", direccion_logica, tamanio_a_leer);
        
        ejecutar_IO_STD_IN(ins_actual->arg1, direccion_logica, tamanio_a_leer);
        break;

    case IO_STDOUT_WRITE:
        log_info(logger,"PID: %u - Ejecutando: IO_STDOUT_WRITE - %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_STDOUT;

        registro_1 = direccion_registro(contexto_interno, ins_actual->arg2);
        if (registro_chico(ins_actual->arg2))
        {
            memcpy(&valorchico1, registro_1, sizeof(uint8_t));
            direccion_logica = valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_1, sizeof(uint32_t));
            direccion_logica = valorgrande1;
        }
        
        registro_2 = direccion_registro(contexto_interno, ins_actual->arg3);
        uint32_t tamanio_a_escribir;
        if (registro_chico(ins_actual->arg3))
        {
            memcpy(&valorchico2, registro_2, sizeof(uint8_t));
            tamanio_a_escribir =  valorchico2;
        }
        else
        {
            memcpy(&valorgrande2, registro_2, sizeof(uint32_t));
            tamanio_a_escribir =  valorgrande2;
        }
        // log_debug(logger_debug, "IO_STDOUT parametros - direccion: %u, tamanio leido: %u", direccion_logica, tamanio_a_escribir);

        ejecutar_IO_STD_OUT(ins_actual->arg1, direccion_logica, tamanio_a_escribir);
        break;

    case IO_FS_CREATE:
        log_info(logger,"PID: %u - Ejecutando: IO_FS_CREATE - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_CREATE;
        // arg1: nombre_interfaz, arg2: nombre_archivo
        enviar_CE_con_2_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2);
        break;

    case IO_FS_DELETE:
        log_info(logger,"PID: %u - Ejecutando: IO_FS_DELETE - %s %s", PID, ins_actual->arg1, ins_actual->arg2);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_DELETE;
        // arg1: nombre_interfaz, arg2: nombre_archivo
        enviar_CE_con_2_arg(motivo_desalojo, ins_actual->arg1, ins_actual->arg2);
        break;

    case IO_FS_TRUNCATE:
        log_info(logger,"PID: %u - Ejecutando: IO_FS_TRUNCATE - %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_TRUNCATE;

        registro_1 = direccion_registro(contexto_interno, ins_actual->arg3);
        uint32_t tamanio_truncar;
        if (registro_chico(ins_actual->arg3))
        {
            memcpy(&valorchico1, registro_1, sizeof(uint8_t));
            tamanio_truncar =  valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_1, sizeof(uint32_t));
            tamanio_truncar =  valorgrande1;
        }
        // log_debug(logger_debug, "Tamanio truncate leido: %u", tamanio_truncar);

        // arg1: nombre_interfaz, arg2: nombre_archivo, valorgrande1 nuevo_tamaño
        solicitar_IO_FS_TRUNCATE(ins_actual->arg1, ins_actual->arg2, tamanio_truncar);
        break;

    case IO_FS_WRITE:
        log_info(logger,"PID: %u - Ejecutando: IO_FS_WRITE - %s %s %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3, ins_actual->arg4, ins_actual->arg5);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_WRITE;

        // Direccion a leer
        registro_1 = direccion_registro(contexto_interno, ins_actual->arg3);
        if (registro_chico(ins_actual->arg3))
        {
            memcpy(&valorchico1, registro_1, sizeof(uint8_t));
            direccion_logica = valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_1, sizeof(uint32_t));
            direccion_logica = valorgrande1;
        }
        
        // Tamaño a leer
        uint32_t tamanio_write;
        registro_2 = direccion_registro(contexto_interno, ins_actual->arg4);
        if (registro_chico(ins_actual->arg4))
        {
            memcpy(&valorchico1, registro_2, sizeof(uint8_t));
            tamanio_write = valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_2, sizeof(uint32_t));
            tamanio_write = valorgrande1;
        }
        
        // Puntero escritura
        uint32_t puntero_write;
        registro_3 = direccion_registro(contexto_interno, ins_actual->arg5);
        if (registro_chico(ins_actual->arg5))
        {
            memcpy(&valorchico1, registro_3, sizeof(uint8_t));
            puntero_write = valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_3, sizeof(uint32_t));
            puntero_write = valorgrande1;
        }

        // log_debug(logger_debug, "FS_WRITE direccion logica: %u, tamanio: %u, puntero: %u", direccion_logica, tamanio_write, puntero_write);
        solicitar_IO_FS_MEMORIA(motivo_desalojo, ins_actual->arg1, ins_actual->arg2, direccion_logica, tamanio_write, puntero_write);
        break;

    case IO_FS_READ:
        log_info(logger,"PID: %u - Ejecutando: IO_FS_READ - %s %s %s %s %s", PID, ins_actual->arg1, ins_actual->arg2, ins_actual->arg3, ins_actual->arg4, ins_actual->arg5);
        contexto_interno->PC++;
        motivo_desalojo = DESALOJO_POR_IO_FS_READ;

        // Direccion a leer
        registro_1 = direccion_registro(contexto_interno, ins_actual->arg3);
        if (registro_chico(ins_actual->arg3))
        {
            memcpy(&valorchico1, registro_1, sizeof(uint8_t));
            direccion_logica = valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_1, sizeof(uint32_t));
            direccion_logica = valorgrande1;
        }
        
        // Tamaño a leer
        uint32_t tamanio_read;
        registro_2 = direccion_registro(contexto_interno, ins_actual->arg4);
        if (registro_chico(ins_actual->arg4))
        {
            memcpy(&valorchico1, registro_2, sizeof(uint8_t));
            tamanio_read = valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_2, sizeof(uint32_t));
            tamanio_read = valorgrande1;
        }
        
        // Puntero escritura
        uint32_t puntero_read;
        registro_3 = direccion_registro(contexto_interno, ins_actual->arg5);
        if (registro_chico(ins_actual->arg4))
        {
            memcpy(&valorchico1, registro_3, sizeof(uint8_t));
            puntero_read = valorchico1;
        }
        else
        {
            memcpy(&valorgrande1, registro_3, sizeof(uint32_t));
            puntero_read = valorgrande1;
        }

        // log_debug(logger_debug, "FS_READ direccion logica: %u, tamanio: %u, puntero: %u", direccion_logica, tamanio_read, puntero_read);
        solicitar_IO_FS_MEMORIA(motivo_desalojo, ins_actual->arg1, ins_actual->arg2, direccion_logica, tamanio_read, puntero_read);
        break;

    case WAIT:
        log_info(logger,"PID: %u - Ejecutando: WAIT - %s", PID, ins_actual->arg1);
        contexto_interno->PC++;
        enviar_CE_con_1_arg(DESALOJO_POR_WAIT, ins_actual->arg1);
        break;

    case SIGNAL:
        log_info(logger,"PID: %u - Ejecutando: SIGNAL - %s", PID, ins_actual->arg1);
        contexto_interno->PC++;
        enviar_CE_con_1_arg(DESALOJO_POR_SIGNAL, ins_actual->arg1);
        break;

    case EXIT:
        log_info(logger,"PID: %u - Ejecutando: EXIT", PID);
        motivo_desalojo = DESALOJO_POR_FIN_PROCESO;
        desalojar_proceso(motivo_desalojo);
        break;
        
    default:
        log_error(logger, "Proceso no cargado en memoria");
        break;
    }
}

void* direccion_registro(t_contexto_ejecucion* contexto, char* registro)
{
    if (string_equals_ignore_case(registro, "PC"))  {return &(contexto->PC);}
    else if (string_equals_ignore_case(registro, "AX"))  {return &(contexto->AX);}
    else if (string_equals_ignore_case(registro, "BX"))  {return &(contexto->BX);}
    else if (string_equals_ignore_case(registro, "CX"))  {return &(contexto->CX);}
    else if (string_equals_ignore_case(registro, "DX"))  {return &(contexto->DX);}
    else if (string_equals_ignore_case(registro, "EAX"))  {return &(contexto->EAX);}
    else if (string_equals_ignore_case(registro, "EBX"))  {return &(contexto->EBX);}
    else if (string_equals_ignore_case(registro, "ECX"))  {return &(contexto->ECX);}
    else if (string_equals_ignore_case(registro, "EDX"))  {return &(contexto->EDX);}
    else if (string_equals_ignore_case(registro, "SI"))  {return &(contexto->SI);}
    else if (string_equals_ignore_case(registro, "DI"))  {return &(contexto->DI);}
    else {
        log_error(logger, "Error en traduccion de string a registro");
        return NULL;
    }
}

bool registro_chico(char* registro)
{
    return (
        string_equals_ignore_case(registro, "AX") || 
        string_equals_ignore_case(registro, "BX") || 
        string_equals_ignore_case(registro, "CX") || 
        string_equals_ignore_case(registro, "DX")
    );
}

void ejecutar_IO_STD_IN(char* nombre_interfaz, uint32_t direccion_logica, uint32_t tamanio_a_leer)
{
    t_paquete* paquete = crear_paquete(DESALOJO_POR_IO_STDIN);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, string_length(nombre_interfaz)+1, nombre_interfaz);
    agregar_a_paquete_uint32(paquete, tamanio_a_leer);

    uint32_t bytes_restantes = tamanio_a_leer;
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica);
    uint32_t offset = obtener_desplazamiento(direccion_logica);
    uint32_t marco;

    float fin_lectura = (tamanio_a_leer + offset);
    float cant_paginas = fin_lectura / tamanio_de_pagina;
    uint32_t cant_accesos = ceil(cant_paginas);
    agregar_a_paquete_uint32(paquete, cant_accesos);

    marco = get_marco(PID, nro_pag);
    uint32_t dir_fisica = (marco*tamanio_de_pagina)+offset;
    uint32_t tam_acceso = cant_accesos==1 ? tamanio_a_leer : (tamanio_de_pagina-offset);

    agregar_a_paquete_uint32(paquete, dir_fisica);
    agregar_a_paquete_uint32(paquete, tam_acceso);
    // log_debug(logger_debug, "IO_STDIN acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);
    bytes_restantes -= tam_acceso;

    int i = 1;
    while (bytes_restantes>0)
    {
        marco = get_marco(PID, nro_pag+i);
        dir_fisica = marco*tamanio_de_pagina;

        if (bytes_restantes>tamanio_de_pagina)
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_uint32(paquete, tamanio_de_pagina);
            // log_debug(logger_debug, "IO_STDIN acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);
            bytes_restantes-=tamanio_de_pagina;
        }
        else
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_uint32(paquete, bytes_restantes);
            // log_debug(logger_debug, "IO_STDIN acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);
            bytes_restantes-=bytes_restantes; //aca sale del while
        }
        ++i;
    }
    
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    log_info(logger, "El proceso PID: %u es desalojado, motivo: %s", PID, codigo_operacion_string(motivo_desalojo));
    pthread_mutex_lock(&mutex_detenerEjecucion);
    detener_ejecucion=true;
    pthread_mutex_unlock(&mutex_detenerEjecucion);
}

void ejecutar_IO_STD_OUT(char* nombre_interfaz, uint32_t direccion_logica, uint32_t tamanio_a_leer)
{
    uint32_t bytes_restantes = tamanio_a_leer;
    t_paquete* paquete = crear_paquete(DESALOJO_POR_IO_STDOUT);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, string_length(nombre_interfaz)+1, nombre_interfaz);
    agregar_a_paquete_uint32(paquete, tamanio_a_leer);

    uint32_t nro_pag = obtener_nro_pagina(direccion_logica);
    uint32_t offset = obtener_desplazamiento(direccion_logica);
    uint32_t marco;
    
    float fin_lectura = (tamanio_a_leer + offset);
    float cant_paginas = fin_lectura / tamanio_de_pagina;
    uint32_t cant_accesos = ceil(cant_paginas);
    agregar_a_paquete_uint32(paquete, cant_accesos);

    marco = get_marco(PID, nro_pag);
    uint32_t dir_fisica = (marco*tamanio_de_pagina)+offset;
    uint32_t tam_acceso = cant_accesos==1 ? tamanio_a_leer : (tamanio_de_pagina-offset);

    agregar_a_paquete_uint32(paquete, dir_fisica);
    agregar_a_paquete_uint32(paquete, tam_acceso);
    // log_debug(logger_debug, "IO_STDOUT acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);
    bytes_restantes -= tam_acceso;

    int i = 1;
    while (bytes_restantes>0)
    {
        marco = get_marco(PID, nro_pag+i);
        dir_fisica = marco*tamanio_de_pagina;

        if (bytes_restantes>tamanio_de_pagina)
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_uint32(paquete, tamanio_de_pagina);
            // log_debug(logger_debug, "IO_STDOUT acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);
            bytes_restantes-=tamanio_de_pagina;
        }
        else
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_uint32(paquete, bytes_restantes);
            // log_debug(logger_debug, "IO_STDOUT acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);
            bytes_restantes-=bytes_restantes; //aca sale del while
        }
        ++i;
    }
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    log_info(logger, "El proceso PID: %u es desalojado, motivo: %s", PID, codigo_operacion_string(motivo_desalojo));
    pthread_mutex_lock(&mutex_detenerEjecucion);
    detener_ejecucion=true;
    pthread_mutex_unlock(&mutex_detenerEjecucion);
}

void solicitar_IO_FS_TRUNCATE(char* nombre_interfaz, char* nombre_archivo, uint32_t tamanio)
{
    /*Paquete
    op_code
    uint32 PID
    CE contexto
    string nombre_interfaz
    string nombre_archivo
    uint32 nuevo_tam
    */
    t_paquete* paquete = crear_paquete(DESALOJO_POR_IO_FS_TRUNCATE);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(nombre_interfaz) + 1, nombre_interfaz);
    agregar_a_paquete_string(paquete, strlen(nombre_archivo) + 1, nombre_archivo);
    agregar_a_paquete_uint32(paquete, tamanio);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    pthread_mutex_lock(&mutex_detenerEjecucion);
    detener_ejecucion=true;
    pthread_mutex_unlock(&mutex_detenerEjecucion);
};

void solicitar_IO_FS_MEMORIA(op_code motivo_desalojo, char* nombre_interfaz, char* nombre_archivo, uint32_t direccion_logica, uint32_t tamanio_a_leer, uint32_t puntero)
{
    /*Paquete
    op_code
    uint32 PID
    ---CE contexto (no van para E/S)
    ---string nombre_interfaz (no van para E/S)
    string nombre_archivo
    uint32 tam_total_a_leer
    uint32 puntero
    uint32 cant_accesos
    uint32 dir_fisica (se repiten)
    uint32 tam_acceso (se repiten)
    */

    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(nombre_interfaz) + 1, nombre_interfaz);
    agregar_a_paquete_string(paquete, strlen(nombre_archivo) + 1, nombre_archivo);
    agregar_a_paquete_uint32(paquete, tamanio_a_leer);
    agregar_a_paquete_uint32(paquete, puntero);

    uint32_t bytes_restantes = tamanio_a_leer;
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica);
    uint32_t offset = obtener_desplazamiento(direccion_logica);

    float fin_lectura = (tamanio_a_leer + offset);
    float cant_paginas = fin_lectura / tamanio_de_pagina;
    uint32_t cant_accesos = ceil(cant_paginas);
    agregar_a_paquete_uint32(paquete, cant_accesos);

    uint32_t marco = get_marco(PID, nro_pag);
    uint32_t dir_fisica = (marco*tamanio_de_pagina)+offset;
    uint32_t tam_acceso = cant_accesos==1 ? tamanio_a_leer : (tamanio_de_pagina-offset);

    agregar_a_paquete_uint32(paquete, dir_fisica);
    agregar_a_paquete_uint32(paquete, tam_acceso);
    // log_debug(logger_debug, "IO_FS_MEMORIA acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);
    bytes_restantes -= tam_acceso;

    int i = 1;
    while (bytes_restantes>tamanio_de_pagina)
    {
        marco = get_marco(PID, nro_pag+i);
        dir_fisica = (marco*tamanio_de_pagina);

        agregar_a_paquete_uint32(paquete, dir_fisica);
        agregar_a_paquete_uint32(paquete, tamanio_de_pagina);
        // log_debug(logger_debug, "IO_FS_MEMORIA acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);
        bytes_restantes -= tamanio_de_pagina;
        ++i;
    }
    if (bytes_restantes>0)
    {
        marco = get_marco(PID, nro_pag+i);
        dir_fisica = (marco*tamanio_de_pagina);

        agregar_a_paquete_uint32(paquete, dir_fisica);
        agregar_a_paquete_uint32(paquete, bytes_restantes);
        // log_debug(logger_debug, "IO_FS_MEMORIA acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);
        bytes_restantes -= bytes_restantes;
    }
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    log_info(logger, "El proceso PID: %u es desalojado, motivo: %s", PID, codigo_operacion_string(motivo_desalojo));
    pthread_mutex_lock(&mutex_detenerEjecucion);
    detener_ejecucion=true;
    pthread_mutex_unlock(&mutex_detenerEjecucion);
}

void loggear_valores()
{
    log_info(logger_valores, "PID: %u, PC: %u, AX: %u, BX: %u, CX: %u, DX: %u, EAX: %u, EBX: %u, ECX: %u, EDX: %u, SI: %u, DI: %u",
    PID, contexto_interno.PC,
    contexto_interno.AX, contexto_interno.BX, contexto_interno.CX, contexto_interno.DX,
    contexto_interno.EAX, contexto_interno.EBX, contexto_interno.ECX, contexto_interno.EDX,
    contexto_interno.SI, contexto_interno.DI);
}


bool instruccion_de_IO_o_exit(cod_ins instruccion){
    switch (instruccion)
    {
    case IO_GEN_SLEEP:
    case IO_STDIN_READ:
    case IO_STDOUT_WRITE:
    case IO_FS_CREATE:
    case IO_FS_DELETE:
    case IO_FS_TRUNCATE:
    case IO_FS_WRITE:
    case IO_FS_READ:
    case EXIT:
        return	true;
        break;
    
    default:
        return false;
        break;
    }
}

void destruir_instruccion(t_instruccion* instruccion)
{
    if (instruccion->arg1) {free(ins_actual->arg1);}
    if (instruccion->arg2) {free(ins_actual->arg2);}
    if (instruccion->arg3) {free(ins_actual->arg3);}
    if (instruccion->arg4) {free(ins_actual->arg4);}
    if (instruccion->arg5) {free(ins_actual->arg5);}
    free(ins_actual);
}

void validar_argumento(char* parametros)
{
    if(parametros == NULL){
        printf("Agregar argumento 'parametros'");
        exit(EXIT_FAILURE);
    }
}