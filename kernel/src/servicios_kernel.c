#include "../include/servicios_kernel.h"

uint32_t asignar_pid(){
    
    uint32_t valor_pid;

    pthread_mutex_lock(&mutex_pid);
    valor_pid= identificador_PID;
    identificador_PID++;
    pthread_mutex_unlock(&mutex_pid);

    return valor_pid;

}