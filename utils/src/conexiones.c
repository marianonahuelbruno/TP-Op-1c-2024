#include "../include/conexiones.h"

/*----------Cliente----------*/

int32_t crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int32_t socket_cliente = socket(server_info->ai_family,
                         server_info->ai_socktype,
                         server_info->ai_protocol);;


	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return socket_cliente;
}

void liberar_conexion(int32_t socket_cliente)
{
	close(socket_cliente);
}
/*----------Fin Cliente----------*/

/*----------Servidor----------*/

int32_t iniciar_servidor(char* puerto_escucha, t_log* logger)
{
    uint32_t err;

    struct addrinfo hints, *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    err = getaddrinfo(NULL, puerto_escucha, &hints, &server_info);

    if (err != 0) {
        perror("Error en getaddrinfo");
        log_error(logger, "Error en getaddrinfo");
        exit(EXIT_FAILURE);
    }

    uint32_t fd_escucha = socket(server_info->ai_family,
                            server_info->ai_socktype,
                            server_info->ai_protocol);
    if (fd_escucha == -1) {
        perror("Error al crear el socket del servidor");
        log_error(logger, "Error en getaddrinfo");
        exit(EXIT_FAILURE);
    }
    // Asociamos el socket a un puerto
    err = bind(fd_escucha, server_info->ai_addr, server_info->ai_addrlen);
    if (err == -1) {
        perror("Error al enlazar el socket del servidor");
        log_error(logger, "Error en getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // Escuchamos las conexiones entrantes
    err = listen(fd_escucha, SOMAXCONN);
    if (err == -1) {
        perror("Error al escuchar las conexiones entrantes");
        log_error(logger, "Error al escuchar las conexiones entrantes");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(server_info);
    log_info(logger, "SERVIDOR PREPARADO.");

    return fd_escucha;
}

int32_t esperar_cliente(int32_t socket_servidor, t_log* logger)
{
    // Aceptamos un nuevo cliente
    int32_t socket_cliente = accept(socket_servidor, NULL, NULL);
    if (socket_cliente == -1) {
        perror("Error en accept");
        log_error(logger, "Error en accept: %s", strerror(errno));
        return -1;
    }
    // Registramos un mensaje informativo indicando que se ha conectado un cliente
    log_info(logger, "Se conectó un cliente. Socket del cliente: %d", socket_cliente);
    return socket_cliente;
}

/*----------Fin Servidor----------*/


/*----------Mensajeria----------*/

 void enviar_mensaje(char* mensaje, uint32_t socket)
 {
 	t_paquete* paquete = crear_paquete(MENSAJE);
    uint32_t tamanio = string_length(mensaje)+1;
    agregar_a_paquete_string(paquete, tamanio, mensaje);

    enviar_paquete(paquete, socket);	
	eliminar_paquete(paquete);
 }

t_paquete* crear_paquete(op_code codigo)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	crear_buffer(paquete);
	return paquete;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void enviar_paquete(t_paquete* paquete, int32_t socket_cliente)
{
	int32_t bytes = paquete->buffer->size + sizeof(uint32_t)+sizeof(op_code);
    //imprimir_paquete(paquete);
    void* a_enviar = serializar_paquete(paquete, bytes);
    //verificar_paquete(a_enviar);
    send(socket_cliente, a_enviar, bytes, 0);


	free(a_enviar);
}


void imprimir_paquete(t_paquete* paquete) {
    if (paquete == NULL || paquete->buffer == NULL || paquete->buffer->stream == NULL) {
        printf("El paquete o su buffer es NULL.\n");
        return;
    }

    printf("VERIFICACION PAQUETE ANTES DE SERIALIZAR\n");
    printf("Código de operación: %d\n", paquete->codigo_operacion);

    void* stream = paquete->buffer->stream;
    uint32_t desplazamiento = 0;

    // Leer el primer uint32_t (valor1)
    uint32_t valor1;
    memcpy(&valor1, stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Leer el segundo uint32_t (valor2)
    uint32_t valor2;
    memcpy(&valor2, stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Leer el string
    char* string = strdup((char*)(stream + desplazamiento));

    // Imprimir los valores leídos
    printf("Tamaño total 1: %u\n", valor1);
    printf("Tamaño buffer: %u\n", valor2);
    printf("String: %s\n", string);

    // Liberar la memoria asignada para el string
    free(string);
}
/*
void verificar_paquete(void* buffer) {
    op_code codigo_operacion;
    uint32_t size;
    uint32_t valor1;
    uint32_t valor2;
    char* str;

    // Desplazamiento para recorrer el buffer
    uint32_t desplazamiento = 0;

    // Leer el código de operación
  //  memcpy(&codigo_operacion, buffer + desplazamiento, sizeof(op_code));
  //  desplazamiento += sizeof(op_code);

    // Leer el tamaño del buffer
    memcpy(&size, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Leer el primer uint32_t
    memcpy(&valor1, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Leer el segundo uint32_t
    memcpy(&valor2, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Leer el string (asumimos que el string está al final y es null-terminated)
    str = strdup((char*)(buffer + desplazamiento));

    // Imprimir los valores leídos
    printf("VERIFICACION PAQUETE A ENVIAR SERIALIZADO\n");
    printf("Código de Operación: %d\n", codigo_operacion);
    printf("Tamaño del Buffer: %u\n", size);
    printf("Tamaño total 1: %u\n", valor1);
    printf("Tamaño buffer: %u\n", valor2);
    printf("String: %s\n", str);

    // Liberar la memoria asignada para el string
    free(str);
}

*/



void* serializar_paquete(t_paquete* paquete, uint32_t bytes)
{
	void * magic = malloc(bytes);
	int32_t desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(op_code));
	desplazamiento+= sizeof(op_code);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento+= sizeof(uint32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

op_code recibir_operacion(int32_t socket_cliente){
	op_code cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(op_code), MSG_WAITALL) > 0)
		return cod_op;
	else
	{   
        close(socket_cliente);
        socket_cliente=-1;
		return FALLO;
	}
}

void recibir_mensaje(int32_t socket_cliente, t_log* logger)
{
    uint32_t size;
    uint32_t desplazamiento = 0;
    void* buffer = recibir_buffer(&size, socket_cliente);

    char* mensaje = leer_de_buffer_string(buffer, &desplazamiento);

    log_info(logger, "%s", mensaje);
    free(buffer);
    free(mensaje);
}

void* recibir_buffer(uint32_t* size, int32_t socket_cliente)
{
    void * buffer;

    recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
    buffer = malloc(*size);
    recv(socket_cliente, buffer, *size, MSG_WAITALL);

    return buffer;
}



/*
void* recibir_buffer(uint32_t* size, uint32_t socket_cliente) {
    void* buffer = NULL;

    // Recibir el tamaño del buffer
    ssize_t bytes_received = recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL);
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            fprintf(stderr, "El socket se cerró de manera inesperada\n");
        } else {
            fprintf(stderr, "Error al recibir el tamaño del buffer: %s\n", strerror(errno));
        }
        return NULL;
    }

    // Validar el tamaño recibido
    if (*size == 0 || *size > 1000000) { // 1000000 es un valor arbitrario para evitar tamaños muy grandes
        fprintf(stderr, "Tamaño del buffer inválido: %u\n", *size);
        return NULL;
    }

    // Asignar memoria para el buffer
    buffer = malloc(*size);
    if (buffer == NULL) {
        fprintf(stderr, "Error al asignar memoria para el buffer\n");
        return NULL;
    }

    // Recibir el buffer real
    bytes_received = recv(socket_cliente, buffer, *size, MSG_WAITALL);
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            fprintf(stderr, "El socket se cerró de manera inesperada\n");
        } else {
            fprintf(stderr, "Error al recibir el buffer: %s\n", strerror(errno));
        }
        free(buffer);
        return NULL;
    }

    return buffer;
}
*/
/*----------Fin Mensajeria----------*/


/*----------Serializacion----------*/

void agregar_a_paquete_uint8(t_paquete* paquete, uint8_t numero)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint8_t));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(uint8_t));

	paquete->buffer->size += sizeof(uint8_t);
}


