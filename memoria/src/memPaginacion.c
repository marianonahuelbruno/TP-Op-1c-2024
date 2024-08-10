#include "../include/memPaginacion.h"

t_bitarray* bitmap;
t_list* tablaDePaginas;
t_list* procesos;

bool crear_procesoM(char* path_instrucciones, uint32_t PID){ 
    procesoM* proceso = malloc(sizeof(procesoM));
    proceso->pid = PID;
    t_list* lista_inst = leer_pseudocodigo(path_instrucciones);
    if(lista_inst==NULL){
        perror("Error al crear la lista de instrucciones");
        return false;
    }else{proceso->instrs = lista_inst;}

    t_list* tabla_paginas = list_create(); //empieza con tabla de paginas vacia
    proceso->paginas = tabla_paginas;

    añadirTablaALista(tabla_paginas, PID);
    
    pthread_mutex_lock(&mutex_procesos);
    list_add(procesos, proceso);
    pthread_mutex_unlock(&mutex_procesos);
   
    
    //log_debug(logger_debug, "Proceso con PID %d creado", PID);
    return true;
} 

void destruir_instruccion(t_instruccion* instr)
{
    if (instr != NULL) {
        free(instr->arg1);
        free(instr->arg2);
        free(instr->arg3);
        free(instr->arg4);
        free(instr->arg5);
        free(instr);
    }
}

