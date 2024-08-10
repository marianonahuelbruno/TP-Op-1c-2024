#include "../include/memKernel.h"


void conexion_con_kernel(){
    
    //ENVIAR MENSAJE A KERNEL
    enviar_mensaje("CONEXION CON MEMORIA OK", socket_kernel_memoria);
    log_info(logger, "Handshake enviado: KERNEL");
    
    op_code codigo;

    bool continuarIterando = true;
    while (continuarIterando) {
        
        //pthread_mutex_unlock(&accediendo_a_memoria);
        codigo = recibir_operacion(socket_kernel_memoria);   
        //pthread_mutex_lock(&accediendo_a_memoria);
        
        
        switch (codigo){
        case MENSAJE:
            recibir_mensaje(socket_kernel_memoria,logger_debug);
            break;
        case CREAR_PROCESO:
            log_trace(logger_debug, "Llega una peticion de CREAR PROCESO");
            crear_proceso();
            break;
        case ELIMINAR_PROCESO:
            log_trace(logger_debug, "Llega una peticion de ELIMINAR PROCESO");
            eliminar_proceso();
            break;
        default:
            log_error(logger_debug, "Modulo KERNEL se desconectó.Cerrando Socket de kernel");
            continuarIterando = false;
            break;
        }
    }
}

void crear_proceso(){ // llega el pid y el path de instrucciones
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket_kernel_memoria);

    if (buffer != NULL) {
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        char* path_parcial = leer_de_buffer_string(buffer, &desplazamiento);
        char* path;

        if (path_base!=NULL && path_parcial!= NULL)
            {path = path_completo(path_base, path_parcial);}
        else
            {log_error(logger_debug,"Error al obtener el path");}
                
        if (path!=NULL)
            {//log_debug(logger_debug,"Llego un proceso para cargar: PID: %u  Direccion: %s",PID,path);}
        
        bool creado = crear_procesoM(path, PID);

        usleep(retardo*1000);

        if(creado){
            enviar_instruccion_con_PID_por_socket(CARGA_EXITOSA_PROCESO, PID, socket_kernel_memoria);
        }else{
            //log_debug(logger_debug, "Falla al cargar un proceso, PID: %u", PID);
            enviar_instruccion_con_PID_por_socket(ERROR_AL_CARGAR_EL_PROCESO, PID, socket_kernel_memoria);
        }
        free(path);
        free(path_parcial);

    } else {
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}}


void eliminar_proceso(){ // llega un pid
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket_kernel_memoria);

    if(buffer!=NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        //log_debug(logger_debug,"Llego un proceso para eliminar: PID: %u",PID);

        eliminar_procesoM(PID);
        usleep(retardo*1000);
    } else {
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}

            


