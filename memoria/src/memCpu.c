#include "../include/memCpu.h"

void conexion_con_cpu(){
    enviar_mensaje("CONEXION CON MEMORIA OK", socket_cpu_memoria);
    log_info(logger, "Handshake enviado: CPU");
    
    bool continuarIterando = true;
    while(continuarIterando){
        //pthread_mutex_unlock(&accediendo_a_memoria);
        op_code codigo = recibir_operacion(socket_cpu_memoria);
        //pthread_mutex_lock(&accediendo_a_memoria);




        switch (codigo){
        case MENSAJE:
            recibir_mensaje(socket_cpu_memoria,logger_debug);
            break;
        case FETCH:
            fetch(socket_cpu_memoria);
            break;
        case TLB_MISS:
            log_trace(logger_debug, "CPU pide un marco a memoria");
            frame(socket_cpu_memoria);
            break;
        case SOLICITUD_MOV_IN: 
            log_trace(logger_debug, "Llega una peticion de MOV_IN");
            movIn();
            break;
        case SOLICITUD_MOV_OUT:
            log_trace(logger_debug, "Llega una peticion de MOV_OUT");
            movOut();
            break;
        case SOLICITUD_COPY_STRING_READ: 
            log_trace(logger_debug, "Llega una peticion de COPY STRING READ");
            copiar_string_read(socket_cpu_memoria);
            break;
        case SOLICITUD_COPY_STRING_WRITE: 
            log_trace(logger_debug, "Llega una peticion de COPY STRING WRITE");
            copiar_string_write(socket_cpu_memoria);
            break;
        case SOLICITUD_RESIZE:
            log_trace(logger_debug, "Llega una peticion de RESIZE");
            ins_resize(socket_cpu_memoria);
            break;
        default:
            log_error(logger_debug, "Modulo CPU se desconectó. Terminando servidor");
            continuarIterando = 0;
            break;
        }
        
    }
}

void fetch(int socket_cpu_memoria){ 
    uint32_t PID; 
    uint32_t PC;
    recibir_fetch(socket_cpu_memoria, &PID, &PC);
    log_trace(logger_debug, "CPU solicita instruccion, PID: %d, PC: %d", PID, PC);
    
   
    
    pthread_mutex_lock(&mutex_listaDeinstrucciones);
    
    t_list* lista_instrucciones = obtener_instrs(PID);
    
    if(lista_instrucciones==NULL){ 
        t_paquete* paquete = crear_paquete(PROCESO_NO_CARGADO);
        enviar_paquete(paquete, socket_cpu_memoria);
        eliminar_paquete(paquete);
        pthread_mutex_unlock(&mutex_listaDeinstrucciones);
    }
    else
    {
        t_instruccion* sig_ins = get_ins(lista_instrucciones, PC);
        pthread_mutex_unlock(&mutex_listaDeinstrucciones);
        
        usleep(retardo*1000);

        enviar_instruccion(socket_cpu_memoria, sig_ins);
        //log_debug(logger_debug, "Instruccion enviada");
    }
}

void frame(int socket_cpu_memoria){
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket_cpu_memoria);
    
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer,&desplazamiento);
        uint32_t pagina = leer_de_buffer_uint32(buffer,&desplazamiento);

        uint32_t marco = encontrar_frame(PID, pagina); 

        log_info(logger,"Acceso a Tabla de Páginas: PID: %u  Pagina: %u  Marco: %u", PID, pagina, marco);

        usleep(retardo*1000);

        t_paquete* paquete = crear_paquete(TLB_MISS);
        agregar_a_paquete_uint32(paquete, marco);
        enviar_paquete(paquete, socket_cpu_memoria);
        eliminar_paquete(paquete);            
        //log_debug(logger_debug, "Se envia el marco: %d, asignado a la pagina: %d, a CPU", marco, pagina);
    }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}

void movIn(){
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket_cpu_memoria);
    int i=0;
    
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t n = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);

        uint32_t bytes_leidos = 0;
        void* bytes_del_nro = malloc(tamanio_total);
        memset(bytes_del_nro, 0, tamanio_total);
        
        while(i<n){
            uint32_t dir_fisica_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
            uint32_t tamanio_acceso = leer_de_buffer_uint32(buffer, &desplazamiento);
            void* leido = leer_memoria(dir_fisica_leer, tamanio_acceso, PID);

            memcpy(bytes_del_nro+bytes_leidos, leido, tamanio_acceso);
            bytes_leidos+=tamanio_acceso;
            free(leido);
            ++i;
        }

        uint32_t num;
        if (tamanio_total==sizeof(uint8_t))
        {
            uint8_t aux;// Crea un contenedor auxiliar
            memcpy(&aux, bytes_del_nro, tamanio_total);// Le asigna al contenedor el valor
            num = aux;// Hace la conversion de uint8 a uint32
        }
        else
        {
            memcpy(&num, bytes_del_nro, sizeof(uint32_t));
        }
        log_debug(logger_debug, "MOV_IN nro leido: %u", num);
        
        t_paquete* paquete = crear_paquete(SOLICITUD_MOV_IN);
        agregar_a_paquete_uint32(paquete, tamanio_total);
        agregar_a_paquete_uint32(paquete, num);
        usleep(retardo*1000);

        enviar_paquete(paquete, socket_cpu_memoria);
        eliminar_paquete(paquete);
        free(bytes_del_nro);
        //log_info(logger_debug, "Mov_In completado");
    }
    else
    {
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}

