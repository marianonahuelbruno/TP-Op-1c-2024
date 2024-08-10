#include "../include/CPU_mmu.h"

void inicializar_TLB()
{
    if (cant_entradas_TLB == 0)
    {
        usa_TLB = false;
        log_info(logger, "TLB deshabilitada");
    }
    else
    {
        usa_TLB = true;
        log_info(logger, "TLB habilitada, cantidad de entradas: %u", cant_entradas_TLB);

        tabla_TLB = list_create();
    }    
}

uint32_t obtener_nro_pagina(uint32_t direccion_logica)
{
    return floor(direccion_logica/tamanio_de_pagina);
}

uint32_t obtener_desplazamiento(uint32_t direccion_logica)
{
    return direccion_logica%tamanio_de_pagina;
}

uint32_t get_marco(uint32_t PID_pedida, uint32_t nro_pag)
{
    char* tlb = usa_TLB ? "Usa TLB" : "No usa TLB";
    log_debug(logger_debug, "PID: %u - Necesita la pagina nro: %u. %s", PID_pedida, nro_pag, tlb);
    uint32_t marco;
    if (usa_TLB)
    {
        entrada_TLB* entrada = buscar_en_tlb(PID_pedida, nro_pag);
        marco = marco_TLB(entrada);
    }
    else
    {
        marco = pedir_marco_a_memoria(PID_pedida, nro_pag);
    }
    return marco;
}

entrada_TLB* buscar_en_tlb(uint32_t PID_pedida, uint32_t nro_pag)
{
    entrada_TLB* entrada;
    for (int i = 0; i < list_size(tabla_TLB); i++)
    {
        entrada = list_get(tabla_TLB, i);
        if (entrada->PID == PID_pedida && entrada->nro_pag == nro_pag)
        {
            log_trace(logger, "ID: %u - TLB HIT - Pagina: %u", PID_pedida, nro_pag);
            temporal_destroy(entrada->t_ultimo_uso);
            entrada->t_ultimo_uso = temporal_create();
            return entrada;
        }
    }
    log_trace(logger, "ID: %u - TLB MISS - Pagina: %u", PID_pedida, nro_pag);
    entrada = TLB_miss(PID_pedida, nro_pag);
    return entrada;
}

uint32_t marco_TLB(entrada_TLB* entrada)
{
    return entrada->marco;
}

uint32_t pedir_marco_a_memoria(uint32_t PID, uint32_t nro_pag)
{
    t_paquete* paquete = crear_paquete(TLB_MISS);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, nro_pag);
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);

    log_trace(logger_debug, "Esperando marco de memoria");
    sem_wait(&respuesta_marco);
    uint32_t marco = marco_pedido;

    log_info(logger, "PID: %u - OBTENER MARCO - Página: %u - Marco: %u", PID, nro_pag, marco);
    return marco;
}

entrada_TLB* TLB_miss(uint32_t PID, uint32_t nro_pag)
{
    uint32_t marco = pedir_marco_a_memoria(PID, nro_pag);
    entrada_TLB* entrada = malloc(sizeof(entrada_TLB));
    entrada->PID = PID;
    entrada->nro_pag = nro_pag;
    entrada->marco = marco;
    entrada->t_ingreso = temporal_create();
    entrada->t_ultimo_uso = temporal_create();

    if (list_size(tabla_TLB)==cant_entradas_TLB)
    {
        uint32_t entrada_a_eliminar = buscar_entrada_para_reemplazar();
        entrada_TLB* entrada_removida = list_remove(tabla_TLB, entrada_a_eliminar);
        log_info(logger, "PID: %u, nro_pag: %u - Entrada removida de tabla TLB", entrada_removida->PID, entrada_removida->nro_pag);
        temporal_destroy(entrada_removida->t_ingreso);
        temporal_destroy(entrada_removida->t_ultimo_uso);
        free(entrada_removida);
    }    
    list_add(tabla_TLB, entrada);

    return entrada;
}

uint32_t buscar_entrada_para_reemplazar()
{
    uint32_t entrada_a_reemplazar = 0;
    uint32_t entrada_actual;
    
    for (int i = 1; i < cant_entradas_TLB; i++)
    {
        entrada_actual = i;
        entrada_a_reemplazar = algoritmo_de_reemplazo(entrada_actual, entrada_a_reemplazar);
    }
    return entrada_a_reemplazar;
}

