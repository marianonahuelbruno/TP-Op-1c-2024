#include "../include/atender_operacion.h"

void atender_instruccion_validada(char* leido)
{
    char** array_de_comando= string_split (leido, " ");

    if(strcmp(array_de_comando[0],"EJECUTAR_SCRIPT")==0)//---------------------------------/////////////
    {
        leer_path_comandos(array_de_comando[1]);
        

    }else if (strcmp(array_de_comando[0],"INICIAR_PROCESO")==0)//---------------------------------/////////////
    {   
        log_trace(logger_debug,"Enviando a memoria el comando: %s %s", array_de_comando[0],array_de_comando[1]);
        iniciar_proceso(leido);        


    }else if (strcmp(array_de_comando[0],"FINALIZAR_PROCESO")==0)//---------------------------------/////////////
    {
        uint32_t pid_a_finalizar= atoi (array_de_comando[1]);                          
        finalizar_proceso_con_pid(pid_a_finalizar);
                

    }else if (strcmp(array_de_comando[0],"DETENER_PLANIFICACION")==0)//---------------------------------/////////////
        
    {   
        
        if (!detener_planificacion)
        {
            log_info(logger_debug,"Planificacion detenida");
        }
        detener_planificacion=true;
        
        
    }else if (strcmp(array_de_comando[0],"INICIAR_PLANIFICACION")==0)//---------------------------------/////////////
    {   
        if (detener_planificacion)
        {   
            detener_planificacion=false;          
            //log_error(logger_debug,"El valor del contador es: %d", cantidad_procesos_bloq_plp);
            while (cantidad_procesos_bloq_plp>0)
            {
                sem_post(&semaforo_plp);
                pthread_mutex_lock(&mutex_cont_plp);
                cantidad_procesos_bloq_plp--;
                pthread_mutex_unlock(&mutex_cont_plp);
                //log_error(logger_debug,"El valor del semaforo (W) es: %d",cantidad_procesos_bloq_plp);
            }

            while (cantidad_procesos_bloq_pcp>0)
            {
                sem_post(&semaforo_pcp);
                pthread_mutex_lock(&mutex_cont_pcp);
                cantidad_procesos_bloq_pcp--;
                pthread_mutex_unlock(&mutex_cont_pcp);
                //log_error(logger_debug,"El valor del semaforo (W) es: %d",cantidad_procesos_bloq_plp);
            }

            cantidad_procesos_bloq_plp=0;
            cantidad_procesos_bloq_pcp=0;

        }
        
        
        

    }else if (strcmp(array_de_comando[0],"MULTIPROGRAMACION")==0)//---------------------------------/////////////
    {   
        int32_t *valor = malloc(sizeof(uint32_t));                                                    //LA UNICA DIFERENCIA ENTRE EL FIFO Y EL RR ES EL DESALOJO POR QUANTUM
        if (valor== NULL) {
            perror("malloc");
            return;
        }
        *valor= atoi (array_de_comando[1]);


        pthread_t hilo_cambio_multiprogramacion;
        pthread_create(&hilo_cambio_multiprogramacion,NULL,(void*)cambiar_grado_multiprogramacion,valor);
        pthread_detach(hilo_cambio_multiprogramacion);
           
        

    }else if (strcmp(array_de_comando[0],"PROCESO_ESTADO")==0)//---------------------------------/////////////
    {   if (pcb_actual_en_cpu!=0)
        {
            log_info(logger,"PID de proceso en estado EJECUCION : %u",pcb_actual_en_cpu);
        }else{
            log_info(logger,"PID de proceso en estado EJECUCION : vacio");
        }
    
        imprimir_listas_de_estados(lista_new, "NEW");
        imprimir_listas_de_estados(lista_ready,"READY");
        imprimir_listas_de_estados(lista_ready_prioridad,"READY_PRIORITARIO");
        imprimir_listas_de_estados(lista_bloqueado,"BLOCKED");                                  //en bloqueados tambien imprimo los que estan esperando recursos
        imprimir_listas_de_estados(lista_bloqueado_prioritario,"BLOCKED_PRIORITARIO");
         
  /*  
    t_recurso* auxiliar = lista_de_recursos;

        
        while(auxiliar!=NULL){
        printf("\n\nImprimiendo lista asignaciones de recurso %s\n",auxiliar->nombre_recurso);
        if(list_size(auxiliar->lista_de_asignaciones)>0){
            
            printf("Cantidad de procesos en espera= %d\n",list_size(auxiliar->lista_de_espera));
        
            for(int32_t i=0; i<list_size(auxiliar->lista_de_asignaciones); i++){
                uint32_t *pid_auxiliar = (uint32_t*)list_get(auxiliar->lista_de_asignaciones, i);   //Busco el PID en la lista de instancias asignadas a procesos
                
                if (pid_auxiliar==NULL)
                {
                    log_error(logger_debug,"Pid es igual a null");
                
                }
                    
                    printf(" %u\n",*pid_auxiliar);

        }
            
        auxiliar = auxiliar->siguiente_recurso;
       
    }

}
 */       
    }

 liberar_array_de_comando(array_de_comando,string_array_size(array_de_comando));

}