void agregar_a_paquete_uint32(t_paquete* paquete, uint32_t numero)//ESTA FUNCION AGREGA SOLO UN UINT_32, NO AGREGA UN PREFIJO CON EL TAMAÑO
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(uint32_t));

	paquete->buffer->size += sizeof(uint32_t);
}

void agregar_a_paquete_string(t_paquete* paquete, uint32_t tamanio, char* string)//ESTA FUNCION AGREGA EL TAMANIO Y UN STRING = UINT_32||STRING
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(uint32_t));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(uint32_t));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(uint32_t), string, tamanio);

	paquete->buffer->size += tamanio + sizeof(uint32_t);
}

void agregar_a_paquete_bytes(t_paquete* paquete, uint32_t tamanio, void* bytes)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);

	memcpy(paquete->buffer->stream + paquete->buffer->size, bytes, tamanio);

	paquete->buffer->size += tamanio;
}

//-------------------------------------------------------------------------------------------

uint8_t leer_de_buffer_uint8(void* buffer, uint32_t* desplazamiento)
{
    uint8_t valor;

    memcpy(&valor,  buffer + (*desplazamiento), sizeof(uint8_t));

    (*desplazamiento) += sizeof(uint8_t);
    
    return valor;
};

uint32_t leer_de_buffer_uint32(void* buffer, uint32_t* desplazamiento)
{
    uint32_t valor;

    memcpy(&valor,  buffer + (*desplazamiento), sizeof(uint32_t));

    (*desplazamiento) += sizeof(uint32_t);
    
    return valor;
};

