#include "../include/memES.h"

void conexion_con_es(){
    while (1)
    {
    // ESPERO QUE SE CONECTE E/S
    
        log_trace(logger_debug, "Esperando que se conecte E/S");
        socket_entradasalida_memoria = esperar_cliente(socket_escucha,logger_debug);
    
        int32_t* socket_de_hilo= malloc(sizeof(int32_t));
        if (socket_de_hilo== NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

        *socket_de_hilo=socket_entradasalida_memoria;
    
        pthread_t hilo_escucha_ENTRADASALIDA_MEMORIA;
        pthread_create(&hilo_escucha_ENTRADASALIDA_MEMORIA,NULL,(void*)escuchar_nueva_Interfaz_mem,socket_de_hilo);    //Hilo donde escucho los mensajes que envia la interfaz recien creada
        pthread_detach(hilo_escucha_ENTRADASALIDA_MEMORIA);
    }
}

void escuchar_nueva_Interfaz_mem(void* socket_hilo)
{
    int32_t socket_IO= *((int32_t*)socket_hilo);

    enviar_mensaje("CONEXION CON MEMORIA OK", socket_IO);
    log_info(logger, "Handshake enviado: IO");
    
    bool continuarIterando = true;
    while(continuarIterando){
        op_code codigo = recibir_operacion(socket_IO);
        switch (codigo){
        case MENSAJE:
            recibir_mensaje(socket_IO,logger_debug);
            break;
        case SOLICITUD_IO_STDIN_READ:
            log_trace(logger_debug, "Llega una peticion de STDIN");
            write_es(socket_IO);
            break;
        case SOLICITUD_IO_STDOUT_WRITE:
            log_trace(logger_debug, "Llega una peticion de STDOUT");
            read_es(socket_IO, SOLICITUD_IO_STDOUT_WRITE);
            break;
        case DESALOJO_POR_IO_FS_READ:
            log_trace(logger_debug, "Llega una peticion de FS_READ");
            write_es(socket_IO);
            break;
        case DESALOJO_POR_IO_FS_WRITE:
            log_trace(logger_debug, "Llega una peticion de FS_WRITE");
            read_es(socket_IO, DESALOJO_POR_IO_FS_WRITE);
            break;
        default:
            log_error(logger_debug, "Modulo ENTRADA SALIDA se desconectó. Cerrando el socket");
            continuarIterando = 0;
            break;
        }
    }
}

void read_es(int32_t socket_hilo, op_code motivo)
{
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket_hilo);
    int i=0;
    
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t n = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t tam_total = leer_de_buffer_uint32(buffer, &desplazamiento);

        char* str_leida = malloc(tam_total+1);
        int32_t bytes_leidos = 0;

        while(i<n){
            uint32_t dir_fisica_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
            uint32_t tam_acceso = leer_de_buffer_uint32(buffer, &desplazamiento);
            // log_debug(logger_debug, "acceso de lectura recibido - dir_fisica: %u, tamanio: %u", dir_fisica_leer, tam_acceso);
            
            void* leido = leer_memoria(dir_fisica_leer, tam_acceso, PID);
            memcpy(str_leida+bytes_leidos, leido, tam_acceso);
            bytes_leidos+=tam_acceso;
            free(leido);
            ++i;
        }

        str_leida[tam_total] = '\0';
        usleep(retardo*1000);

        t_paquete* paquete = crear_paquete(motivo);
        agregar_a_paquete_string(paquete, strlen(str_leida)+1, str_leida);
        enviar_paquete(paquete, socket_hilo);
        eliminar_paquete(paquete);
        //(logger_debug, "Solicitud de lectura de E/S completada, string leida: %s", str_leida);
        free(str_leida);
        }
        else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
        }
    free(buffer);
}

void write_es(int32_t socket_hilo){
    uint32_t sizeTotal;
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(&sizeTotal, socket_hilo);
    int i=0;
    bool escrito = false;
    
    if(buffer != NULL){
        uint32_t PID = leer_de_buffer_uint32(buffer, &desplazamiento);
        // uint32_t tam_total = leer_de_buffer_uint32(buffer, &desplazamiento);
        uint32_t n = leer_de_buffer_uint32(buffer, &desplazamiento);

        while(i<n){
            uint32_t dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
            uint32_t tamanio_acceso = leer_de_buffer_uint32(buffer, &desplazamiento);
            void* escribir = leer_de_buffer_bytes(buffer, &desplazamiento, tamanio_acceso);
            // char* string_escrito = mem_hexstring(escribir, tamanio_acceso);
            
            // log_debug(logger_debug, "acceso de escritura recibido - dir_fisica: %u, tamanio: %u, escrito: %s", dir_fisica, tamanio_acceso, string_escrito);

            escrito = escribir_memoria(dir_fisica, tamanio_acceso, escribir, PID);
            free(escribir);
            ++i;
            if(!escrito){break;}
        }

        usleep(retardo*1000);

        t_paquete* paquete = crear_paquete(MENSAJE);
        char* mensaje;
        if(escrito){
            mensaje = "El pedido de escritura a memoria fue exitoso";
            //log_debug(logger_debug, "El pedido de escritura desde E/S resulto perfecto");
        }else{
            mensaje = "El pedido de escritura a memoria fallo";
            //log_debug(logger_debug, "El pedido de escritura desde E/S resulto fallido");
        }   
        agregar_a_paquete_string(paquete, strlen(mensaje)+1, mensaje);
        enviar_paquete(paquete, socket_hilo);
        eliminar_paquete(paquete);
    }
    else{
        // Manejo de error en caso de que recibir_buffer devuelva NULL
        log_error(logger_debug,"Error al recibir el buffer");
    }
    free(buffer);
}