void iniciar_proceso(char*leido){

    t_pcb *new_pcb= malloc(sizeof(t_pcb));
    new_pcb->PID=asignar_pid();
    new_pcb->estado=NEW; 
    new_pcb->quantum_ejecutado=0;
    new_pcb->CE.PC=0;
    new_pcb->CE.AX=0;
    new_pcb->CE.BX=0;
    new_pcb->CE.CX=0;
    new_pcb->CE.DX=0;
    new_pcb->CE.EAX=0;
    new_pcb->CE.EBX=0;
    new_pcb->CE.ECX=0;
    new_pcb->CE.EDX=0;
    new_pcb->CE.SI=0;
    new_pcb->CE.DI=0;

    ingresar_en_lista(new_pcb, lista_new, &semaforo_new, &cantidad_procesos_new , NEW); //loggeo el cambio de estado, loggeo el proceso si es cola ready/prioritario 
    log_info(logger, "Se crea el proceso con PID: %u en NEW",new_pcb->PID);
    solicitud_de_creacion_proceso_a_memoria(new_pcb->PID,leido);                // ENVIO A CARGAR EL PROGRAMA A EJECUTAR, YA QUE SUPUESTAMENTE ARRANCA CON UNA PAGINA VACIA
                                                                             // EL SEMAFORO DE CONTROL DE MULTIPROGRAMACION LO COLOCO ANTES DEL PASO A LISTA READY 
}



