
#include "../include/planificacion.h"

//////////////////   PLANIFICADOR LARGO PLAZO  /////////////////


void ingresar_en_lista(t_pcb* pcb, t_list* lista, pthread_mutex_t* semaforo_mutex, sem_t* semaforo_contador, t_estado estado_nuevo) {
	pthread_mutex_lock(semaforo_mutex);
    char* estado_nuevo_string=codigo_estado_string(estado_nuevo);

	if (pcb->estado != estado_nuevo){
		loggeo_de_cambio_estado(pcb->PID, pcb->estado, estado_nuevo);
	}
    
	pcb->estado = estado_nuevo;
	list_add(lista, pcb);

    sem_post(semaforo_contador);

	//log_info(logger, "Proceso PID:%u ingreso en %s", pcb->PID,estado_nuevo_string);

////////////////////////    LOGGEO OBLIGATORIO DE READY Y READY PRIORITARIO /////////////


	if(strcmp(estado_nuevo_string, "READY")==0){
		char* log_cola_ready = string_new();
		string_append(&log_cola_ready, "[");
		for(int32_t i=0; i<list_size(lista); i++){
			t_pcb* pcb_logueado = list_get(lista, i);
			char* string_pid = string_itoa(pcb_logueado->PID);
			string_append(&log_cola_ready, string_pid);
			free(string_pid);
			if(i!= (list_size(lista)-1)){
				string_append(&log_cola_ready, ", ");
			}
		}
		string_append(&log_cola_ready, "]");
		log_info(logger,"Lista de PID de procesos en estado READY (%s) : %s.\n",algoritmo_planificacion, log_cola_ready);
		free(log_cola_ready);
	}
 
    
    
    if(strcmp(estado_nuevo_string, "READY_PRIORITARIO")==0){
		char* log_cola_ready_prioritario = string_new();
		string_append(&log_cola_ready_prioritario, "[");
		for(int32_t i=0; i<list_size(lista); i++){
			t_pcb* pcb_logueado = list_get(lista, i);
			char* string_pid = string_itoa(pcb_logueado->PID);
			string_append(&log_cola_ready_prioritario, string_pid);
			free(string_pid);
			if(i!= (list_size(lista)-1)){
				string_append(&log_cola_ready_prioritario, ", ");
			}
		}
		string_append(&log_cola_ready_prioritario, "]");
		log_info(logger,"Lista de PID de procesos en estado READY Prioritario %s : %s\n",algoritmo_planificacion, log_cola_ready_prioritario);
		free(log_cola_ready_prioritario);
	}
    pthread_mutex_unlock(semaforo_mutex);



    if(strcmp(estado_nuevo_string, "READY_PRIORITARIO")==0 || strcmp(estado_nuevo_string, "READY")==0){


        if (!gestionando_dispatch && !ocupacion_cpu)            //aca tengo que controlar que no haya un hilo o enviando procesos o gestionando dispatch;
        {   
                int error;

                 error = pthread_cancel(hilo_CPU_dispatch);
                if (error != 0) {
                    fprintf(stderr, "Error cancelando el hilo: %s\n", strerror(error));
                }
    
                error = pthread_join(hilo_CPU_dispatch, NULL);
                if (error != 0) {
                    fprintf(stderr, "Error haciendo join al hilo: %s\n", strerror(error));
                }
    
                error = pthread_create(&hilo_CPU_dispatch, NULL, (void*) enviar_siguiente_proceso_a_ejecucion, NULL);
                if (error != 0) {
                    fprintf(stderr, "Error creando el hilo enviar_siguiente_proceso_a_ejecucion: %s\n", strerror(error));
                }

        }
    }

}


void loggeo_de_cambio_estado(uint32_t pid, t_estado viejo, t_estado nuevo){
    	log_info(logger, "PID: %u - Cambio de estado %s -> %s", pid, codigo_estado_string(viejo), codigo_estado_string(nuevo));
}