uint32_t algoritmo_de_reemplazo(uint32_t indice_1, uint32_t indice_2)
{
    entrada_TLB* entrada_1 = list_get(tabla_TLB, indice_1);
    entrada_TLB* entrada_2 = list_get(tabla_TLB, indice_2);

    uint32_t reemplazado;
    if (string_equals_ignore_case(algoritmo_TLB, "FIFO"))
    {
        //si diff>0 entonces entrada_1 esta hace mas tiempo en TLB que entrada_2
        reemplazado = temporal_diff(entrada_1->t_ingreso, entrada_2->t_ingreso) > 0 ? indice_1 : indice_2;
    }
    else //(string_equals_ignore_case(algoritmo_TLB, "LRU"))
    {
        //si diff>0 entonces entrada_1 se uso hace mas tiempo que entrada_2
        reemplazado = temporal_diff(entrada_1->t_ultimo_uso, entrada_2->t_ultimo_uso) > 0 ? indice_1 : indice_2;
    }
    return reemplazado;
}

void solicitar_lectura_string(uint32_t direccion_logica_READ, uint32_t bytes_a_copiar)
{
    uint32_t bytes_restantes = bytes_a_copiar;
    
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica_READ);
    uint32_t offset = obtener_desplazamiento(direccion_logica_READ);
    uint32_t marco;
    
    float fin_lectura = (bytes_a_copiar + offset);
    float cant_paginas = fin_lectura / tamanio_de_pagina;

    uint32_t cant_accesos = ceil(cant_paginas);

    t_paquete* paquete = crear_paquete(SOLICITUD_COPY_STRING_READ);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, cant_accesos);
    agregar_a_paquete_uint32(paquete, bytes_a_copiar);
    
    marco = get_marco(PID, nro_pag);

    uint32_t dir_fisica = (marco*tamanio_de_pagina)+offset;
    agregar_a_paquete_uint32(paquete, dir_fisica);

    uint32_t tam_acceso = cant_accesos==1 ? bytes_a_copiar : (tamanio_de_pagina-offset);
    agregar_a_paquete_uint32(paquete, tam_acceso);
    bytes_restantes-=tam_acceso;
    // log_debug(logger_debug, "COPY_STRING read acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);

    int i = 1;
    while (bytes_restantes>0)
    {
        marco = get_marco(PID, nro_pag+i);

        dir_fisica = marco*tamanio_de_pagina;

        if (bytes_restantes>tamanio_de_pagina)
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_uint32(paquete, tamanio_de_pagina);
            // log_debug(logger_debug, "COPY_STRING read acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tamanio_de_pagina);
            bytes_restantes-=tamanio_de_pagina;
        }
        else
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_uint32(paquete, bytes_restantes);
            // log_debug(logger_debug, "COPY_STRING read acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, bytes_restantes);
            bytes_restantes-=bytes_restantes; //aca sale del while
        }
        ++i;
    }
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);  
}

void escribir_en_memoria_string(char* string_leida, uint32_t direccion_logica_WRITE, uint32_t bytes_a_copiar)
{
    uint32_t bytes_leidos = 0;
    
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica_WRITE);
    uint32_t offset = obtener_desplazamiento(direccion_logica_WRITE);
    uint32_t marco;

    float fin_lectura = (bytes_a_copiar + offset);
    float cant_paginas = fin_lectura / tamanio_de_pagina;

    uint32_t cant_accesos = ceil(cant_paginas);

    t_paquete* paquete = crear_paquete(SOLICITUD_COPY_STRING_WRITE);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, cant_accesos);
    
    marco = get_marco(PID, nro_pag);

    uint32_t dir_fisica = (marco*tamanio_de_pagina)+offset;
    uint32_t tam_acceso = cant_accesos==1 ? bytes_a_copiar : (tamanio_de_pagina-offset);

    void* bytes_str = malloc(bytes_a_copiar);
    memcpy(bytes_str, string_leida, bytes_a_copiar);

    agregar_a_paquete_uint32(paquete, dir_fisica);
    agregar_a_paquete_uint32(paquete, tam_acceso);
    agregar_a_paquete_bytes(paquete, tam_acceso, bytes_str);
    bytes_leidos+=tam_acceso;
    uint32_t bytes_restantes = bytes_a_copiar-bytes_leidos;

    // log_debug(logger_debug, "COPY_STRING write acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tam_acceso);

    int i = 1;
    while (bytes_restantes>0)
    {
        marco = get_marco(PID, nro_pag+i);
        
        dir_fisica = marco*tamanio_de_pagina;

        if (bytes_restantes>tamanio_de_pagina)
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_uint32(paquete, tamanio_de_pagina);
            agregar_a_paquete_bytes(paquete, tamanio_de_pagina, bytes_str+bytes_leidos);
            // log_debug(logger_debug, "COPY_STRING write acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, tamanio_de_pagina);
            bytes_leidos+=tamanio_de_pagina;
            bytes_restantes-=tamanio_de_pagina;
        }
        else
        {
            agregar_a_paquete_uint32(paquete, dir_fisica);
            agregar_a_paquete_uint32(paquete, bytes_restantes);
            agregar_a_paquete_bytes(paquete, bytes_restantes, bytes_str+bytes_leidos);
            // log_debug(logger_debug, "COPY_STRING write acceso añadido - dir_fisica: %u, tamanio: %u", dir_fisica, bytes_restantes);
            bytes_leidos+=bytes_restantes;
            bytes_restantes-=bytes_restantes; //aca sale del while
        }
        ++i;
    }
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
    free(bytes_str);
}


