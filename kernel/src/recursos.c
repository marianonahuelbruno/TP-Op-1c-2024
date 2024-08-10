
#include "../include/recursos.h"



int32_t* convertir_a_enteros_la_lista_de_instancias(char** array_de_cadenas) {
    
    int32_t contador = 0;
    while (array_de_cadenas[contador] != NULL) {
        contador++;
    }

    // Aloca memoria para el array de enteros
    int32_t* array_de_enteros = malloc(contador * sizeof(int));

    // Convierte cada cadena a un entero y almacénalo en el array de enteros
    for (int32_t i = 0; i < contador; i++) {
        array_de_enteros[i] = atoi(array_de_cadenas[i]);
    }
    cantidadDeRecursos=contador;
    return array_de_enteros;
}



 void construir_lista_de_recursos() {
    
    t_recurso* auxiliar = NULL;
    t_recurso* ultimo = NULL;

    for (int32_t i = 0; i < cantidadDeRecursos; i++) {
        auxiliar = malloc(sizeof(t_recurso));
        

        auxiliar->nombre_recurso = malloc(strlen(recursos[i]) + 1);
 
        strcpy(auxiliar->nombre_recurso, recursos[i]);

        auxiliar->instancias_del_recurso = instancias_recursos_int[i];
        auxiliar->instancias_solicitadas_del_recurso = 0;
        auxiliar->lista_de_espera = list_create();
        auxiliar->lista_de_asignaciones = list_create();        
        auxiliar->siguiente_recurso = NULL;

        if (lista_de_recursos == NULL) {
            lista_de_recursos = auxiliar;  
        } else {
            ultimo->siguiente_recurso = auxiliar;  
        }
        ultimo = auxiliar; 
    }
    
}



void imprimir_recursos(){
    t_recurso* auxiliar = lista_de_recursos;

    while(auxiliar!=NULL){
        log_info(logger_debug, "El recurso '%s', tiene %d instancias, de las cuales utiliza %d",auxiliar->nombre_recurso,auxiliar->instancias_del_recurso,auxiliar->instancias_solicitadas_del_recurso);
        auxiliar = auxiliar->siguiente_recurso;
    }


}




int32_t wait_recursos(char* recurso_solicitado,t_pcb* pcb_solicitante){
    t_recurso* auxiliar = lista_de_recursos;
    
    while(auxiliar!=NULL){
        if(strcmp(auxiliar->nombre_recurso,recurso_solicitado)==0){
            break;
        }else{
            auxiliar = auxiliar->siguiente_recurso;
        }

    }

    if(auxiliar==NULL){                                                                       //RECURSO NO ENCONTRADO
        return -1;
    }

    log_error(logger_debug,"Instancias disponibles del recurso %s = %d",auxiliar->nombre_recurso,auxiliar->instancias_del_recurso - auxiliar->instancias_solicitadas_del_recurso);
    if (auxiliar->instancias_del_recurso - auxiliar->instancias_solicitadas_del_recurso <=0)             //RECURSO ENCONTRADO SIN INSTANCIAS DISPONIBLES
    {
        log_info(logger, "PID: %d - Cambio de estado READY -> BLOQUEADO", pcb_solicitante->PID);        //PCB QUEDO EN COLA DE ESPERA DEL RECURSO
        pcb_solicitante->estado= BLOCKED,
        pthread_mutex_lock(&semaforo_recursos);
        list_add(auxiliar->lista_de_espera,pcb_solicitante);
        uint32_t *Pid=malloc(sizeof(uint32_t));
        *Pid=pcb_solicitante->PID;
        list_add(auxiliar->lista_de_asignaciones,Pid);                                                                                                   // //RECURSO ENCONTRADO CON INSTANCIAS DISPONIBLES
        pthread_mutex_unlock(&semaforo_recursos);
        auxiliar->instancias_solicitadas_del_recurso+=1; 
        
        return 1;
        
    }else{
        uint32_t *Pid=malloc(sizeof(uint32_t));
        *Pid=pcb_solicitante->PID;
        pthread_mutex_lock(&semaforo_recursos);
        list_add(auxiliar->lista_de_asignaciones,Pid);
        pthread_mutex_unlock(&semaforo_recursos);                                                                                                   // //RECURSO ENCONTRADO CON INSTANCIAS DISPONIBLES
        auxiliar->instancias_solicitadas_del_recurso+=1;                                                    //WAIT REALIZADO, DEVOLVER EL PROCESO A EJECUCION
        return 2;
    }
    

}