void cambiar_grado_multiprogramacion(void* nuevoValor) {                         // CON ESTA FUNCION AGREGO O DISMINUYO INSTANCIAS DEL SEMAFORO QUE GESTIONA LA MULTIPROGRAMCION 

        
        int32_t *nuevo_valor = (int32_t *)nuevoValor;
        int32_t val= grado_multiprogramacion- *nuevo_valor;
        
    
    if (*nuevo_valor > grado_multiprogramacion) {
        // Incrementar el semáforo
        for (int32_t i = 0; i < *nuevo_valor - grado_multiprogramacion ; i++) {
            //log_error(logger_debug,"Aumentando valor semaforo\n");
            sem_post(&control_multiprogramacion);                           //Suma instancias al semaforo
                               
            
        }
    } else if (*nuevo_valor < grado_multiprogramacion) {
             // Decrementar el semáforo
        barrera_activada=true;
        for (int32_t i = 0; i < val; i++) {
            //log_error(logger_debug,"Disminuyendo valor semaforo\n");
            sem_wait(&control_multiprogramacion);                           //Resta instancias al semaforo
                                
            
        }
        barrera_activada=false;
    }
    grado_multiprogramacion= *nuevo_valor;

    free(nuevo_valor);
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


        /////////////////////////////////////////////////////       PLANIFICADOR CORTO PLAZO               //////////////////////////////////////////////////////////////


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------






void gestionar_dispatch (){ 

    
    uint32_t desplazamiento;
    uint32_t size;

    bool continuarIterando=true;
    char* recurso_solicitado;

    while(continuarIterando){    

        gestionando_dispatch=false;             ///GESTIONANDO DISPATCH Y OCUPACION CPU SON PARA SINCRONIZAR LA ENTRADA DE NUEVOS PROCESOS
        
        //-----------------------------------------------------------------------
        
        cod_op_dispatch = recibir_operacion(socket_kernel_cpu_dispatch);
        
        //-----------------------------------------------------------------------
        
        ocupacion_cpu=false;
        gestionando_dispatch=true;    
        
        
        if ((strcmp(algoritmo_planificacion,"VRR")==0 ||strcmp(algoritmo_planificacion,"RR")==0 ) && temporizador!=NULL && cod_op_dispatch!= DESALOJO_POR_QUANTUM)
        {
            tiempo_recien_ejecutado= temporal_gettime(temporizador); //recupero el valor antes de eliminar el temporizador
            temporal_destroy(temporizador);
            temporizador=NULL;
            pthread_cancel(hilo_de_desalojo_por_quantum);
            pthread_join(hilo_de_desalojo_por_quantum,NULL);
        }else if(temporizador!=NULL){
            temporal_destroy(temporizador);
            temporizador=NULL;
        }

    ////////////////////////////////   EXTRAIGO DEL SOCKET LO COMUN A TODOS LOS PROCESOS //////////////////
        
        t_pcb *pcb_dispatch=malloc(sizeof(t_pcb));
        void* buffer = recibir_buffer(&size, socket_kernel_cpu_dispatch);

        desplazamiento = 0;

        if (detener_planificacion)                             /// Si la PLANIFICACION ESTA DETENIDA QUEDO BLOQEUADO EN WAIT
        {
            
            pthread_mutex_lock(&mutex_cont_pcp);
            cantidad_procesos_bloq_pcp++;
            pthread_mutex_unlock(&mutex_cont_pcp);
            //log_info(logger_debug,"Planificacion corto plazo detenida");
            sem_wait(&semaforo_pcp);
        }
       


        
        if (cod_op_dispatch==MENSAJE)
        {
            char* mensaje = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger_debug, "%s", mensaje);
            free(mensaje);
            
        }else{
            pcb_dispatch->PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            leer_de_buffer_CE(buffer, &desplazamiento, &pcb_dispatch->CE);

            if(strcmp(algoritmo_planificacion,"VRR")==0 ){
            pcb_dispatch->quantum_ejecutado=tiempo_recien_ejecutado+ backup_de_quantum_ejecutado;
            }else{
            pcb_dispatch->quantum_ejecutado=0;
            }
            pcb_dispatch->estado=EXEC;
            backup_de_quantum_ejecutado=0;      ////    RESETEO BACKUP DE QUANTUM   
            tiempo_recien_ejecutado=0;          ////    RESETEO EL VALOR QUE OBTIENE EL TIEMPO DEL CONTADOR
            pcb_actual_en_cpu=0;                ////Para indicar que actualmente no hay ningun proceso en ejecucion
        }
        
        


    ///////////////////////////////   EJECUTO SEGUN EL CODIGO DE OPERACION  ///////////////////////////

        char* nombre_interfaz;
        char* nombre_archivo;
        t_paquete* paquete;
        uint32_t cant_accesos;
        switch (cod_op_dispatch){
        case RETORNAR:
        //Este caso es para cuando elimino un proceso
        break;
        case MENSAJE:
        //lo dejo vacio para que pegue la vuelta
        break;
        
        case OUT_OF_MEMORY:
            log_info(logger, "Finaliza el proceso PID: %u Motivo: OUT_OF_MEMORY ", pcb_dispatch->PID);
            log_info(logger, "PID: %u - Cambio de estado EJECUCION-> EXIT", pcb_dispatch->PID);
            enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel); ///ELIMINO DE MEMORIA
            eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);                                           //ELIMINO DE LISTA DE RECURSOS ASIGNADOS  
            eliminar_proceso_de_lista_asignaciones_recurso(pcb_dispatch->PID);
            sem_post(&control_multiprogramacion);
            
                                                                         //AGREGO UNA INSTANCIA A CANTIDAD DE PROCESOS  
            //vengode_gestionarDispatch=true;
            enviar_siguiente_proceso_a_ejecucion();
            break;

        case DESALOJO_POR_WAIT:
            recurso_solicitado = leer_de_buffer_string(buffer, &desplazamiento);

            switch (wait_recursos(recurso_solicitado, pcb_dispatch)) {

                case 1:
                    log_info(logger, "PID: %u - Bloqueado por recurso: %s", pcb_dispatch->PID, recurso_solicitado);
                    //PCB QUEDO EN COLA DE ESPERA DEL RECURSO
                    //vengode_gestionarDispatch=true;
                    enviar_siguiente_proceso_a_ejecucion();	

                    break;
                case 2: 
                    log_info(logger, "PID: %u hace WAIT de recurso: %s. Enviando de nuevo a CPU", pcb_dispatch->PID, recurso_solicitado);
                    //WAIT REALIZADO, DEVOLVER EL PROCESO A EJECUCION
                        pthread_mutex_lock(&semaforo_ready_prioridad);
                        list_add_in_index(lista_ready_prioridad, 0, pcb_dispatch);
                        pthread_mutex_unlock(&semaforo_ready_prioridad);

                        sem_post(&cantidad_procesos_en_algun_ready);
                        //log_info(logger_debug, "Se gestiono el recurso, y se envio nuevamente a CPU a ejecutar el proceso PID:  %u", pcb_dispatch->PID);
                        //vengode_gestionarDispatch=true;
                        enviar_siguiente_proceso_a_ejecucion(pcb_dispatch);

                    break;
                case -1:  
                    log_info(logger, "Finaliza el proceso PID: %u Motivo: INVALID_RESOURCE: %s", pcb_dispatch->PID, recurso_solicitado);
                    log_info(logger, "PID: %u - Cambio de estado EJECUCION-> EXIT", pcb_dispatch->PID);
                    //RECURSO NO ENCONTRADO, ENVIAR PROCESO A EXIT
                    enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
                    eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);
                    eliminar_proceso_de_lista_asignaciones_recurso(pcb_dispatch->PID);
                    sem_post(&control_multiprogramacion); 

                    //vengode_gestionarDispatch=true;
                    enviar_siguiente_proceso_a_ejecucion();
                    break;
                default:
                    log_error(logger_debug,"La funcion wait devolvio error");
                    break;
                }
            break; 

        case DESALOJO_POR_SIGNAL:
            recurso_solicitado = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger, "PID: %u solicita un SIGNAL del recurso: %s", pcb_dispatch->PID, recurso_solicitado );
            
            switch(signal_recursos (recurso_solicitado,pcb_dispatch->PID)){
                case 1:
                    //log_info(logger, "PID: %u hace SIGNAL a un recurso: %s exitosamente", pcb_dispatch->PID, recurso_solicitado);
                    //SIGNAL EXITOSO, DEVUELVO EL PROCESO A EJECUCION
                    pthread_mutex_lock(&semaforo_ready_prioridad);
                    list_add_in_index(lista_ready_prioridad, 0, pcb_dispatch);
                    pthread_mutex_unlock(&semaforo_ready_prioridad);

                    sem_post(&cantidad_procesos_en_algun_ready);
                    log_info(logger_debug, "Se gestiono el recurso, y se envio nuevamente a CPU a ejecutar el proceso PID:  %u", pcb_dispatch->PID);
                    
                    //vengode_gestionarDispatch=true;
                    enviar_siguiente_proceso_a_ejecucion(pcb_dispatch);

                    break;
                case -1:
                    log_info(logger, "Finaliza el proceso PID: %u Motivo: INVALID_RESOURCE: %s", pcb_dispatch->PID, recurso_solicitado);
                    log_info(logger, "PID: %u - Cambio de estado EJECUCION-> EXIT", pcb_dispatch->PID);
                    //RECURSO NO ENCONTRADO, ENVIAR PROCESO A EXIT
                    enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
                    eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);
                    eliminar_proceso_de_lista_asignaciones_recurso(pcb_dispatch->PID);
                    sem_post(&control_multiprogramacion);

                    //vengode_gestionarDispatch=true;
                    enviar_siguiente_proceso_a_ejecucion();
                    break;
                case -2:
                    log_info(logger, "Finaliza el proceso PID: %u Motivo: RECURSO NO ASIGNADO: %s", pcb_dispatch->PID, recurso_solicitado);
                    log_info(logger, "PID: %u - Cambio de estado EJECUCION-> EXIT", pcb_dispatch->PID);
                    //SIGANL FALLIDO, NO TIENE ASIGNAOD EL RECURSO LIBERADO
                    enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
                    eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);
                    eliminar_proceso_de_lista_asignaciones_recurso(pcb_dispatch->PID);
                    sem_post(&control_multiprogramacion);

                    //vengode_gestionarDispatch=true;                    
                    enviar_siguiente_proceso_a_ejecucion();
            }
            
        break;

        case DESALOJO_POR_QUANTUM:
            log_info(logger, "PID: %u - Desalojado por fin de Quantum", pcb_dispatch->PID);
            //RESETEO EL CONTADOR Y LO PONGO NUEVAMENTE EN READY
            pcb_dispatch->quantum_ejecutado=0;
            ingresar_en_lista(pcb_dispatch, lista_ready, &semaforo_ready, &cantidad_procesos_en_algun_ready , READY);
            
            //vengode_gestionarDispatch=true;
            enviar_siguiente_proceso_a_ejecucion();
            break;

        case DESALOJO_POR_FIN_PROCESO:
            log_info(logger, "Finaliza el proceso PID: %u Motivo: SUCCESS", pcb_dispatch->PID);
            log_info(logger, "PID: %u - Cambio de estado EJECUCION-> EXIT", pcb_dispatch->PID);
            //ENVIO PID A MEMORIA PARA QUE ELIMINE EL PROCESO
            enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
            eliminar_proceso_de_lista_recursos (pcb_dispatch->PID);
            sem_post(&control_multiprogramacion);

            //vengode_gestionarDispatch=true;
            enviar_siguiente_proceso_a_ejecucion();
            break;

        case DESALOJO_POR_CONSOLA:
            
            //vengode_gestionarDispatch=true;
            enviar_siguiente_proceso_a_ejecucion();
            break;

        case DESALOJO_POR_IO_GEN_SLEEP:
            log_debug(logger_debug, "Gestionando IO_GEN_SLEEP");
            nombre_interfaz = leer_de_buffer_string(buffer, &desplazamiento);
            uint32_t unidades_trabajo = leer_de_buffer_uint32(buffer, &desplazamiento);

            paquete = crear_paquete(cod_op_dispatch);
            agregar_a_paquete_uint32(paquete, pcb_dispatch->PID);
            agregar_a_paquete_uint32(paquete, unidades_trabajo);
            gestionar_solicitud_IO(pcb_dispatch, nombre_interfaz, cod_op_dispatch, paquete);

            //vengode_gestionarDispatch=true;
            enviar_siguiente_proceso_a_ejecucion();
            free(nombre_interfaz);
            break;

        case DESALOJO_POR_IO_STDIN:
        case DESALOJO_POR_IO_STDOUT:
            log_debug(logger_debug, "Gestionando IO_STD");
            paquete = crear_paquete(cod_op_dispatch);
            agregar_a_paquete_uint32(paquete, pcb_dispatch->PID);
            nombre_interfaz = leer_de_buffer_string(buffer, &desplazamiento);
            agregar_a_paquete_uint32(paquete, leer_de_buffer_uint32(buffer, &desplazamiento));// tamanio_total

            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);
            agregar_a_paquete_uint32(paquete, cant_accesos);
            for (int i = 0; i < cant_accesos; i++)
            {
                agregar_a_paquete_uint32(paquete, leer_de_buffer_uint32(buffer, &desplazamiento));// dir_fisica
                agregar_a_paquete_uint32(paquete, leer_de_buffer_uint32(buffer, &desplazamiento)); //tamanio_acceso
            }

            gestionar_solicitud_IO(pcb_dispatch, nombre_interfaz, cod_op_dispatch, paquete);

            //vengode_gestionarDispatch=true;
            enviar_siguiente_proceso_a_ejecucion();
            free(nombre_interfaz);
            break;

        case DESALOJO_POR_IO_FS_CREATE:
        case DESALOJO_POR_IO_FS_DELETE:
            log_debug(logger_debug, "Gestionando IO_FS CREATE/DELETE");
            nombre_interfaz = leer_de_buffer_string(buffer, &desplazamiento);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);

            paquete = crear_paquete(cod_op_dispatch);
            agregar_a_paquete_uint32(paquete, pcb_dispatch->PID);
            agregar_a_paquete_string(paquete, string_length(nombre_archivo)+1, nombre_archivo);

            gestionar_solicitud_IO(pcb_dispatch, nombre_interfaz, cod_op_dispatch, paquete);

            //vengode_gestionarDispatch=true;
            enviar_siguiente_proceso_a_ejecucion();
            free(nombre_interfaz);
            free(nombre_archivo);
            break;
            
        case DESALOJO_POR_IO_FS_TRUNCATE:
            log_debug(logger_debug, "Gestionando IO_FS TRUNCATE");
            nombre_interfaz = leer_de_buffer_string(buffer, &desplazamiento);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            uint32_t nuevo_tamanio = leer_de_buffer_uint32(buffer, &desplazamiento);

            paquete = crear_paquete(cod_op_dispatch);
            agregar_a_paquete_uint32(paquete, pcb_dispatch->PID);
            agregar_a_paquete_string(paquete, string_length(nombre_archivo)+1, nombre_archivo);
            agregar_a_paquete_uint32(paquete, nuevo_tamanio);

            gestionar_solicitud_IO(pcb_dispatch, nombre_interfaz, cod_op_dispatch, paquete);

            //vengode_gestionarDispatch=true;
            enviar_siguiente_proceso_a_ejecucion();
            free(nombre_interfaz);
            free(nombre_archivo);
            break;

        case DESALOJO_POR_IO_FS_WRITE:
        case DESALOJO_POR_IO_FS_READ:
            log_debug(logger_debug, "Gestionando IO_FS READ/WRITE");
            nombre_interfaz = leer_de_buffer_string(buffer, &desplazamiento);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            
            paquete = crear_paquete(cod_op_dispatch);
            agregar_a_paquete_uint32(paquete, pcb_dispatch->PID);
            agregar_a_paquete_string(paquete, string_length(nombre_archivo)+1, nombre_archivo);
            agregar_a_paquete_uint32(paquete, leer_de_buffer_uint32(buffer, &desplazamiento));// tamanio_total
            agregar_a_paquete_uint32(paquete, leer_de_buffer_uint32(buffer, &desplazamiento));// puntero
            uint32_t cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);
            agregar_a_paquete_uint32(paquete, cant_accesos);

            int32_t i=0;
            while (i<cant_accesos)
            {
                agregar_a_paquete_uint32(paquete, leer_de_buffer_uint32(buffer, &desplazamiento));// dir_fisica
                agregar_a_paquete_uint32(paquete, leer_de_buffer_uint32(buffer, &desplazamiento));// tam_acceso
                ++i;
            }

            gestionar_solicitud_IO(pcb_dispatch, nombre_interfaz, cod_op_dispatch, paquete);

            //vengode_gestionarDispatch=true;
            enviar_siguiente_proceso_a_ejecucion();
            free(nombre_interfaz);
            free(nombre_archivo);
            break;
            
        case FALLO:
            log_error(logger_debug,"El modulo CPU se desconecto");
            continuarIterando=false;
            break;

        default:
            log_warning(logger_debug,"Operacion desconocida para Kernel al recibir de socket CPU-Dispatch.");
            break;
        }

        free(pcb_dispatch);
        free(buffer);
        
    }
}