void movOut(){
    int32_t i=0;
    uint32_t sizeTotal;
    void* buffer = recibir_buffer(&sizeTotal, socket_cpu_memoria);
    uint32_t desplazamiento = 0;
    bool escrito = true;
    
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t n = leer_de_buffer_uint32(buffer, &desplazamiento);

        //log_debug(logger_debug, "PID: %u - Solicitud de MOV_OUT con %u acceso(s)", PID, n);

        while(i<n && escrito){
            uint32_t dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
            uint32_t tamanio_acceso = leer_de_buffer_uint32(buffer, &desplazamiento);
            void* escribir = leer_de_buffer_bytes(buffer, &desplazamiento, tamanio_acceso);

            //log_debug(logger_debug, "PID: %u - acceso de escritura - dir_fisica: %u, tamanio_acceso: %u", PID, dir_fisica, tamanio_acceso);
            escrito = escribir_memoria(dir_fisica, tamanio_acceso, escribir, PID);
            free(escribir);
            ++i;
        }
        usleep(retardo*1000);

        t_paquete* paquete = crear_paquete(SOLICITUD_MOV_OUT);

        if(escrito){
            agregar_a_paquete_op_code(paquete, OK);
            //log_info(logger_debug, "Mov_Out perfecto");
        }else{
            agregar_a_paquete_op_code(paquete, FALLO);
            //log_debug(logger_debug, "Mov_Out **FALLIDO**");
        }
        enviar_paquete(paquete, socket_cpu_memoria);
        eliminar_paquete(paquete);
    }
    else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}

void copiar_string_read(int socket_cpu_memoria){
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket_cpu_memoria);
    int i=0;
    
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t n = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);

        char* str_leida = malloc(tamanio_total+1);
        int32_t bytes_leidos = 0;

        while(i<n){
            uint32_t dir_fisica_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
            uint32_t tam_acceso = leer_de_buffer_uint32(buffer, &desplazamiento);
            //log_debug(logger_debug, "Copy String Read acceso recibido - Dir_fisica: %u, Tamanio: %u", dir_fisica_leer, tam_acceso);

            void* leido = leer_memoria(dir_fisica_leer, tam_acceso, PID);
            memcpy(str_leida+bytes_leidos, leido, tam_acceso);
            bytes_leidos+=tam_acceso;
            free(leido);
            ++i;
        }

        str_leida[tamanio_total] = '\0';
        // log_debug(logger_debug, "Copy String Read completado, string leida: %s", str_leida);
        usleep(retardo*1000);

        t_paquete* paquete = crear_paquete(SOLICITUD_COPY_STRING_READ);
        agregar_a_paquete_string(paquete, strlen(str_leida)+1, str_leida);
        enviar_paquete(paquete, socket_cpu_memoria);
        eliminar_paquete(paquete);
        free(str_leida);

    }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}

void copiar_string_write(int socket_cpu_memoria){
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket_cpu_memoria);
    int i=0;
    bool escrito = true;
    
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t n = leer_de_buffer_uint32(buffer, &desplazamiento);

        while(i<n && escrito){
            uint32_t dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
            uint32_t tamanio_acceso = leer_de_buffer_uint32(buffer, &desplazamiento);
            void* escribir = leer_de_buffer_bytes(buffer, &desplazamiento, tamanio_acceso);
            //log_debug(logger_debug, "Copy String Write acceso recibido - Dir_fisica: %u, Tamanio: %u", dir_fisica, tamanio_acceso);

            escrito = escribir_memoria(dir_fisica, tamanio_acceso, escribir, PID);
            free(escribir);
            ++i;
        }

        usleep(retardo*1000);
        t_paquete* paquete = crear_paquete(SOLICITUD_COPY_STRING_WRITE);
        if(escrito){
            agregar_a_paquete_op_code(paquete, OK);
            //log_info(logger_debug, "Copy String Write completado");

        }else{
            agregar_a_paquete_op_code(paquete, FALLO);
            //log_debug(logger_debug, "Copy String Write **FALLIDO**");
        }
        enviar_paquete(paquete, socket_cpu_memoria);
        eliminar_paquete(paquete);
    }else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}

void ins_resize(int socket_cpu_memoria){
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket_cpu_memoria);

    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t bytes = leer_de_buffer_uint32(buffer, &desplazamiento);

        bool exito = resize(PID, bytes);
        usleep(retardo*1000);

        op_code respuesta;
        if(exito){
            respuesta = SOLICITUD_RESIZE;
            send(socket_cpu_memoria, &respuesta, sizeof(op_code), 0);
            //log_info(logger_debug, "Resize perfecto");
        }else{
            respuesta = OUT_OF_MEMORY;
            send(socket_cpu_memoria, &respuesta, sizeof(op_code), 0);
            //log_info(logger_debug, "Resize fallido");
        }
    }else
    {// Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}

void recibir_fetch(int socket_cpu_memoria, uint32_t* PID, uint32_t* PC){
    uint32_t size;
    uint32_t desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cpu_memoria);

    *PID = leer_de_buffer_uint32(buffer, &desplazamiento);
    *PC = leer_de_buffer_uint32(buffer, &desplazamiento);
    free(buffer);
}

void enviar_instruccion(int socket_cpu_memoria, t_instruccion* instruccion){
    t_paquete* paquete = crear_paquete(FETCH);
    
    agregar_a_paquete_cod_ins(paquete, instruccion->ins);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg1) + 1, instruccion->arg1);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg2) + 1, instruccion->arg2);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg3) + 1, instruccion->arg3);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg4) + 1, instruccion->arg4);
    agregar_a_paquete_string(paquete, strlen(instruccion->arg5) + 1, instruccion->arg5);

    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
}