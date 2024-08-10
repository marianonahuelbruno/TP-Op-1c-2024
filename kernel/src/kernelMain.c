#include "../include/kernelMain.h"

int32_t main(int32_t argc, char* argv[]) {

//INICIALIZO LOS LOGS Y CONGIFURACIONES Y COLAS DE ESTADO
    iniciar_Kernel();


// INICIALIZO SERVIDOR KERNEL
    socket_escucha = iniciar_servidor(puerto_escucha, logger);


//CONECTO CON MEMORIA
    socket_memoria_kernel = crear_conexion(ip_memoria, puerto_memoria);
    log_info(logger_debug,"Kernel conectado a MEMORIA");

 //CONECTO CON CPU
    socket_kernel_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch);
    log_info(logger_debug,"Kernel conectado a CPU dispatch");
    socket_kernel_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt);
     log_info(logger_debug,"Kernel conectado a CPU interrupt");

// ENTRADAS SALIDAS LAS CONECTO DENTRO DEL HILO, YA QUE PODRIA ACEPTAR UNA O MULTIPLES CONEXIONES


    //signal(SIGINT, signal_handler);

      /////////////////////////////////////                AHORA ATENDEMOS LAS CONEXIONES                        //////////////////////////////////////////////////////
    

    // CREO HILO ENTRADA-SALIDA
         pthread_t hilo_entradaSalida;
         pthread_create(&hilo_entradaSalida,NULL,(void*)atender_conexion_ENTRADASALIDA_KERNEL,NULL);
         pthread_detach(hilo_entradaSalida);
    
    
    // CREO HILO ESCUCHA DE CPU DISPATCH
        
        pthread_create(&hilo_CPU_dispatch,NULL,(void*)atender_conexion_CPU_DISPATCH_KERNEL,NULL);
        //pthread_detach(hilo_CPU_dispatch);    

    
                  
    //  INICIO CONSOLA INTERACTIVA  
        pthread_t hilo_consola_interactiva;
        pthread_create(&hilo_consola_interactiva,NULL,(void*)iniciar_consola_interactiva,NULL);
        pthread_detach(hilo_consola_interactiva); 
         


    //CREO HILO de MEMORIA
        pthread_t hilo_MEMORIA;
        pthread_create(&hilo_MEMORIA,NULL,(void*)atender_conexion_MEMORIA_KERNEL,NULL);
        pthread_join(hilo_MEMORIA,NULL);

    if (socket_escucha) {liberar_conexion(socket_escucha);}
    if (socket_kernel_cpu_dispatch) {liberar_conexion(socket_kernel_cpu_dispatch);}
    if (socket_kernel_cpu_interrupt) {liberar_conexion(socket_kernel_cpu_interrupt);}
    if (socket_memoria_kernel) {liberar_conexion(socket_memoria_kernel);}
    if (socket_entradasalida_kernel) {liberar_conexion(socket_entradasalida_kernel);}

    log_destroy(logger_debug);
    end_program(logger, config);


    sem_destroy (&control_multiprogramacion);
    sem_destroy (&cantidad_procesos_new);
    sem_destroy (&cantidad_procesos_ready);
    sem_destroy (&cantidad_procesos_ready_prioritario);
    sem_destroy (&cantidad_procesos_en_algun_ready);
    sem_destroy (&cantidad_procesos_bloqueados);
    
    sem_destroy (&semaforo_plp);
    sem_destroy (&semaforo_pcp);
    
    pthread_mutex_destroy(&semaforo_new);
    pthread_mutex_destroy(&semaforo_ready);
    pthread_mutex_destroy(&semaforo_ready_prioridad);
    pthread_mutex_destroy(&semaforo_bloqueado_prioridad);
    pthread_mutex_destroy(&semaforo_bloqueado);
    pthread_mutex_destroy(&semaforo_lista_interfaces);
    pthread_mutex_destroy(&semaforo_recursos);





    return 0;
}



void signal_handler(int signum) {
    printf("Interrupción recibida. Liberando recursos...\n");
        if (socket_escucha) {liberar_conexion(socket_escucha);}
    if (socket_kernel_cpu_dispatch) {liberar_conexion(socket_kernel_cpu_dispatch);}
    if (socket_kernel_cpu_interrupt) {liberar_conexion(socket_kernel_cpu_interrupt);}
    if (socket_memoria_kernel) {liberar_conexion(socket_memoria_kernel);}
    if (socket_entradasalida_kernel) {liberar_conexion(socket_entradasalida_kernel);}
    log_destroy(logger_debug);
    end_program(logger, config);
    
    sem_destroy (&control_multiprogramacion);
    sem_destroy (&cantidad_procesos_new);
    sem_destroy (&cantidad_procesos_ready);
    sem_destroy (&cantidad_procesos_ready_prioritario);
    sem_destroy (&cantidad_procesos_en_algun_ready);
    sem_destroy (&cantidad_procesos_bloqueados);
    
    sem_destroy (&semaforo_plp);
    sem_destroy (&semaforo_pcp);
    
    pthread_mutex_destroy(&semaforo_new);
    pthread_mutex_destroy(&semaforo_ready);
    pthread_mutex_destroy(&semaforo_ready_prioridad);
    pthread_mutex_destroy(&semaforo_bloqueado_prioridad);
    pthread_mutex_destroy(&semaforo_bloqueado);
    pthread_mutex_destroy(&semaforo_lista_interfaces);
    pthread_mutex_destroy(&semaforo_recursos);

}