void gestionar_solicitud_IO(t_pcb* pcb_dispatch, char* nombre_interfaz, op_code cod_op_dispatch, t_paquete* paquete)
{
    if(validar_conexion_interfaz_y_operacion (nombre_interfaz, cod_op_dispatch)){
        log_info(logger_debug, "PID: %u envia peticion a interfaz %s", pcb_dispatch->PID, nombre_interfaz);

        agregar_a_cola_interfaz(nombre_interfaz,pcb_dispatch->PID,paquete);   /// lo agrego a la cola y voy enviando a medida que tengo disponible la interfaz

        if(strcmp(algoritmo_planificacion,"VRR")==0 ) /// -------------------BLOQUEO EL PROCESO SEGUN PLANIFICADOR
        {   
            if(pcb_dispatch->quantum_ejecutado<quantum){
                log_info(logger,"PID: %u bloqueado en prioridad esperando uso interfaz: %s",pcb_dispatch->PID,nombre_interfaz);
                ingresar_en_lista(pcb_dispatch, lista_bloqueado_prioritario , &semaforo_bloqueado_prioridad, &cantidad_procesos_bloqueados , BLOCKED_PRIORITARIO);
            }else{
                log_warning(logger,"PID: %u bloqueado esperando uso interfaz: %s (ademas de solicitar interfaz se acabo su quantum).",pcb_dispatch->PID,nombre_interfaz);
               pcb_dispatch->quantum_ejecutado=0;
                ingresar_en_lista(pcb_dispatch, lista_bloqueado , &semaforo_bloqueado, &cantidad_procesos_bloqueados , BLOCKED);
            }
              
        }else
        {
            ingresar_en_lista(pcb_dispatch, lista_bloqueado, &semaforo_bloqueado, &cantidad_procesos_bloqueados , BLOCKED);  
            log_info(logger,"PID: %u bloqueado esperando uso interfaz: %s",pcb_dispatch->PID,nombre_interfaz);  

        }
    }else{
        enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pcb_dispatch->PID,socket_memoria_kernel);
        eliminar_proceso_de_lista_recursos(pcb_dispatch->PID);
        sem_post(&control_multiprogramacion);
        eliminar_paquete(paquete);
    }
}


