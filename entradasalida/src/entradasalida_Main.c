#include "../include/entradasalida_Main.h"

int32_t main(int32_t argc, char* argv[]) {

    if (argc < 2) {
    fprintf(stderr, "Uso: %s <nombre_interfaz>\n", argv[0]);
    return 1;
    }

    printf("Argumento : %s\n", argv[1]);
   
      
    //VALIDO ARGUMENTOS
    validar_argumentos(argv[1]);
    
    nombre_interfaz = argv[1];

    size_t len = strlen(nombre_interfaz);
    if (len > 0 && nombre_interfaz[len - 1] == '\n') {
            nombre_interfaz[len - 1] = '\0';
    }

    iniciar_entradasalida(nombre_interfaz);

    socket_kernel_entradasalida = crear_conexion(IP_KERNEL,PUERTO_KERNEL);
    log_info(logger, "Se creo la conexion entre IO y Kernel");

    enviar_mensaje("CONEXION CON INTERFAZ OK", socket_kernel_entradasalida);
    log_info(logger_debug, "Handshake enviado: KERNEL");

    recibir_operacion(socket_kernel_entradasalida);
    recibir_mensaje(socket_kernel_entradasalida, logger);

    //Envio nombre y tipo a kernel:::  op_code (nueva IO) ||  cod_interfaz tipo interfaz || string nombre interfaz
    t_paquete* paquete= crear_paquete(NUEVA_IO);
    cod_interfaz interfaz = get_tipo_interfaz(TIPO_INTERFAZ);
    agregar_a_paquete_cod_interfaz(paquete,interfaz);
    
    uint32_t tamanio=string_length(nombre_interfaz)+1;
    agregar_a_paquete_string(paquete,tamanio,nombre_interfaz);
    enviar_paquete(paquete,socket_kernel_entradasalida);
    eliminar_paquete(paquete);
    log_debug(logger_debug,"Se confirma a kernel la creacion de la IO");
    
    if (interfaz!=GENERICA)
    {
        socket_memoria_entradasalida = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);
        log_info(logger, "Se creo la conexion entre IO y MEMORIA");

        enviar_mensaje("CONEXION CON INTERFAZ OK", socket_memoria_entradasalida);
        log_info(logger_debug, "Handshake enviado: MEMORIA");

        pthread_t hilo_conexion_memoria;
        pthread_create(&hilo_conexion_memoria, NULL, (void*) gestionar_conexion_memoria, NULL);
        pthread_detach(hilo_conexion_memoria);
    }
    

    if (string_equals_ignore_case(TIPO_INTERFAZ, "DIALFS"))
    {
        inicializar_FS();
    }
    else {bloques = NULL;}

    bool continuarIterando = true;
    op_code cod_op;
    uint32_t size;
    
    void* buffer;

    uint32_t PID;
    char* nombre_archivo;
    uint32_t tamanio_total;
    uint32_t cant_accesos;
    uint32_t dir_fisica;
    uint32_t tamanio_a_leer;

    uint32_t puntero;
    char* path_archivo_metadata;
    t_config* metadata;
    uint32_t tamanio_archivo;
    uint32_t bloque_inicial;
    uint32_t acumulador;

    while (continuarIterando) {
        
        cod_op = recibir_operacion(socket_kernel_entradasalida);
        
        // char* operacion= codigo_operacion_string(cod_op);

        // log_debug(logger_debug, " A la interfaz se le solicito la operacion: %s", operacion);

        uint32_t desplazamiento = 0;

        switch (cod_op) {
        case MENSAJE:
            recibir_mensaje(socket_kernel_entradasalida, logger);
            break;
        case DESALOJO_POR_IO_GEN_SLEEP:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            uint32_t unidades_trabajo = leer_de_buffer_uint32(buffer, &desplazamiento);
            log_info(logger,"PID: %u - Operacion: IO_GEN_SLEEP unidades de trabajo %u", PID, unidades_trabajo);

            usleep(unidades_trabajo*TIEMPO_UNIDAD_TRABAJO);
            log_trace(logger, "PID: %u - Finaliza GEN_SLEEP", PID);
            notificar_kernel(true);
            free(buffer);
            break;
        case DESALOJO_POR_IO_STDIN:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_STDIN_READ", PID);
            tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);
            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);
            char* string_leida = leer_de_teclado(tamanio_total);
            
            paquete = crear_paquete(SOLICITUD_IO_STDIN_READ);
            agregar_a_paquete_uint32(paquete, PID);
            // agregar_a_paquete_uint32(paquete, tamanio_total);
            agregar_a_paquete_uint32(paquete, cant_accesos);

            acumulador = 0;
            for (int i = 0; i < cant_accesos; i++)
            {
                uint32_t dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
                uint32_t tamanio_a_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
                
                agregar_a_paquete_uint32(paquete, dir_fisica);
                agregar_a_paquete_string(paquete, tamanio_a_leer, string_leida+acumulador);
                acumulador+=tamanio_a_leer;
            }
            
            free(string_leida);

            enviar_paquete(paquete, socket_memoria_entradasalida);
            eliminar_paquete(paquete);
            free(buffer);
            notificar_kernel(true);
            break;

        case DESALOJO_POR_IO_STDOUT:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_STDOUT_WRITE ", PID);
            tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);
            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);
            
            // Pedir lectura de string a memoria
            paquete = crear_paquete(SOLICITUD_IO_STDOUT_WRITE);
            agregar_a_paquete_uint32(paquete, PID);
            agregar_a_paquete_uint32(paquete, cant_accesos);
            agregar_a_paquete_uint32(paquete, tamanio_total);
            
            for (int i = 0; i < cant_accesos; i++)
            {
                dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
                tamanio_a_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
                
                // log_debug(logger_debug, "Peticion de lectura enviada a memoria - dir_fisica: %u, tam_acceso: %u", dir_fisica, tamanio_a_leer);
                agregar_a_paquete_uint32(paquete, dir_fisica);
                agregar_a_paquete_uint32(paquete, tamanio_a_leer);
            }

            enviar_paquete(paquete, socket_memoria_entradasalida);
            eliminar_paquete(paquete);
            
            sem_wait(&respuesta_memoria);
            // // Enviar string a kernel para que lo imprima
            // paquete = crear_paquete(DESALOJO_POR_IO_STDOUT);
            // agregar_a_paquete_string(paquete, tamanio_total, string_leida_memoria);
            // enviar_paquete(paquete, socket_kernel_entradasalida);
            // eliminar_paquete(paquete);
            // log_info(logger, "Se envio la string \'%s\', a kernel para que sea imprimida en pantalla", string_leida_memoria);
            // Imprimir por pantalla
            
            log_debug(logger_debug, "String para imprimir: %s", string_leida_memoria);
            printf("\n%s\n", string_leida_memoria);
            notificar_kernel(true);
            free(buffer);
            free(string_leida_memoria);
            break;

        case DESALOJO_POR_IO_FS_CREATE:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_FS_CREATE", PID);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger, "PID: %u - Crear Archivo: %s", PID, nombre_archivo);
            bool creado = crear_archivo(nombre_archivo);

            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
            notificar_kernel(creado);
            free(buffer);
            free(nombre_archivo);
            break;

        case DESALOJO_POR_IO_FS_DELETE:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_FS_DELETE", PID);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            log_info(logger, "PID: %u - Eliminar Archivo: %s", PID, nombre_archivo);
            bool eliminado = eliminar_archivo(nombre_archivo);

            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
            notificar_kernel(eliminado);
            free(buffer);
            free(nombre_archivo);
            break;

        case DESALOJO_POR_IO_FS_TRUNCATE:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);

            log_info(logger,"PID: %u - Operacion: IO_FS_TRUNCATE", PID);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            int32_t nuevo_tamanio = leer_de_buffer_uint32(buffer, &desplazamiento);
            log_info(logger, "PID: %u - Truncar Archivo: %s", PID, nombre_archivo);
            bool truncado = truncar_archivo(PID, nombre_archivo, nuevo_tamanio);

            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
            notificar_kernel(truncado);
            free(buffer);
            free(nombre_archivo);
            break;

        case DESALOJO_POR_IO_FS_WRITE:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);
            puntero = leer_de_buffer_uint32(buffer, &desplazamiento);
            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);
            
            // Pedir lectura de string a memoria
            paquete = crear_paquete(DESALOJO_POR_IO_FS_WRITE);
            agregar_a_paquete_uint32(paquete, PID);
            agregar_a_paquete_uint32(paquete, cant_accesos);
            agregar_a_paquete_uint32(paquete, tamanio_total);
            
            for (int i = 0; i < cant_accesos; ++i)
            {
                dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
                tamanio_a_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
                // log_debug(logger_debug, "acceso a memoria añadido - dir fisica: %u, tam acceso: %u", dir_fisica, tamanio_a_leer);

                agregar_a_paquete_uint32(paquete, dir_fisica);
                agregar_a_paquete_uint32(paquete, tamanio_a_leer);
            }
            enviar_paquete(paquete, socket_memoria_entradasalida);
            eliminar_paquete(paquete);

            // Esperar respuesta y grabarla en disco
            sem_wait(&respuesta_memoria);

            path_archivo_metadata = string_duplicate(path_metadata);
            string_append(&path_archivo_metadata, nombre_archivo);
            metadata = config_create(path_archivo_metadata);
            tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
            bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");

            bool write;
            log_info(logger, "PID: %u - Escribir Archivo: %s - Tamaño a Escribir: %u - Puntero Archivo: %u", PID, nombre_archivo, tamanio_total, puntero);
            if (puntero+tamanio_total >= tamanio_archivo)
            {
                log_error(logger, "PID: %u trato de escribir en disco al archivo: %s mas alla de su tamaño asignado", PID, nombre_archivo);
                write = false;
            }
            else
            {
                FS_WRITE(bloques, bloque_inicial, puntero, tamanio_total, string_leida_memoria);
                write = true;
            }

            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
            notificar_kernel(write);

            config_destroy(metadata);
            free(buffer);
            free(nombre_archivo);
            free(string_leida_memoria);
            free(path_archivo_metadata);
            break;

        case DESALOJO_POR_IO_FS_READ:
            buffer = recibir_buffer(&size, socket_kernel_entradasalida);
            PID = leer_de_buffer_uint32(buffer, &desplazamiento);
            nombre_archivo = leer_de_buffer_string(buffer, &desplazamiento);
            tamanio_total = leer_de_buffer_uint32(buffer, &desplazamiento);
            puntero = leer_de_buffer_uint32(buffer, &desplazamiento);
            cant_accesos = leer_de_buffer_uint32(buffer, &desplazamiento);

            path_archivo_metadata = string_duplicate(path_metadata);
            string_append(&path_archivo_metadata, nombre_archivo);
            metadata = config_create(path_archivo_metadata);
            tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
            bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
            free(path_archivo_metadata);

            log_info(logger, "PID: %u - Escribir Archivo: %s - Tamaño a Escribir: %u - Puntero Archivo: %u", PID, nombre_archivo, tamanio_total, puntero);
            void* datos_leidos = NULL;
            bool read;
            if (puntero+tamanio_total >= tamanio_archivo)
            {
                log_error(logger, "PID: %u trato de leer de disco al archivo: %s mas alla de su tamaño asignado", PID, nombre_archivo);
                read = false;
            }
            else
            {
                datos_leidos = malloc(tamanio_total);
                FS_READ(bloques, bloque_inicial, puntero, tamanio_total, datos_leidos);

                // char* string_leida = mem_hexstring(datos_leidos, tamanio_total);

                // log_debug(logger_debug, "Datos leidos: %s", string_leida);

                t_paquete* paq = crear_paquete(DESALOJO_POR_IO_FS_READ);
                agregar_a_paquete_uint32(paq, PID);
                //agregar_a_paquete_uint32(paq, tamanio_total);
                agregar_a_paquete_uint32(paq, cant_accesos);

                acumulador = 0;
                for (int i = 0; i < cant_accesos; i++)
                {
                    dir_fisica = leer_de_buffer_uint32(buffer, &desplazamiento);
                    tamanio_a_leer = leer_de_buffer_uint32(buffer, &desplazamiento);
                    
                    // log_debug(logger_debug, "acceso a memoria añadido - dir fisica: %u, tam acceso: %u", dir_fisica, tamanio_a_leer);
                    agregar_a_paquete_uint32(paq, dir_fisica);
                    agregar_a_paquete_uint32(paq, tamanio_a_leer);
                    agregar_a_paquete_bytes(paq, tamanio_a_leer, datos_leidos+acumulador);
                    acumulador+=tamanio_a_leer;
                }
                enviar_paquete(paq, socket_memoria_entradasalida);
                eliminar_paquete(paq);
                read = true;
            }

            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
            notificar_kernel(read);
            
            config_destroy(metadata);
            free(buffer);
            free(datos_leidos);
            free(nombre_archivo);
            break;

        case FALLO:
            log_error(logger, "KERNEL SE DESCONECTO. Terminando PROCESO");
            continuarIterando=false;
            break;

        case VERIFICAR_CONEXION:
            //log_info(logger, "Kernel pide verificar la conexion");
            break;

        default:
            log_warning(logger,"Operacion desconocida de ENTRADA Y SALIDA. Codigo: %d", cod_op);
            break;
        
        }
    }

    config_destroy(config);
    log_destroy(logger);
    log_destroy(logger_debug);

    if (bloques) {munmap(bloques, BLOCK_SIZE*BLOCK_COUNT);}
    if (array_bitmap) {munmap(array_bitmap, BLOCK_COUNT/8);}
    if (socket_memoria_entradasalida) {liberar_conexion(socket_memoria_entradasalida);}
    if (socket_kernel_entradasalida) {liberar_conexion(socket_kernel_entradasalida);}

    return 0;
}

