#include "../include/entradasalida-memoria.h"

void gestionar_conexion_memoria()
{
    op_code operacion;
    bool continuar_iterando = true;

    while (continuar_iterando)
    {
        operacion = recibir_operacion(socket_memoria_entradasalida);

        uint32_t size;
        uint32_t desplazamiento;
        void* buffer;
        char* str_recibida;

        switch (operacion)
        {
        case MENSAJE:
            recibir_mensaje(socket_memoria_entradasalida, logger_debug);
            break;

        case SOLICITUD_IO_STDOUT_WRITE:
            desplazamiento = 0;
            buffer = recibir_buffer(&size, socket_memoria_entradasalida);
            str_recibida = leer_de_buffer_string(buffer, &desplazamiento);
            string_leida_memoria = string_duplicate(str_recibida);
            free(buffer);
            free(str_recibida);
            // log_info(logger, "Entrada-salida recibe una respuesta de memoria por STDOUT, string: %s", string_leida_memoria);
            sem_post(&respuesta_memoria);
            break;

        case DESALOJO_POR_IO_FS_WRITE:
            desplazamiento = 0;
            buffer = recibir_buffer(&size, socket_memoria_entradasalida);
            str_recibida = leer_de_buffer_string(buffer, &desplazamiento);
            string_leida_memoria = string_duplicate(str_recibida);
            free(buffer);
            free(str_recibida);
            // log_info(logger, "Entrada-salida recibe una respuesta de memoria por FS_WRITE, string: %s", string_leida_memoria);
            sem_post(&respuesta_memoria);
            break;

        case FALLO:
            log_error(logger, "Modulo MEMORIA desconectado, terminando servidor");
            continuar_iterando = false;
            break;

        default:
            log_error(logger, "Llego una operacion desconocida por socket memoria, op_code: %d", operacion);
            break;
        }
    }
}