void enviar_siguiente_proceso_a_ejecucion ()
{
    // SI NO HAY NINGUN PROCESO EN READY.... ESPERO QUE SE ENCOLE ALGUNO EN READY O READY PRIORIDAD
    sem_wait(&cantidad_procesos_en_algun_ready);
    ocupacion_cpu=true;

    // Si la PLANIFICACION ESTA DETENIDA QUEDO BLOQEUADO EN WAIT
    if (detener_planificacion)
    {
        pthread_mutex_lock(&mutex_cont_pcp);
        cantidad_procesos_bloq_pcp++;
        pthread_mutex_unlock(&mutex_cont_pcp);
        sem_wait(&semaforo_pcp);
    }

    t_pcb* pcb_a_ejecutar;

    if (list_size(lista_ready_prioridad)>0)
    {   
        pthread_mutex_lock(&semaforo_ready_prioridad);
        pcb_a_ejecutar = (t_pcb*) list_remove(lista_ready_prioridad, 0);
        pthread_mutex_unlock(&semaforo_ready_prioridad);
    }
    else{
        pthread_mutex_lock(&semaforo_ready);
        pcb_a_ejecutar = (t_pcb*)  list_remove(lista_ready, 0);
        pthread_mutex_unlock(&semaforo_ready);
    
    }
    
    if(strcmp(algoritmo_planificacion,"VRR")==0 || strcmp(algoritmo_planificacion,"RR")==0){       //CREO HILO DE DESALOJO SI CORRESPONDIERA
        t_pcb *pcb_interrupt = malloc(sizeof(t_pcb));                                                    //LA UNICA DIFERENCIA ENTRE EL FIFO Y EL RR ES EL DESALOJO POR QUANTUM
        if (pcb_interrupt== NULL) {
            perror("malloc");
            return;
        }
        backup_de_quantum_ejecutado= pcb_a_ejecutar->quantum_ejecutado;
        pcb_actual_en_cpu=pcb_a_ejecutar->PID;

        if(pcb_a_ejecutar->quantum_ejecutado<quantum)
        {
            *pcb_interrupt = *pcb_a_ejecutar;

            log_info(logger,"El proceso con PID:%u cambio su estado de READY -> EJECUCION",pcb_a_ejecutar->PID);
            log_debug(logger, "Se mando a CPU para ejecutar el proceso PID:  %u, planificado por '%s'\n ", pcb_a_ejecutar->PID,algoritmo_planificacion);
 

            pthread_create(&hilo_de_desalojo_por_quantum, NULL,(void*) interruptor_de_QUANTUM, pcb_interrupt); 
            pthread_detach(hilo_de_desalojo_por_quantum);
            
            enviar_CE(socket_kernel_cpu_dispatch, pcb_a_ejecutar->PID,pcb_a_ejecutar->CE);            
 
        }else
        {
            log_warning(logger_debug,"El calculo de quantum del proceso PID: %u dio un numero negativo: %ld. Agregado a lista READY",pcb_a_ejecutar->PID, quantum - pcb_a_ejecutar->quantum_ejecutado);
            ingresar_en_lista(pcb_a_ejecutar,lista_ready,&semaforo_ready,&cantidad_procesos_en_algun_ready, READY);
            enviar_siguiente_proceso_a_ejecucion ();
        }
    
    }
    else
    {
        pcb_actual_en_cpu = pcb_a_ejecutar->PID;
        log_info(logger,"El proceso con PID:%u cambio su estado de READY -> EJECUCION",pcb_a_ejecutar->PID);
        log_debug(logger, "Se mando a CPU para ejecutar el proceso PID:  %u, planificado por '%s'\n ", pcb_a_ejecutar->PID,algoritmo_planificacion);
        enviar_CE(socket_kernel_cpu_dispatch, pcb_a_ejecutar->PID, pcb_a_ejecutar->CE);  
    }
        
    free(pcb_a_ejecutar);
    gestionar_dispatch ();
/*
    if(//vengode_gestionarDispatch){
        //vengode_gestionarDispatch=false;
    }else{
        //vengode_gestionarDispatch=false;
        gestionar_dispatch ();
    }*/
}