uint32_t solicitar_MOV_IN(uint32_t direccion_logica, uint32_t tamanio_registro)
{
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica);
    uint32_t offset = obtener_desplazamiento(direccion_logica);
    uint32_t bytes_restantes = tamanio_registro;
    uint32_t cant_accesos = (offset+tamanio_registro > tamanio_de_pagina) ? 2 : 1;

    uint32_t marco;
    uint32_t dir_fisica;
    uint32_t dir_fisica_inicial;

    t_paquete* paquete = crear_paquete(SOLICITUD_MOV_IN);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, cant_accesos);
    agregar_a_paquete_uint32(paquete, tamanio_registro);


    marco = get_marco(PID, nro_pag);

    dir_fisica = (marco*tamanio_de_pagina)+offset;
    agregar_a_paquete_uint32(paquete, dir_fisica);
    uint32_t tam_acceso = cant_accesos==1 ? tamanio_registro : (tamanio_de_pagina-offset);
    agregar_a_paquete_uint32(paquete, tam_acceso);//n bytes, los faltantes hasta el fin del marco/pagina
    
    dir_fisica_inicial = dir_fisica;
    bytes_restantes-=tam_acceso;

    if (cant_accesos>1)
    {
        marco = get_marco(PID, nro_pag+1);

        dir_fisica = (marco*tamanio_de_pagina);
        agregar_a_paquete_uint32(paquete, dir_fisica);
        agregar_a_paquete_uint32(paquete, bytes_restantes);
    }

    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);

    return dir_fisica_inicial;
}

uint32_t solicitar_MOV_OUT(uint32_t direccion_logica, uint32_t tamanio_registro, int valor)
{
    uint32_t nro_pag = obtener_nro_pagina(direccion_logica);
    uint32_t offset = obtener_desplazamiento(direccion_logica);
    uint32_t bytes_restantes = tamanio_registro;
    uint32_t cant_accesos = (offset+tamanio_registro > tamanio_de_pagina) ? 2 : 1;
    void* puntero_valor = &valor;

    uint32_t marco;
    uint32_t dir_fisica;
    uint32_t dir_fisica_inicial;

    t_paquete* paquete = crear_paquete(SOLICITUD_MOV_OUT);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, cant_accesos);

    marco = get_marco(PID, nro_pag);

    dir_fisica = (marco*tamanio_de_pagina)+offset;
    agregar_a_paquete_uint32(paquete, dir_fisica);

    uint32_t tam_acceso = cant_accesos==1 ? tamanio_registro : (tamanio_de_pagina-offset);
    agregar_a_paquete_string(paquete, tam_acceso, puntero_valor);
    dir_fisica_inicial = dir_fisica;
    
    bytes_restantes -= (tamanio_de_pagina-offset);
    if (cant_accesos>1)
    {
        marco = get_marco(PID, nro_pag+1);

        dir_fisica = (marco*tamanio_de_pagina);
        agregar_a_paquete_uint32(paquete, dir_fisica);
        agregar_a_paquete_string(paquete, bytes_restantes, puntero_valor + (tamanio_de_pagina-offset));
    }

    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
    return dir_fisica_inicial;
}