int32_t signal_recursos ( char*recurso_solicitado,uint32_t PID){                                    
    t_recurso* auxiliar = lista_de_recursos;
    bool encontrado=false;

    
    while(auxiliar!=NULL){
        if(strcmp(auxiliar->nombre_recurso,recurso_solicitado)==0){
            break;
        }else{
            auxiliar = auxiliar->siguiente_recurso;
        }

    }
  

    if(auxiliar==NULL){                                                                //RECURSO NO ENCONTRADO
        return -1;
    }
    

         pthread_mutex_lock(&semaforo_lista_interfaces);
        if(list_size(auxiliar->lista_de_asignaciones)>0){
                                                
    		
            for(int32_t i=0; i<list_size(auxiliar->lista_de_asignaciones); i++){
			    uint32_t *pid_auxiliar = (uint32_t*)list_get(auxiliar->lista_de_asignaciones, i); //Busco el PID en la lista de instancias asignadas a procesos
    
    
                if (*pid_auxiliar == PID){
                    if (list_remove_element(auxiliar->lista_de_asignaciones,pid_auxiliar))
                    {   encontrado=true;
                        auxiliar->instancias_solicitadas_del_recurso-=1;
                        break;
                    }else{
                        log_error(logger_debug,"Se encontro el proceso en lista de asignaciones pero no se pudo eliminar");
                        pthread_mutex_unlock(&semaforo_lista_interfaces);
                        return -1;
                    }

                }
            }
                       
        }



            if (auxiliar->instancias_del_recurso - auxiliar->instancias_solicitadas_del_recurso >=0 && list_size(auxiliar->lista_de_espera)>0)//VERIFICO SI HABIA UN PROCESO ESPERANDO EL RECURSO Y LO LIBERO
            {   
                 t_pcb *pcb_liberado=list_remove(auxiliar->lista_de_espera,0);
                ingresar_en_lista(pcb_liberado, lista_ready, &semaforo_ready, &cantidad_procesos_en_algun_ready , READY);
                log_debug(logger_debug,"Se libero el proceso PID: %d de la cola de espera del recurso %s",pcb_liberado->PID,auxiliar->nombre_recurso);
            }    

             pthread_mutex_unlock(&semaforo_lista_interfaces); 

            if (encontrado)                               
            {   
               return 1;                                     //SIGNAL EXITOSO   
            }else{
                return -2;                                  //SIGNAL SOBRE RECURSO NO ASIGNADO
            }

                                                                                                      

}





bool eliminar_proceso_de_lista_recursos (uint32_t PID){
    bool eliminado=false;
    t_pcb* pcb_a_eliminar;
    t_recurso* auxiliar = lista_de_recursos;
    
    log_info(logger_debug,"Eliminando proceso de la lista de recursos");
    while(auxiliar!=NULL){
        pcb_a_eliminar=buscar_pcb_por_PID_en_lista(auxiliar->lista_de_espera,PID,&semaforo_recursos);    //esta funcion me devuelve el puntero al PCB si lo encuentra o NULL si no lo encuentra
        if(pcb_a_eliminar!=NULL){
            if(list_remove_element(auxiliar->lista_de_espera,pcb_a_eliminar)){
                eliminado=true;
                auxiliar->instancias_solicitadas_del_recurso+=1;
            }else{
                log_error(logger_debug,"Se encontro el proceso con PID: %u en la lista de recursos pero no se pudo eliminar.",PID);
            }
        }

        auxiliar=auxiliar->siguiente_recurso;
    }
    return eliminado;
}



bool eliminar_proceso_de_lista_asignaciones_recurso(uint32_t PID){
    t_recurso* auxiliar = lista_de_recursos;
    bool eliminado=false;
    
    pthread_mutex_lock(&semaforo_lista_interfaces);
    
    while(auxiliar!=NULL){                                  //avanzo recurso

        if(list_size(auxiliar->lista_de_asignaciones)>0){

        
            for(int32_t i=0; i<list_size(auxiliar->lista_de_asignaciones); i++){                    //avanzo lista pid asignados
                uint32_t *pid_auxiliar = (uint32_t*)list_get(auxiliar->lista_de_asignaciones, i);   //Busco el PID en la lista de instancias asignadas a procesos
                //printf("marca1\n");
                if (pid_auxiliar==NULL)
                {
                    log_error(logger_debug,"Pid es igual a null");
                
                }else if (*pid_auxiliar == PID){
                    //printf("marca2\n");
                    if (list_remove_element(auxiliar->lista_de_asignaciones,pid_auxiliar))
                    {   
                        //printf("marca3\n");
                        eliminado=true;
                        log_debug(logger_debug,"Se elimino el proceso de la lista de asignaciones");
                        auxiliar->instancias_solicitadas_del_recurso-=1;
                        printf("Intancias del recurso= %d\n",auxiliar->instancias_del_recurso);
                        printf("Intancias solicitadas del recurso= %d\n",auxiliar->instancias_solicitadas_del_recurso);
                        printf("Cantidad de procesos en espera= %d\n",list_size(auxiliar->lista_de_espera));
                         t_pcb *pcb_auxiliar = (t_pcb *)list_get(auxiliar->lista_de_espera,0);
                        if (auxiliar->instancias_del_recurso - auxiliar->instancias_solicitadas_del_recurso >=0 && list_size(auxiliar->lista_de_espera)>0 && *pid_auxiliar!=pcb_auxiliar->PID)//VERIFICO SI HABIA UN PROCESO ESPERANDO EL RECURSO Y LO LIBERO si no es el que stoy eliminando
                        {   
                                                       
                            //printf("marca4\n");
                            t_pcb *pcb_liberado=list_remove(auxiliar->lista_de_espera,0);
                            log_debug(logger_debug,"Se libero el proceso PID: %d de la cola de espera del recurso %s",pcb_liberado->PID,auxiliar->nombre_recurso);
                            ingresar_en_lista(pcb_liberado, lista_ready, &semaforo_ready, &cantidad_procesos_en_algun_ready , READY);
                        }
                           
                        
                    }else{
                        log_error(logger_debug,"Se encontro el proceso en lista de asignaciones pero no se pudo eliminar");
                       
                    }
                }
                //log_debug(logger_debug,"El valor de pid extraido es: %u",*pid_auxiliar);
            }

        }
            
        auxiliar = auxiliar->siguiente_recurso;
       
    }

    pthread_mutex_unlock(&semaforo_lista_interfaces);
    return eliminado;

}