char* leer_de_buffer_string(void* buffer, uint32_t* desplazamiento)
{
    uint32_t tamanio = leer_de_buffer_uint32(buffer, desplazamiento);
    char* valor = malloc(tamanio+1);

    memcpy(valor, buffer + (*desplazamiento), tamanio);
    (*desplazamiento) += tamanio;

    return valor;
};

void* leer_de_buffer_bytes(void* buffer, uint32_t* desplazamiento, uint32_t tamanio_leido)
{
    void* bytes = malloc(tamanio_leido);

    memcpy(bytes, buffer + (*desplazamiento), tamanio_leido);
    (*desplazamiento) += tamanio_leido;

    return bytes;
};

void leer_de_buffer_CE(void* buffer, uint32_t* desplazamiento, t_contexto_ejecucion* contexto_contenedor){
    contexto_contenedor->PC = leer_de_buffer_uint32(buffer, desplazamiento);
    contexto_contenedor->AX = leer_de_buffer_uint8(buffer, desplazamiento);
    contexto_contenedor->BX = leer_de_buffer_uint8(buffer, desplazamiento);
    contexto_contenedor->CX = leer_de_buffer_uint8(buffer, desplazamiento);
    contexto_contenedor->DX = leer_de_buffer_uint8(buffer, desplazamiento);
    contexto_contenedor->EAX = leer_de_buffer_uint32(buffer, desplazamiento);
    contexto_contenedor->EBX = leer_de_buffer_uint32(buffer, desplazamiento);
    contexto_contenedor->ECX = leer_de_buffer_uint32(buffer, desplazamiento);
    contexto_contenedor->EDX = leer_de_buffer_uint32(buffer, desplazamiento);
    contexto_contenedor->SI = leer_de_buffer_uint32(buffer, desplazamiento);
    contexto_contenedor->DI = leer_de_buffer_uint32(buffer, desplazamiento);
}

void serializar_CE(t_paquete* paquete, t_contexto_ejecucion contexto)
{
    agregar_a_paquete_uint32(paquete, contexto.PC);// uint32_t PC
    agregar_a_paquete_uint8(paquete, contexto.AX);// uint8_t AX
    agregar_a_paquete_uint8(paquete, contexto.BX);// uint8_t BX
    agregar_a_paquete_uint8(paquete, contexto.CX);// uint8_t CX
    agregar_a_paquete_uint8(paquete, contexto.DX);// uint8_t DX
    agregar_a_paquete_uint32(paquete, contexto.EAX);// uint32_t EAX
    agregar_a_paquete_uint32(paquete, contexto.EBX);// uint32_t EBX
    agregar_a_paquete_uint32(paquete, contexto.ECX);// uint32_t ECX
    agregar_a_paquete_uint32(paquete, contexto.EDX);// uint32_t EDX
    agregar_a_paquete_uint32(paquete, contexto.SI);// uint32_t SI
    agregar_a_paquete_uint32(paquete, contexto.DI);// uint32_t DI
};