void validar_argumentos(char* nombre_interfaz)
{
    if(nombre_interfaz == NULL ){
        printf("Agregar argumentos 'nombre_interfaz");
        exit(EXIT_FAILURE);
    }
}

void notificar_kernel(bool exito)
{
    op_code codigo = exito ? SOLICITUD_EXITOSA_IO : ERROR_SOLICITUD_IO;
    send(socket_kernel_entradasalida, &codigo, sizeof(op_code), 0);
}

char* leer_de_teclado(uint32_t tamanio_a_leer)
{
    char* string_a_memoria = string_new();
    char* leido = malloc(tamanio_a_leer);
    uint32_t restante = tamanio_a_leer;

    //Primera lectura de teclado
    char* mensaje_mostrado = malloc(25);
    sprintf(mensaje_mostrado, "Ingresar %d caracteres\n", tamanio_a_leer);
    leido = readline(mensaje_mostrado);
    string_append(&string_a_memoria, leido);
    restante-=string_length(leido);
    free(mensaje_mostrado);
    free(leido);

    //Si lo leido no ocupa todo el tamanio_a_leer sigue pidiendo datos hasta completar
    while (restante>0)
    {
        mensaje_mostrado = malloc(25);
        sprintf(mensaje_mostrado, "Ingresar %d caracteres", restante);
        leido = readline(mensaje_mostrado);
        string_append(&string_a_memoria, leido);
        restante-=string_length(leido);

        if (string_equals_ignore_case(leido, "")) {break;}

        free(mensaje_mostrado);
        free(leido);
    }
    
    //Verifica que no se pase del tamaño pedido
    char* string_resultado = string_substring_until(string_a_memoria, tamanio_a_leer);
    free(string_a_memoria);
    string_resultado[tamanio_a_leer] = '\0';

    return string_resultado;
}