void interruptor_de_QUANTUM(void* pcb)
{   
    t_pcb* pcb_interrupt= (t_pcb*)pcb;
    int64_t quantum_ejecucion= quantum - pcb_interrupt->quantum_ejecutado;
    
    log_trace(logger_debug,"El tiempo a ejecutar registrado es %ld",quantum_ejecucion);
    
    temporizador= temporal_create();

    usleep(quantum_ejecucion*1000);
 


    enviar_instruccion_con_PID_por_socket(DESALOJO_POR_QUANTUM,pcb_interrupt->PID,socket_kernel_cpu_interrupt);
    log_trace(logger_debug,"Interrupcion de quantum enviado");
    free(pcb);

}





void enviar_nuevamente_proceso_a_ejecucion(t_pcb* pcb_a_reenviar){                         //ESTA FUNCION ES PARA CUANDO SE SOLICITA UN RECURSO Y PUEDE SEGUIR EJECUTANDO

    ocupacion_cpu=true;
    t_pcb *pcb_interrupt = malloc(sizeof(t_pcb));                                                    //LA UNICA DIFERENCIA ENTRE EL FIFO Y EL RR ES EL DESALOJO POR QUANTUM
    if (pcb_interrupt== NULL) {
        perror("malloc");
        return;
    }


    *pcb_interrupt = *pcb_a_reenviar;
    pthread_t hilo_de_desalojo_por_quantum;                
    pthread_create(&hilo_de_desalojo_por_quantum, NULL,(void*) interruptor_de_QUANTUM, pcb_interrupt); 
    pthread_detach(hilo_de_desalojo_por_quantum);
    

    enviar_CE(socket_kernel_cpu_dispatch, pcb_a_reenviar->PID,pcb_a_reenviar->CE);     
    log_info(logger_debug, "Se gestiono el recurso, y se envio nuevamente a CPU a ejecutar el proceso PID:  %u\n", pcb_a_reenviar->PID);
    
    gestionar_dispatch ();
    
}