void eliminar_procesoM(uint32_t PID){
    // Buscar el proceso en la lista de procesos
    procesoM* proceso = buscar_proceso_por_pid(PID);
    if (proceso == NULL) {
        log_error(logger_debug, "Proceso con PID %d no encontrado para finalizar", PID);
    }
    else
    {    
    // Liberar los frames del proceso
    liberar_frames(proceso->paginas);

    // Buscar y eliminar la tabla de páginas del proceso
    tabla_pag_proceso* tabla_pag_p = obtener_tabla_pag_proceso(PID);
    if (tabla_pag_p != NULL) {
        log_info(logger, "Destrucción de Tabla de Páginas: PID: %u Tamaño: %d", PID, list_size(tabla_pag_p->paginas));

        for (int i = 0; i < list_size(tablaDePaginas); i++) {
                pthread_mutex_lock(&mutex_tablaDePaginas);
                tabla_pag_proceso* tabla = list_get(tablaDePaginas, i);
                pthread_mutex_unlock(&mutex_tablaDePaginas);
            
            if (tabla->pid == PID) {
                // Liberar la memoria asociada a la tabla de páginas del proceso
                list_destroy_and_destroy_elements(tabla->paginas, free);
                pthread_mutex_lock(&mutex_tablaDePaginas);
                list_remove_and_destroy_element(tablaDePaginas, i, free);
                pthread_mutex_unlock(&mutex_tablaDePaginas);
                break;
            }
        }
    }

    // Eliminar el proceso de la lista de procesos
    pthread_mutex_lock(&mutex_procesos);
       
    for (int i = 0; i < list_size(procesos); i++) {
        procesoM* p = list_get(procesos, i);
        if (p->pid == PID) {
            list_remove(procesos, i);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_procesos);

    // Liberar la memoria asociada al proceso

    list_destroy_and_destroy_elements(proceso->instrs, (void*) destruir_instruccion);
    
    //list_destroy(proceso->paginas); (ya esta liberada ya que es la misma lista que en su tabla)
    free(proceso);

    //log_debug(logger_debug, "Proceso con PID %d finalizado y eliminado", PID);
    }
}

uint32_t encontrar_frame(uint32_t PID, uint32_t pagina){
    tabla_pag_proceso* tabla_pid = obtener_tabla_pag_proceso(PID);
    tabla_pag* tabla_pagina = obtener_pagina_proceso(tabla_pid, pagina);
    uint32_t frame = tabla_pagina->marco;
    return frame;
}

void liberar_frames(t_list* paginas){
    for (int i = 0; i < list_size(paginas); i++) {
        tabla_pag* pagina = list_get(paginas, i);
        if (pagina->presencia) {
            bitarray_clean_bit(bitmap, pagina->marco);
        }
    }
}

bool resize(uint32_t PID, uint32_t size) {
    tabla_pag_proceso* tabla_proceso = obtener_tabla_pag_proceso(PID);
    if (tabla_proceso == NULL) {
        log_error(logger_debug, "Proceso con PID %d no encontrado", PID);
        return false;
    }

    int num_paginas_actuales = list_size(tabla_proceso->paginas);
    int num_paginas_requeridas;
    int tam_actual = num_paginas_actuales*tam_pagina;

    if (size > tam_actual) { // ampliacion
        // Añadir páginas
        log_info(logger,"Ampliación de Proceso: PID: %u - Tamaño Actual: %d - Tamaño a Ampliar: %u", PID, tam_actual, size);
        num_paginas_requeridas = ((size - tam_actual) + tam_pagina - 1) / tam_pagina;  
        bool exito = añadir_pagina_a_proceso(tabla_proceso, num_paginas_requeridas, PID);
        if(!exito){
            return false;
        }
    } else if (size < tam_actual) { // reduccion
        // Eliminar páginas
        log_info(logger,"Reducción de Proceso: PID: %u - Tamaño Actual: %d - Tamaño a Reducir: %u", PID, tam_actual, size);
        num_paginas_requeridas = (tam_actual - size) / tam_pagina;
        eliminar_pagina_de_proceso(tabla_proceso, num_paginas_requeridas);
    }
    return true;
}

bool añadir_pagina_a_proceso(tabla_pag_proceso* tabla, uint32_t num_paginas, uint32_t PID) {
    for (int i = 0; i < num_paginas; i++) {
        tabla_pag* nueva_pagina = malloc(sizeof(tabla_pag));
        if (nueva_pagina == NULL) {
            log_error(logger_debug, "Error al asignar memoria para la nueva página");
            return false;
        }
        nueva_pagina->marco = -1;  // Asignar marco apropiado
        nueva_pagina->presencia = false;  // Inicialmente no presente

        list_add(tabla->paginas, nueva_pagina);
        int numero_pagina = list_size(tabla->paginas) - 1;
        int marco = asignar_marco(PID, numero_pagina);
        if (marco == -1) {
            log_error(logger, "Out Of Memory");
            return false;
        } else {
            //log_info(logger_debug, "Página añadida y asignada al proceso %d. Total de páginas: %d", PID, list_size(tabla->paginas));
        }    
    }
    return true;
}

void eliminar_pagina_de_proceso(tabla_pag_proceso* tabla, int num_paginas) {
    for (int i = 0; i < num_paginas; i++) {
        int index = list_size(tabla->paginas) - 1;
        if (index < 0) break;

        // Obtener la página a eliminar antes de eliminarla de la lista
        tabla_pag* pagina_a_eliminar = list_get(tabla->paginas, index);
        if (pagina_a_eliminar != NULL) {
            // Liberar el marco correspondiente si es necesario
            if (pagina_a_eliminar->presencia) {
                bitarray_clean_bit(bitmap, pagina_a_eliminar->marco);  // Libera el marco en el bitmap
                //log_debug(logger_debug, "Liberado el marco %d del proceso %d", pagina_a_eliminar->marco, tabla->pid);
            }
            free(pagina_a_eliminar);
        }

        // Ahora eliminar la página de la lista
        list_remove(tabla->paginas, index);
        //log_info(logger_debug, "Página eliminada del proceso %d. Total de páginas: %d", tabla->pid, list_size(tabla->paginas));
    }
}

void añadirTablaALista(t_list* paginas, uint32_t PID){  
    // añade una tabla de pagina a la lista global de todas las tablas con su PID

    tabla_pag_proceso* nueva_tabla_proceso = malloc(sizeof(tabla_pag_proceso)); 
    if (nueva_tabla_proceso == NULL) {
        perror("Error al crear la tabla del proceso");
        return;
    }

    nueva_tabla_proceso->pid = PID;
    nueva_tabla_proceso->paginas = paginas;

    log_info(logger, "Creación de Tabla de Páginas: PID: %u Tamaño: %d", PID, list_size(paginas));
    
    pthread_mutex_lock(&mutex_tablaDePaginas);
    list_add(tablaDePaginas, nueva_tabla_proceso);
    pthread_mutex_unlock(&mutex_tablaDePaginas);

}

tabla_pag_proceso* obtener_tabla_pag_proceso(uint32_t PID){
     pthread_mutex_lock(&mutex_tablaDePaginas);
    for (int i = 0; i < list_size(tablaDePaginas); i++) {
        tabla_pag_proceso* tabla = list_get(tablaDePaginas, i);
        if (tabla->pid == PID) {
            //log_info(logger, "Tabla de paginas del proceso PID: %d obtenida con exito", PID);
            pthread_mutex_unlock(&mutex_tablaDePaginas);
            return tabla;
        }
    }
    pthread_mutex_unlock(&mutex_tablaDePaginas);
    return NULL;
}

tabla_pag* obtener_pagina_proceso(tabla_pag_proceso* tabla_proceso, int numero_pagina) {
    if (numero_pagina < list_size(tabla_proceso->paginas)) {
        return list_get(tabla_proceso->paginas, numero_pagina);
    }
    return NULL;
}

tabla_pag* buscar_siguiente_pagina(tabla_pag_proceso* tabla_proceso, int marco_actual) {
    for (int i = 0; i < list_size(tabla_proceso->paginas); i++) {
        tabla_pag* entrada_pagina = list_get(tabla_proceso->paginas, i);
        if (entrada_pagina->marco == marco_actual && entrada_pagina->presencia) {
            // Verificar si hay una página siguiente
            if (i + 1 < list_size(tabla_proceso->paginas)) {
                return list_get(tabla_proceso->paginas, i + 1);
            }
        }
    }
    return NULL;
}

procesoM* buscar_proceso_por_pid(uint32_t pid){
        pthread_mutex_lock(&mutex_procesos);
        
    for (int i = 0; i < list_size(procesos); i++) {
        procesoM* proceso = list_get(procesos, i);
        if (proceso->pid == pid) {
            pthread_mutex_unlock(&mutex_procesos);
            return proceso;
        }
    }
    pthread_mutex_unlock(&mutex_procesos);
    log_error(logger_debug, "Proceso con PID %d no encontrado", pid);
    return NULL;
}

t_list* obtener_instrs(uint32_t pid){
    procesoM* proceso = buscar_proceso_por_pid(pid);
    if (proceso != NULL) {
        return proceso->instrs;
    }
    log_error(logger_debug, "No se pudo obtener la lista de instrucciones del proceso con PID %d", pid);
    return NULL;
}

int asignar_marco(uint32_t PID, int numero_pagina) {
    tabla_pag_proceso* tabla_proceso = obtener_tabla_pag_proceso(PID);
    if (tabla_proceso == NULL) {
        log_error(logger_debug, "Error: no se encontró la tabla de páginas del proceso");
        return -1;
    }
    tabla_pag* pagina = obtener_pagina_proceso(tabla_proceso, numero_pagina);
    if (pagina == NULL) {
        log_error(logger_debug, "Error: no se encontró la tabla de páginas del proceso");
        return -1;
    }

    // Buscar un marco libre en el bitarray
    int cant_frames = bitarray_get_max_bit(bitmap);
    int marco_libre = -1;

    for (int i = 0; i < cant_frames; i++) {
        if (!bitarray_test_bit(bitmap, i)) { // Si el bit está en 0, el marco está libre
            marco_libre = i;
            bitarray_set_bit(bitmap, i); // Marcar el bit como ocupado
            break;
        }
    }

    if (marco_libre == -1) {
        return -1; // No hay marcos disponibles
    }

    // Asignar el marco a la página
    pagina->marco = marco_libre;
    pagina->presencia = true;

    return marco_libre; // Retornar el marco asignado
}


int obtener_marco(int direccion_fisica) {
    return direccion_fisica / tam_pagina;
}

int obtener_desplazamiento(int direccion_fisica) {
    return direccion_fisica % tam_pagina;
}

bool escribir_memoria(uint32_t direccion_fisica, uint32_t tamanio_acceso, void* valor, uint32_t PID)
{
    if (memoria_usuario == NULL || valor == NULL) {
        log_error(logger, "Error: memoria_usuario o valor es NULL.");
        return false;
    }

    if (direccion_fisica+tamanio_acceso > tam_memoria) {
        //log_info(logger_debug, "Out of Memory");
        return false;
    }

    memcpy(memoria_usuario+direccion_fisica, valor, tamanio_acceso);
    
    log_info(logger, "Acceso a Espacio de Usuario: PID: %u - Accion: ESCRIBIR - Direccion fisica: %u - Tamaño %u bytes", PID, direccion_fisica, tamanio_acceso);

    return true;
}

void* leer_memoria(uint32_t direccion_fisica, uint32_t tamanio_acceso, uint32_t PID)
{
    void* buffer = malloc(tamanio_acceso);
    memcpy(buffer, memoria_usuario+direccion_fisica, tamanio_acceso);

    log_info(logger, "Acceso a Espacio de Usuario: PID: %u - Accion: LEER - Direccion fisica: %u - Tamaño %u bytes", PID, direccion_fisica, tamanio_acceso);

    return buffer;
}

char* leer_memoria_string(uint32_t direccion_fisica, uint32_t size, uint32_t PID)
{
    char* buffer = malloc(size+1);    
    memcpy(buffer, (char*)memoria_usuario + direccion_fisica, size);

    log_info(logger, "Acceso a Espacio de Usuario: PID: %u - Accion: LEER - Direccion fisica: %u - Tamaño %u bytes", PID, direccion_fisica, size);

    buffer[size] = '\0';
    return buffer;
}

