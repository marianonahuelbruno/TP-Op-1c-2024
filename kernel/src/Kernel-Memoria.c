#include "../include/Kernel-Memoria.h"


void atender_conexion_MEMORIA_KERNEL()
{    
    //ENVIAR MENSAJE A MEMORIA
    enviar_mensaje("CONEXION CON KERNEL OK", socket_memoria_kernel);
    log_info(logger, "Handshake enviado: MEMORIA");

    uint32_t sizeTotal;
    void* buffer;
    op_code cod_op;

    bool continuarIterando=true;
    
    while (continuarIterando) {
        cod_op = recibir_operacion(socket_memoria_kernel);   ////se queda esperando en recv por ser bloqueante
        switch (cod_op) {
        case MENSAJE:
            recibir_mensaje(socket_memoria_kernel,logger_debug);
            break;
        case CARGA_EXITOSA_PROCESO:
            buffer= recibir_buffer(&sizeTotal,socket_memoria_kernel);                                    /// ABRO UN HILO POR CADA PROCESO QUE SE ENCOLA EN NEW ASI PUEDO SEGUIR ESCUCHANDO PROCESOS
            pthread_t hilo_de_Planificador_largo_plazo;                
            pthread_create(&hilo_de_Planificador_largo_plazo, NULL,(void*) carga_exitosa_en_memoria,buffer); 
            pthread_detach(hilo_de_Planificador_largo_plazo);            
            break;

        case ERROR_AL_CARGAR_EL_PROCESO:                
            uint32_t desplazamiento=0;
            buffer= recibir_buffer(&sizeTotal,socket_memoria_kernel);
            uint32_t PID = leer_de_buffer_uint32(buffer,&desplazamiento); 
            t_pcb *pcb= buscar_pcb_por_PID_en_lista(lista_new,PID,&semaforo_new);
            if(list_remove_element(lista_new,pcb)){
                log_error(logger_debug,"Error al cargar el proceso PID: %u en memoria. Eliminado de NEW",PID);
                free(pcb);
            }else{
                log_error(logger_debug,"Error al cargar el proceso PID: %u en memoria. No se pudo eliminar de NEW",PID);
            }
            free(buffer);                
            break;

        case FALLO:
            log_error(logger_debug, "el MODULO DE MEMORIA SE DESCONECTO.");
            continuarIterando=false;
            break;

        default:
            log_warning(logger_debug,"KERNEL recibio una operacion desconocida de Memoria. %d",cod_op);
            //continuarIterando=false;
            break;
            
            }
        }
}

void solicitud_de_creacion_proceso_a_memoria(uint32_t PID, char *leido){


    char** leido_array = string_split(leido, " ");  //SEPARO EL STRING LEIDO EN UN VECTOR DE STRING: 
    
//LE DOY VALOR A LAS VARIABLES A ENVIAR    

    //estructura: codigo operacion, pid, path_para_memoria
    op_code codigo_de_operacion=CREAR_PROCESO;
    char* path_para_memoria=leido_array[1];
    uint32_t tamanio=strlen(path_para_memoria)+1;                      //--------------LO GUARDO EN UN uint32_t DE 32 BITS PORQUE EL STRLEN DEVUELVE 64 BITS 

//PREAPARO EL STREAM DE DATOS, LOS SERIALIZO Y ENVIO

    t_paquete *paquete= crear_paquete (codigo_de_operacion);
    agregar_a_paquete_uint32(paquete,PID);
    agregar_a_paquete_string(paquete,tamanio,path_para_memoria);
    enviar_paquete(paquete,socket_memoria_kernel);              //--------------ESTA FUNCION SERIALIZA EL PAQUETE ANTES DE ENVIARLO --quedaria un void*= cod_op||SIZE TOTAL||size path|| path
    eliminar_paquete(paquete);

liberar_array_de_comando(leido_array,string_array_size(leido_array));
}