void enviar_CE(int32_t socket, uint32_t PID, t_contexto_ejecucion contexto)
{
    t_paquete* paquete = crear_paquete(CONTEXTO);
    agregar_a_paquete_uint32(paquete, PID);
    serializar_CE(paquete, contexto);
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
};

void recibir_CE(int32_t socket, uint32_t* PID_contenedor, t_contexto_ejecucion* contexto_contenedor)
{
    uint32_t size = 0;
    uint32_t desplazamiento = 0;
    void* buffer;

    buffer = recibir_buffer(&size, socket);
    uint32_t PID_aux = leer_de_buffer_uint32(buffer, &desplazamiento);
    memcpy(PID_contenedor, &PID_aux, sizeof(uint32_t));
    leer_de_buffer_CE(buffer, &desplazamiento, contexto_contenedor);

    free(buffer);
};

void agregar_a_paquete_cod_ins(t_paquete* paquete, cod_ins codigo)
{    
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(cod_ins));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &codigo, sizeof(cod_ins));
	paquete->buffer->size += sizeof(cod_ins);    
};


void agregar_a_paquete_cod_interfaz(t_paquete* paquete, cod_interfaz interfaz)
{    
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(cod_interfaz));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &interfaz, sizeof(cod_interfaz));
	paquete->buffer->size += sizeof(cod_interfaz);    
};



cod_ins leer_de_buffer_cod_ins(void* buffer, uint32_t* desplazamiento)
{
    cod_ins codigo;
    memcpy(&codigo,  buffer + (*desplazamiento), sizeof(cod_ins));
    (*desplazamiento) += sizeof(cod_ins);    
    return codigo;
};

void agregar_a_paquete_op_code(t_paquete* paquete, op_code codigo)
{    
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(op_code));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &codigo, sizeof(op_code));
	paquete->buffer->size += sizeof(op_code);    
};

op_code leer_de_buffer_op_code(void* buffer, uint32_t* desplazamiento)
{
    op_code codigo;
    memcpy(&codigo,  buffer + (*desplazamiento), sizeof(op_code));
    (*desplazamiento) += sizeof(op_code);    
    return codigo;
};

cod_interfaz leer_de_buffer_tipo_interfaz(void* buffer, uint32_t* desplazamiento)
{
    cod_interfaz codigo;
    memcpy(&codigo,  buffer + (*desplazamiento), sizeof(cod_interfaz));
    (*desplazamiento) += sizeof(cod_interfaz);    
    return codigo;
};


void enviar_instruccion_con_PID_por_socket(op_code codigo_operacion, uint32_t PID,int32_t socket_a_enviar){

    t_paquete *paquete= crear_paquete (codigo_operacion);
    agregar_a_paquete_uint32(paquete,PID);
    enviar_paquete(paquete,socket_a_enviar);              //--------------ESTA FUNCION SERIALIZA EL PAQUETE ANTES DE ENVIARLO --quedaria un void*= cod_op||SIZE TOTAL||PID(uint_32)
    eliminar_paquete(paquete);

}

uint32_t recibir_de_buffer_solo_PID (int32_t socket_a_recibir){      /// PARA EJECUTAR ESTA INSTRUCCION CONSIDERO QUE EL CODIGO DE OPERACION YA ESTA EXTRAIDO, SOLO QUEDA=  void*= SIZE TOTAL||PID(uint_32)
    uint32_t *sizeTotal=malloc(sizeof(uint32_t));
    uint32_t desplazamiento = 0;
    void* buffer= recibir_buffer(sizeTotal,socket_a_recibir);
    uint32_t PID = 0; 

    if (buffer != NULL) {
        PID = leer_de_buffer_uint32(buffer, &desplazamiento);
    }
    return PID;
}