void finalizar_proceso_con_pid(uint32_t pid_a_finalizar){

    
    bool encontrado=false;
        if (pid_a_finalizar==pcb_actual_en_cpu)
        {       enviar_instruccion_con_PID_por_socket(DESALOJO_POR_CONSOLA,pid_a_finalizar,socket_kernel_cpu_interrupt);
                cod_op_dispatch=RETORNAR;
                pcb_actual_en_cpu=0;
                encontrado=true;
                log_info(logger, "Finaliza el proceso PID: %u Motivo: INTERRUPTED_BY_USER", pid_a_finalizar);
                log_info(logger, "PID: %u - Cambio de estado EJECUCION -> EXIT", pid_a_finalizar);               

                if (list_size(lista_new)==0 && !ocupacion_cpu )       //aca tengo que controlar que no haya un hilo o enviando procesos o gestionando dispatch;
                {   
                        if (pthread_cancel(hilo_CPU_dispatch) != 0) {
                        perror("Error cancelando el hilo");
                    }
            
                    if (pthread_join(hilo_CPU_dispatch, NULL) != 0) {
                        perror("Error haciendo join al hilo");
                    }
            
                    if (pthread_create(&hilo_CPU_dispatch, NULL,(void*) enviar_siguiente_proceso_a_ejecucion, NULL) != 0) {
                        perror("Error creando el hilo enviar_siguiente_proceso_a_ejecucion");
                    }

                }


        }


        t_pcb* pcb_a_eliminar=buscar_pcb_por_PID_en_lista(lista_ready,pid_a_finalizar,&semaforo_ready);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
                if(pcb_a_eliminar!=NULL){
                    pthread_mutex_lock(&semaforo_ready);
                    encontrado=true;
                    if(list_remove_element(lista_ready,pcb_a_eliminar)){
                        log_info(logger, "Finaliza el proceso PID: %u Motivo: INTERRUPTED_BY_USER", pid_a_finalizar);
                        log_info(logger, "PID: %u - Cambio de estado READY -> EXIT", pid_a_finalizar); 
                    }else{
                        log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista READY pero no se pudo eliminar.",pid_a_finalizar);
                    }
                    sem_wait(&cantidad_procesos_en_algun_ready);
                    pthread_mutex_unlock(&semaforo_ready);
                }

        pcb_a_eliminar=buscar_pcb_por_PID_en_lista(lista_ready_prioridad,pid_a_finalizar,&semaforo_ready_prioridad);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
                if(pcb_a_eliminar!=NULL){
                    pthread_mutex_lock(&semaforo_ready_prioridad);
                    encontrado=true;
                    if(list_remove_element(lista_ready_prioridad,pcb_a_eliminar)){
                        log_info(logger, "Finaliza el proceso PID: %u Motivo: INTERRUPTED_BY_USER", pid_a_finalizar);
                        log_info(logger, "PID: %u - Cambio de estado READY_PRIORIDAD -> EXIT", pid_a_finalizar);                         
                    }else{
                        log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista READY PRIORITARIO pero no se pudo eliminar.",pid_a_finalizar);
                    }
                    sem_wait(&cantidad_procesos_en_algun_ready);
                    pthread_mutex_unlock(&semaforo_ready_prioridad);
                }

        //printf("LLegue a la mitad\n");
        pcb_a_eliminar=buscar_pcb_por_PID_en_lista(lista_bloqueado,pid_a_finalizar,&semaforo_bloqueado);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
                if(pcb_a_eliminar!=NULL){
                    pthread_mutex_lock(&semaforo_bloqueado);
                    encontrado=true;
                    if(list_remove_element(lista_bloqueado,pcb_a_eliminar)){
                        log_info(logger, "Finaliza el proceso PID: %u Motivo: INTERRUPTED_BY_USER", pid_a_finalizar);
                        log_info(logger, "PID: %u - Cambio de estado BLOQUEADO -> EXIT", pid_a_finalizar);                         
                        
                    }else{
                        log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista BLOCKED pero no se pudo eliminar.",pid_a_finalizar);
                    }
                    pthread_mutex_unlock(&semaforo_bloqueado);
                }

        pcb_a_eliminar=buscar_pcb_por_PID_en_lista(lista_bloqueado_prioritario,pid_a_finalizar,&semaforo_bloqueado_prioridad);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
                if(pcb_a_eliminar!=NULL){
                    pthread_mutex_lock(&semaforo_bloqueado_prioridad);    
                    encontrado=true;
                    if(list_remove_element(lista_bloqueado_prioritario,pcb_a_eliminar)){
                        log_info(logger, "Finaliza el proceso PID: %u Motivo: INTERRUPTED_BY_USER", pid_a_finalizar);
                        log_info(logger, "PID: %u - Cambio de estado BLOQUEADO_PRIORITARIO -> EXIT", pid_a_finalizar); 
                    }else{
                        log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista BLOCKED PRIORITARIO pero no se pudo eliminar.",pid_a_finalizar);
                    }
                    pthread_mutex_unlock(&semaforo_bloqueado_prioridad);
                }


        if (encontrado)
        {
            sem_post(&control_multiprogramacion);
        }

        pcb_a_eliminar=buscar_pcb_por_PID_en_lista(lista_new,pid_a_finalizar,&semaforo_new);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
                if(pcb_a_eliminar!=NULL){
                    pthread_mutex_lock(&semaforo_new);
                    encontrado=true;
                    if(list_remove_element(lista_new,pcb_a_eliminar)){
                        log_info(logger, "Finaliza el proceso PID: %u Motivo: INTERRUPTED_BY_USER", pid_a_finalizar);
                        log_info(logger, "PID: %u - Cambio de estado NEW-> EXIT", pid_a_finalizar); 
                    }else{
                        log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista NEW pero no se pudo eliminar.",pid_a_finalizar);
                    }
                    pthread_mutex_unlock(&semaforo_new);
                }

        
        if(eliminar_proceso_de_lista_asignaciones_recurso(pid_a_finalizar)){
            encontrado=true;
        }
        
        if(eliminar_proceso_de_lista_recursos (pid_a_finalizar)){
            encontrado=true;
        }


        

        //printf("llegue al final\n");
        if (!encontrado)
        {
            log_error(logger_debug,"Proceso con PID: %u que se solicito finalizar NO existe.",pid_a_finalizar);
        }
        else{

            enviar_instruccion_con_PID_por_socket(ELIMINAR_PROCESO,pid_a_finalizar,socket_memoria_kernel);

        }



}






