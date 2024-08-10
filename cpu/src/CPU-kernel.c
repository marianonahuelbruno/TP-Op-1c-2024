#include "../include/CPU-kernel.h"

void gestionar_conexion_dispatch()
{
    //ENVIO MENSAJE A KERNEL
    enviar_mensaje("CONEXION CON CPU-DISPATCH OK", socket_cpu_kernel_dispatch);
    log_info(logger, "Handshake enviado: KERNEL");
    
    op_code operacion;
    bool continuar_iterando = true;

    while (continuar_iterando)
    {
        operacion = recibir_operacion(socket_cpu_kernel_dispatch);
        
        switch (operacion)
        {
        case MENSAJE:
            recibir_mensaje(socket_cpu_kernel_dispatch,logger_debug);
            break;
        case CONTEXTO:
            sem_wait(&espera_iterador);
            t_contexto_ejecucion contexto_espera;
            recibir_CE(socket_cpu_kernel_dispatch, &PID, &contexto_espera);
            log_info(logger, "Llega un proceso de PID: %u, esperando CPU", PID);
            
            int_consola = false;
            int_quantum = false;
            pthread_mutex_lock(&mutex_detenerEjecucion);
            detener_ejecucion=false;
            pthread_mutex_unlock(&mutex_detenerEjecucion);
            contexto_interno.PC = contexto_espera.PC;
            contexto_interno.AX = contexto_espera.AX;
            contexto_interno.BX = contexto_espera.BX;
            contexto_interno.CX = contexto_espera.CX;
            contexto_interno.DX = contexto_espera.DX;
            contexto_interno.EAX = contexto_espera.EAX;
            contexto_interno.EBX = contexto_espera.EBX;
            contexto_interno.ECX = contexto_espera.ECX;
            contexto_interno.EDX = contexto_espera.EDX;
            contexto_interno.SI = contexto_espera.SI;
            contexto_interno.DI = contexto_espera.DI;
            log_trace(logger, "Se carga nuevo contexto de ejecucion");
            sem_post(&hay_proceso_ejecutando);

            break;
        
        case FALLO:
            log_error(logger_debug, "Kernel desconectado, terminando servidor DISPATCH");
            continuar_iterando = false;
            break;

        default:
            log_warning(logger_debug, "Llego algo que no es contexto por DISPATCH, codigo: %d", operacion);
            break;
        }
    }
}

void desalojar_proceso(op_code motivo_desalojo){
    pthread_mutex_lock(&mutex_detenerEjecucion);
    detener_ejecucion=true;
    pthread_mutex_unlock(&mutex_detenerEjecucion);
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    log_info(logger, "El proceso PID: %u es desalojado, motivo: %s", PID, codigo_operacion_string(motivo_desalojo));
}

void enviar_CE_con_1_arg(op_code motivo_desalojo, char* arg1)
{
    pthread_mutex_lock(&mutex_detenerEjecucion);
    detener_ejecucion=true;
    pthread_mutex_unlock(&mutex_detenerEjecucion);
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(arg1) + 1, arg1);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
};

void enviar_CE_con_2_arg(op_code motivo_desalojo, char* arg1, char* arg2)
{
    pthread_mutex_lock(&mutex_detenerEjecucion);
    detener_ejecucion=true;
    pthread_mutex_unlock(&mutex_detenerEjecucion);
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(arg1) + 1, arg1);
    agregar_a_paquete_string(paquete, strlen(arg2) + 1, arg2);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
};

void solicitar_IO_GEN_SLEEP(op_code motivo_desalojo, char* nombre_interfaz, uint32_t unidades_trabajo)
{
    pthread_mutex_lock(&mutex_detenerEjecucion);
    detener_ejecucion=true;
    pthread_mutex_unlock(&mutex_detenerEjecucion);
    t_paquete* paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto_interno);
    agregar_a_paquete_string(paquete, strlen(nombre_interfaz) + 1, nombre_interfaz);
    agregar_a_paquete_uint32(paquete, unidades_trabajo);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
}


void gestionar_conexion_interrupt()
{
    op_code operacion;
    bool continuar_iterando = true;
    uint32_t size;
    uint32_t desplazamiento;
    void* buffer;
    uint32_t PID_recibido;

    while (continuar_iterando)
    {
        desplazamiento=0;
        
        operacion = recibir_operacion(socket_cpu_kernel_interrupt);
              
        
        switch (operacion)
        {
        case MENSAJE:
            recibir_mensaje(socket_cpu_kernel_interrupt,logger_debug);
            break;

        case DESALOJO_POR_CONSOLA:
            buffer = recibir_buffer(&size, socket_cpu_kernel_interrupt);
            PID_recibido = leer_de_buffer_uint32(buffer, &desplazamiento);

             pthread_mutex_lock(&mutex_detenerEjecucion);
            if (!detener_ejecucion)
            {
                log_info(logger, "El usuario finaliza el proceso PID: %u por consola, proceso en ejecucion: %u", PID_recibido, PID);
                int_consola = true;
                pthread_mutex_unlock(&mutex_detenerEjecucion);
            }else{
                log_warning(logger_debug,"Llego una interrupcion por consola mientras no se estaba ejecutando");
                pthread_mutex_unlock(&mutex_detenerEjecucion);
            }
            free(buffer);

            break;

        case DESALOJO_POR_QUANTUM:
            buffer = recibir_buffer(&size, socket_cpu_kernel_interrupt);
            PID_recibido = leer_de_buffer_uint32(buffer, &desplazamiento);
            pthread_mutex_lock(&mutex_detenerEjecucion);
            if (!detener_ejecucion )
            {
                log_debug(logger, "El proceso PID: %u termino su quantum sera desalojado, proceso en ejecucion: %u", PID_recibido, PID);
                int_quantum = true;
                pthread_mutex_unlock(&mutex_detenerEjecucion);
            }else{
                log_warning(logger_debug,"Llego una interrupcion de quantum mientras no se estaba ejecutando");
                pthread_mutex_unlock(&mutex_detenerEjecucion);
            }
            free(buffer);
            break;

        case FALLO:
            log_error(logger, "Kernel desconectado, terminando servidor INTERRUPT");
            continuar_iterando = false;
            break;
       
        default:
            log_warning(logger_debug, "Llego algo que no era interrupcion por socket interrupt, op_code: %d", operacion);
            break;
        }
    }
    
}