char* codigo_operacion_string(op_code codigo){  ////////////////////////// TRANSFORMA EL CODIGO DE OPERACION EN UN STRING PARA LOGGEO
    if(codigo == CONTEXTO)                               return "CONTEXTO";   
    if(codigo == SIG_INS)                                return "SIG_INS";   
    if(codigo == FETCH)                                 return "FETCH";
    if(codigo == PROCESO_NO_CARGADO)                     return "PROCESO_NO_CARGADO";               
    if(codigo == TAM_PAG)                                return "TAM_PAG";   
    if(codigo == TLB_MISS)                               return "TLB_MISS";   
    if(codigo == CREAR_PROCESO)                          return "CREAR_PROCESO";       
    if(codigo == ELIMINAR_PROCESO)                       return "ELIMINAR_PROCESO";           
    if(codigo == CARGA_EXITOSA_PROCESO)                  return "CARGA_EXITOSA_PROCESO";               
    if(codigo == ERROR_AL_CARGAR_EL_PROCESO)             return "ERROR_AL_CARGAR_EL_PROCESO";                       
    if(codigo == OUT_OF_MEMORY)                          return "OUT_OF_MEMORY";       
    if(codigo == SOLICITUD_IO_STDIN_READ)                return "SOLICITUD_IO_STDIN_READ";                   
    if(codigo == SOLICITUD_IO_STDOUT_WRITE)              return "SOLICITUD_IO_STDOUT_WRITE";                   
    if(codigo == SOLICITUD_MOV_OUT)                      return "SOLICITUD_MOV_OUT";           
    if(codigo == SOLICITUD_MOV_IN)                       return "SOLICITUD_MOV_IN";           
    if(codigo == SOLICITUD_RESIZE)                       return "SOLICITUD_RESIZE";           
    if(codigo == SOLICITUD_COPY_STRING_READ)             return "SOLICITUD_COPY_STRING_READ";                       
    if(codigo == SOLICITUD_COPY_STRING_WRITE)            return "SOLICITUD_COPY_STRING_WRITE";                       
    if(codigo == RECIBIR_CE_DISPATCH)                    return "RECIBIR_CE_DISPATCH";               
    if(codigo == MENSAJE)                                return "MENSAJE";   
    if(codigo == HANDSHAKE)                              return "HANDSHAKE";   
    if(codigo == PAQUETE)                                return "PAQUETE";   
    if(codigo == DESALOJO_POR_WAIT)                      return "DESALOJO_POR_WAIT";           
    if(codigo == DESALOJO_POR_SIGNAL)                    return "DESALOJO_POR_SIGNAL";               
    if(codigo == DESALOJO_POR_QUANTUM)                   return "DESALOJO_POR_QUANTUM";               
    if(codigo == DESALOJO_POR_FIN_PROCESO)               return "DESALOJO_POR_FIN_PROCESO";                   
    if(codigo == DESALOJO_POR_CONSOLA )                  return "DESALOJO_POR_CONSOLA";               
    if(codigo == DESALOJO_POR_INTERRUPCION)              return "DESALOJO_POR_INTERRUPCION";                   
    if(codigo == DESALOJO_POR_IO_GEN_SLEEP)              return "DESALOJO_POR_IO_GEN_SLEEP";                   
    if(codigo == DESALOJO_POR_IO_STDIN)                  return "DESALOJO_POR_IO_STDIN";               
    if(codigo == DESALOJO_POR_IO_STDOUT)                 return "DESALOJO_POR_IO_STDOUT";                   
    if(codigo == DESALOJO_POR_IO_FS_CREATE)              return "DESALOJO_POR_IO_FS_CREATE";                   
    if(codigo == DESALOJO_POR_IO_FS_DELETE)              return "DESALOJO_POR_IO_FS_DELETE";                   
    if(codigo == DESALOJO_POR_IO_FS_TRUNCATE)            return "DESALOJO_POR_IO_FS_TRUNCATE";                       
    if(codigo == DESALOJO_POR_IO_FS_WRITE)               return "DESALOJO_POR_IO_FS_WRITE";                   
    if(codigo == DESALOJO_POR_IO_FS_READ)                return "DESALOJO_POR_IO_FS_READ";                   
    if(codigo == VERIFICAR_CONEXION)                     return "VERIFICAR_CONEXION";               
    if(codigo == FINALIZA_IO)                            return "FINALIZA_IO";       
    if(codigo == OK)                                    return "OK";
    if(codigo == FALLO)                                 return "FALLO";
    if(codigo == NUEVA_IO)                               return "NUEVA_IO";   
    if(codigo == SOLICITUD_EXITOSA_IO)                   return "SOLICITUD_EXITOSA_IO";               
    if(codigo == ERROR_SOLICITUD_IO)                     return "ERROR_SOLICITUD_IO";               
    else                    							return "OPERACION NO EXISTE";
}
/*- --------Fin Serializacion----------*/    