void imprimir_listas_de_estados(t_list* lista,char* estado){
    t_link_element* punteroLista=lista->head;
    char* log_lista = string_new();
    t_pcb* pcb;
	string_append(&log_lista, "[");
        
        while(punteroLista!=NULL)
		{   
            pcb = (t_pcb*) punteroLista->data;
			char* string_pid = string_itoa(pcb->PID);
			string_append(&log_lista, string_pid);
			free(string_pid);
			if(punteroLista->next!=NULL){
				string_append(&log_lista, ", ");
			}
            punteroLista=punteroLista->next;
		}


        if (strcmp(estado,"BLOCKED")==0)                                    ///EN EL CASO BLOCKED IMPRIMO TAMBIEN LOS PROCESOS QUE ESTAN ESPERANDO RECURSOS
        {bool primer_ingreso=true;   
            t_recurso* auxiliar = lista_de_recursos;
            
            while (auxiliar!=NULL)
            {
                if(list_size(auxiliar->lista_de_espera)!=0)
                {   
                    if(!primer_ingreso){
                    string_append(&log_lista, ", ");
                    }
                    primer_ingreso=false;

                    punteroLista=auxiliar->lista_de_espera->head;
                    while (punteroLista!=NULL)
                    {
                        pcb = (t_pcb*) punteroLista->data;
                        char* string_pid = string_itoa(pcb->PID);
			            string_append(&log_lista, string_pid);
			            free(string_pid);
			            if(punteroLista->next!=NULL){
			            	string_append(&log_lista, ", ");
			            }
                        punteroLista =punteroLista->next;
		            }   
                }
                auxiliar=auxiliar->siguiente_recurso;
            }
                
        }
            
        
        
		string_append(&log_lista, "]");
		log_info(logger,"Lista de PID de procesos en estado %s : %s",estado, log_lista);
		

        free(log_lista);


}



void leer_path_comandos(char* path){
    char* path_completo = string_new();
    string_append(&path_completo,path_de_comandos_base);
    string_append(&path_completo, path);

    
    FILE* archivo =  fopen(path_completo, "r");
    
    free(path_completo);

    if (archivo == NULL) {
        log_error(logger_debug, "No se pudo abrir archivo de comandos");
        return;    
    }
    log_debug(logger_debug,"Leyendo archivo de comandos");

    char linea[60];
    memset(linea, 0, 60);
    while (fgets(linea, 60, archivo) != NULL)
    {
        
        if (validacion_de_ingreso_por_consola(linea)) 
        {   
            size_t len = strlen(linea);
            if (len > 0 && linea[len - 1] == '\n') {
            linea[len - 1] = '\0';
        }
        
            atender_instruccion_validada(linea);
            

        }else{
            log_error(logger_debug, "El archivo de pseudocodigo tiene errores/comandos invalidas");
            fclose(archivo);
            return;
            
        }
        
    }

    fclose(archivo);
    log_info(logger_debug, "Archivo de comandos leido");

    
}
    