void carga_exitosa_en_memoria(void* buffer){  
    uint32_t desplazamiento=0;
    uint32_t PID=0 ; 

    if (buffer != NULL) {
    PID = leer_de_buffer_uint32(buffer, &desplazamiento);
 
        
        log_info(logger_debug,"CARGA EXITOSA DEL PROCESO: PID= %u ",PID);
        


        free(buffer);
    
    } else {
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }

    

              
    
     sem_wait(&control_multiprogramacion);///         SOLO AVANZO SI LA MULTIPROGRAMACION LO PERMITE    --------------------------------------------------------------
    
    if (barrera_activada)                                           /// ESTE IF-WHILE NO DEJA PASAR NINGUN PROCESO CUANDO DISMINUYO EL VALOR DE MULTIPROGRAMACION
    {
        while(barrera_activada){
            sem_post(&control_multiprogramacion);                       //esto lo pongo por el caso especial eliminar procesos algun ready luego bajar multiprogramacion
            //log_error(logger_debug,"Iterando while barrera");
            sem_wait(&control_multiprogramacion);
        }
    }

    //printf("Pase la barrera\n");
    
    

    if (detener_planificacion)                      /// Si la PLANIFICACION ESTA DETENIDA QUEDO BLOQEUADO EN WAIT
    {   
                
            pthread_mutex_lock(&mutex_cont_plp);
            cantidad_procesos_bloq_plp++;
            pthread_mutex_unlock(&mutex_cont_plp);
            //log_debug(logger_debug,"El valor del semaforo es: -%d",cantidad_procesos_bloq_plp);
            sem_wait(&semaforo_plp);
            while(barrera_activada || detener_planificacion){
                sem_post(&control_multiprogramacion);   
                sem_wait(&control_multiprogramacion);   //ESTE LO PONGO PARA QUE NO SE LIBEREN PROCESO EN ESTADO DETENIDO - DIMINUCION MULTIPROGRAMACION
            }
    }

    //printf("Pase la detencion\n");



    t_pcb* pcb_ready= buscar_pcb_por_PID_en_lista(lista_new,PID,&semaforo_new);

    if(pcb_ready==NULL){
        log_warning(logger_debug,"Proceso PID= %u ya no se encuentra en la lista NEW",PID);
        sem_post(&control_multiprogramacion);
    }else{
        pthread_mutex_lock(&semaforo_new);        
        if (list_remove_element(lista_new, pcb_ready)){
        
            ingresar_en_lista(pcb_ready, lista_ready, &semaforo_ready, &cantidad_procesos_en_algun_ready , READY); //loggeo el cambio de estado, loggeo el proceso si es cola ready/prioritario 
        }else{
        
            log_error(logger_debug,"Error al eliminar el elemento PID= u% de la lista NEW",PID);
        }
        pthread_mutex_unlock(&semaforo_new);
    }
}
//funcion buscar pcb por PID en lista pero con punteros
/*
t_pcb* buscar_pcb_por_PID_en_lista(t_list* lista, uint32_t pid_buscado){
	t_link_element* aux=lista->head;
    t_pcb* pcb_auxiliar;
 

    while (aux!=NULL)
    {
        pcb_auxiliar= (t_pcb*) aux->data; 
        log_trace(logger_debug,"Leyendo de lista el PCB con PID: %u",pcb_auxiliar->PID );
    if (pcb_auxiliar->PID==pid_buscado){
        log_info(logger_debug,"PCB encontrado. PID: %u",pcb_auxiliar->PID);
        return pcb_auxiliar;
    }
    aux=aux->next;
    }
    log_info(logger_debug,"PCB NO encontrado. PID: %u",pid_buscado);
	return NULL;
}
*/


t_pcb* buscar_pcb_por_PID_en_lista(t_list* lista, uint32_t pid_buscado,pthread_mutex_t* semaforo_mutex){
	
    t_pcb* pcb_auxiliar;
 
            pthread_mutex_lock(semaforo_mutex);
    		for(int32_t i=0; i<list_size(lista); i++){
			pcb_auxiliar = (t_pcb*)list_get(lista, i);
			//log_trace(logger_debug,"Leyendo de lista el PCB con PID: %u",pcb_auxiliar->PID );
			
            if (pcb_auxiliar->PID == pid_buscado){
                //log_info(logger_debug,"PCB encontrado. PID: %u",pcb_auxiliar->PID);
                pthread_mutex_unlock(semaforo_mutex);
                return pcb_auxiliar;
            }
            }
    pthread_mutex_unlock(semaforo_mutex);        
    //log_info(logger_debug,"PCB NO encontrado. PID: %u",pid_buscado);
	return NULL;

}


