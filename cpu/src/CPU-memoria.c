#include "../include/CPU-memoria.h"

void gestionar_conexion_memoria()
{
    //ENVIO MENSAJE A MEMORIA
    enviar_mensaje("CONEXION CON CPU OK", socket_cpu_memoria);
    log_info(logger, "Handshake enviado: MEMORIA");
    
    op_code operacion;
    bool continuar_iterando = true;
    uint32_t size;
    uint32_t desplazamiento=0;
    void* buffer;

    while (continuar_iterando)
    {   

        operacion = recibir_operacion(socket_cpu_memoria);
        switch (operacion)
        {
        case MENSAJE:
            recibir_mensaje(socket_cpu_memoria, logger_debug);
            break;

        case FETCH:
            ins_actual = recibir_instruccion();
            log_info(logger, "CPU recibe una instruccion de memoria, codigo: %s", codigo_instruccion_string(ins_actual->ins) );
            sem_post(&prox_instruccion);
            break;

        case PROCESO_NO_CARGADO:
            ins_actual = malloc(sizeof(t_instruccion));
            ins_actual->ins = EXIT;
            log_warning(logger, "CPU pidio una instruccion de un proceso que no esta cargado en memoria, PID: %u", PID);
            
            sem_post(&prox_instruccion);
            break;
        
        case SOLICITUD_RESIZE:
            log_info(logger, "PID: %u, realizo resize con exito", PID);
            resize_ok = true;
            sem_post(&respuesta_resize);
            break;
        
        case OUT_OF_MEMORY:
            log_info(logger, "PID: %u, no pudo realizar resize por falta de espacio en memoria", PID);
            resize_ok = false;
            sem_post(&respuesta_resize);
            break;

        case SOLICITUD_COPY_STRING_READ:
            desplazamiento = 0;
            buffer = recibir_buffer(&size, socket_cpu_memoria);
            string_leida_de_memoria = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger_debug, "Llego la string \'%s\' de memoria", string_leida_de_memoria);
            sem_post(&respuesta_copy_string);
            free(buffer);
            break;
            
        case SOLICITUD_COPY_STRING_WRITE:
            desplazamiento = 0;
            buffer = recibir_buffer(&size, socket_cpu_memoria);
            op_code codigo_respuesta = leer_de_buffer_op_code(buffer, &desplazamiento);
            if (codigo_respuesta == OK)
                {log_info(logger_debug, "COPY_STRING exitoso");}
            else            
                {log_info(logger_debug, "COPY_STRING fallido");}
            free(buffer);
            break;

        case SOLICITUD_MOV_IN:
            desplazamiento = 0;
            buffer = recibir_buffer(&size, socket_cpu_memoria);

            uint32_t tamanio = leer_de_buffer_uint32(buffer, &desplazamiento);
            uint32_t respuesta = leer_de_buffer_uint32(buffer, &desplazamiento);
            // log_debug(logger_debug, "valor respuesta MOV_IN: %u", respuesta);

            if (tamanio == sizeof(uint8_t))
                {respuesta_mov_in_8 = respuesta;}
            else {respuesta_mov_in_32 = respuesta;}
            
            sem_post(&respuesta_MOV_IN);
            free(buffer);
            break;

        case SOLICITUD_MOV_OUT:
            desplazamiento = 0;
            buffer = recibir_buffer(&size, socket_cpu_memoria);
            op_code exito_mov_out = leer_de_buffer_op_code(buffer, &desplazamiento);
            if (exito_mov_out == OK)
                {log_info(logger_debug, "PID: %u - MOV_OUT exitoso", PID);}
            else
                {log_error(logger_debug, "PID: %u - MOV_OUT fallido", PID);}
            free(buffer);
            break;

        case TLB_MISS:
            desplazamiento = 0;
            buffer = recibir_buffer(&size, socket_cpu_memoria);
            marco_pedido = leer_de_buffer_uint32(buffer, &desplazamiento);
            
            sem_post(&respuesta_marco);
            free(buffer);
            break;

        case FALLO:
            log_error(logger, "Modulo MEMORIA desconectado, terminando servidor");
            continuar_iterando = false;
            break;

        default:
            log_error(logger, "Llego una operacion desconocida por socket memoria, op_code: %d", operacion);
            break;
        }
    }
}

void fetch(uint32_t PID, uint32_t PC){
    log_info(logger, "PID: %u - FETCH - Program Counter: %u", PID, PC);
    t_paquete* p = crear_paquete(FETCH);
    agregar_a_paquete_uint32(p, PID);
    agregar_a_paquete_uint32(p, PC);
    enviar_paquete(p, socket_cpu_memoria);
    eliminar_paquete(p);
}

t_instruccion* recibir_instruccion(){
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    
    uint32_t size;
    uint32_t desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cpu_memoria);

    instr->ins = leer_de_buffer_cod_ins(buffer, &desplazamiento);
    instr->arg1 = leer_de_buffer_string(buffer, &desplazamiento);
    instr->arg2 = leer_de_buffer_string(buffer, &desplazamiento);
    instr->arg3 = leer_de_buffer_string(buffer, &desplazamiento);
    instr->arg4 = leer_de_buffer_string(buffer, &desplazamiento);
    instr->arg5 = leer_de_buffer_string(buffer, &desplazamiento);

    free(buffer);

    return instr;
}

void recibir_tamanio_de_pagina()
{
    op_code codigo = recibir_operacion(socket_cpu_memoria);
    if (codigo != TAM_PAG){
        log_error(logger, "Llego otra cosa en lugar del tamaño maximo de pagina, codigo: %d", codigo);
        tamanio_de_pagina = 0;
    }
    else
    {
        uint32_t size;
        uint32_t desplazamiento = 0;
        void* buffer = recibir_buffer(&size, socket_cpu_memoria);

        tamanio_de_pagina = leer_de_buffer_uint32(buffer, &desplazamiento);
        log_info(logger, "Llego el tamanio de pagina: %u", tamanio_de_pagina);
        free(buffer);
    }
}

void pedir_rezise(uint32_t PID, uint32_t valor)
{
    t_paquete* paquete = crear_paquete(SOLICITUD_RESIZE);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, valor);
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
}
