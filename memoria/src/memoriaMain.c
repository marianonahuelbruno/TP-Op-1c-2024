#include "../include/memoriaMain.h"

int main(int argc, char* argv[]) {
    
// INICIALIZO LOGs Y SEMAFOROS
    inciarlogsYsemaforos();

// OBTENGO LOS VALORES DEL CONFIG
    cargarConfig();

// INICIALIZO EL ESPACIO DE USUARIO DE MEMORIA (PAGINACION)
    inicializarEspacioMem();                                                           
    
// INICIO SERVIDOR MEMORIA
    socket_escucha = iniciar_servidor(puerto_escucha, logger);

// ESPERO QUE SE CONECTE CPU
    log_trace(logger_debug, "Esperando que se conecte CPU");
    socket_cpu_memoria = esperar_cliente(socket_escucha,logger_debug);
    enviar_tam_pag();
    
// ESPERO QUE SE CONECTE EL KERNEL
    log_trace(logger_debug, "Esperando que se conecte KERNEL");
    socket_kernel_memoria = esperar_cliente(socket_escucha,logger_debug);

// CREO HILO ESCUCHA CPU
    pthread_t hilo_cpu_memoria;
    pthread_create(&hilo_cpu_memoria,NULL,(void*)conexion_con_cpu,NULL);
    pthread_detach(hilo_cpu_memoria);


// CREO HILO ESCUCHA ENTRADA-SALIDA
     pthread_t hilo_entradaSalida_memoria;
     pthread_create(&hilo_entradaSalida_memoria,NULL,(void*)conexion_con_es,NULL);
     pthread_detach(hilo_entradaSalida_memoria);



// CREO HILO ESCUCHA KERNEL 
    pthread_t hilo_kernel_memoria;
    pthread_create(&hilo_kernel_memoria,NULL,(void*)conexion_con_kernel,NULL);
    pthread_join(hilo_kernel_memoria, NULL); 




    //--------------------------------------------


    if (socket_cpu_memoria) {liberar_conexion(socket_cpu_memoria);}
    if (socket_kernel_memoria) {liberar_conexion(socket_kernel_memoria);}
    if (socket_entradasalida_memoria) {liberar_conexion(socket_entradasalida_memoria);}
    if (socket_escucha) {liberar_conexion(socket_escucha);}

    return 0;
}

//-------------------------------         FUNCIONES     -----------------------------------------

t_list* leer_pseudocodigo(char* path){
    FILE* archivo =  fopen(path, "r");
    t_list* lista_instrucciones = list_create();
    t_instruccion* instr;

    if (archivo == NULL) {
        log_error(logger_debug, "No se pudo abrir archivo de pseudocodigo");
        return (t_list*) NULL;    
    }

    char linea[60];
    memset(linea, 0, 60);
    while (fgets(linea, 60, archivo) != NULL)
    {   
        if(linea!=NULL){
           
            size_t len = strlen(linea);
            if (len > 0 && linea[len - 1] == '\n') {
            linea[len - 1] = '\0';
        }
            instr = parsear_instruccion(linea);
            if (!instr) 
            {
                log_error(logger_debug, "El archivo de pseudocodigo tiene errores/instrucciones invalidas");
                return (t_list* ) NULL;
                break;
            }
            list_add(lista_instrucciones, instr);
        }
    }

    fclose(archivo);
    //log_debug(logger_debug, "Archivo pseudocodigo leido, cantidad lineas leidas: [%d]", list_size(lista_instrucciones));

    return lista_instrucciones;
}

void free_tokens(char** tokens) {
    if (tokens != NULL) {
        for (int i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }
}

t_instruccion* parsear_instruccion(char* linea){
    
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    //char* ins;
    char* a1=NULL;
    char* a2=NULL;
    char* a3=NULL;
    char* a4=NULL;
    char* a5=NULL;
    //printf("INSTRUCCION1:%s\n",linea);
    
    char** tokens = string_split(linea, " ");
   
    
    switch (hash_ins(linea))
    {
    // 5 argumentos
    case IO_FS_WRITE:
    case IO_FS_READ:
        if (string_array_size(tokens)!=6)
        {
            log_error(logger_debug,"Cantidad incorrecta de argumentos en instruccion");
            liberar_array_de_comando(tokens,string_array_size(tokens));
            free(instruccion);
            return (t_instruccion* ) NULL;
            break;
        }




        a1 =strdup(tokens[1]);
        a2 =strdup(tokens[2]);
        a3 =strdup(tokens[3]);
        a4 =strdup(tokens[4]);
        a5 =strdup(tokens[5]);



        instruccion->ins = hash_ins(linea);
        instruccion->arg1 = a1;
        instruccion->arg2 = a2;
        instruccion->arg3 = a3;
        instruccion->arg4 = a4;
        instruccion->arg5 = a5;
        break;

    // 3 argumentos
    case IO_STDIN_READ:
    case IO_STDOUT_WRITE:
    case IO_FS_TRUNCATE:
        if (string_array_size(tokens)!=4)
        {
            log_error(logger_debug,"Cantidad incorrecta de argumentos en instruccion");
            liberar_array_de_comando(tokens,string_array_size(tokens));
            free(instruccion);
            return (t_instruccion* ) NULL;
            break;
        }
        

        a1 = strdup(tokens[1]);
        a2 = strdup(tokens[2]);
        a3 = strdup(tokens[3]);
        a4 = string_new();
        a5 = string_new();



        instruccion->ins = hash_ins(linea);
        instruccion->arg1 = a1;
        instruccion->arg2 = a2;
        instruccion->arg3 = a3;
        instruccion->arg4 = a4;
        instruccion->arg5 = a5;
        break;

    // 2 argumentos
    case SET: 
    case SUM:
    case SUB:
    case MOV_IN:
    case MOV_OUT:
    case JNZ:
    case IO_GEN_SLEEP:
    case IO_FS_CREATE:
    case IO_FS_DELETE:
        if (string_array_size(tokens)!=3)
        {
            log_error(logger_debug,"Cantidad incorrecta de argumentos en instruccion");
            liberar_array_de_comando(tokens,string_array_size(tokens));
            free(instruccion);
            return (t_instruccion* ) NULL;
            break;
        }


        a1 = strdup(tokens[1]);
        a2 = strdup(tokens[2]);
        a3 = string_new();
        a4 = string_new();
        a5 = string_new();



        instruccion->ins = hash_ins(linea);
        instruccion->arg1 = a1;
        instruccion->arg2 = a2;
        instruccion->arg3 = a3;
        instruccion->arg4 = a4;
        instruccion->arg5 = a5;
        break;    

    // 1 argumento
    case RESIZE:
    case COPY_STRING:
    case WAIT:
    case SIGNAL:
        if (string_array_size(tokens)!=2)
        {
            log_error(logger_debug,"Cantidad incorrecta de argumentos en instruccion");
            liberar_array_de_comando(tokens,string_array_size(tokens));
            free(instruccion);
            return (t_instruccion* ) NULL;
            break;
        }    
        //strcpy(a1,tokens[1]);
        a1 = strdup(tokens[1]);
        a2 = string_new();
        a3 = string_new();
        a4 = string_new();
        a5 = string_new();
        //string_trim(&a1);
        
        instruccion->ins = hash_ins(linea);
        instruccion->arg1 = a1;
        instruccion->arg2 = a2;
        instruccion->arg3 = a3;
        instruccion->arg4 = a4;
        instruccion->arg5 = a5;
        break;
    case EXIT: // 0 argumentos, solo EXIT
        if (string_array_size(tokens)!=1)
        {
            log_error(logger_debug,"Cantidad incorrecta de argumentos en instruccion");
            liberar_array_de_comando(tokens,string_array_size(tokens));
            free(instruccion);
            return (t_instruccion* ) NULL;
            break;
        }
        a1 = string_new();
        a2 = string_new();
        a3 = string_new();
        a4 = string_new();
        a5 = string_new();

        instruccion->ins = hash_ins(linea);
        instruccion->arg1 = a1;
        instruccion->arg2 = a2;
        instruccion->arg3 = a3;
        instruccion->arg4 = a4;
        instruccion->arg5 = a5;
        break;

    default:
        log_error(logger_debug, "Instruccion no reconocida");
        return (t_instruccion* ) NULL;
        liberar_array_de_comando(tokens,string_array_size(tokens));
        break;
    }
    
    liberar_array_de_comando(tokens,string_array_size(tokens));
    return instruccion;
}




cod_ins hash_ins(char* linea){
    char** tokens = string_split(linea, " ");


         if (strcmp(tokens[0], "SET")==0){
            liberar_array_de_comando(tokens,string_array_size(tokens));
            return SET;}
    else if (strcmp(tokens[0], "SUM")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return SUM;}
    else if (strcmp(tokens[0], "SUB")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return SUB;}
    else if (strcmp(tokens[0], "MOV_IN")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return MOV_IN;}
    else if (strcmp(tokens[0], "MOV_OUT")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return MOV_OUT;}
    else if (strcmp(tokens[0], "RESIZE")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return RESIZE;}
    else if (strcmp(tokens[0], "JNZ")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return JNZ;}
    else if (strcmp(tokens[0], "COPY_STRING")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return COPY_STRING;}
    else if (strcmp(tokens[0], "IO_GEN_SLEEP")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return IO_GEN_SLEEP;}
    else if (strcmp(tokens[0], "IO_STDIN_READ")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return IO_STDIN_READ;}
    else if (strcmp(tokens[0], "IO_STDOUT_WRITE")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return IO_STDOUT_WRITE;}
    else if (strcmp(tokens[0], "IO_FS_CREATE")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return IO_FS_CREATE;}
    else if (strcmp(tokens[0], "IO_FS_DELETE")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return IO_FS_DELETE;}
    else if (strcmp(tokens[0], "IO_FS_TRUNCATE")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return IO_FS_TRUNCATE;}
    else if (strcmp(tokens[0], "IO_FS_WRITE")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return IO_FS_WRITE;}
    else if (strcmp(tokens[0], "IO_FS_READ")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return IO_FS_READ;}
    else if (strcmp(tokens[0], "WAIT")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return WAIT;}
    else if (strcmp(tokens[0], "SIGNAL")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return SIGNAL;}
    else if (strcmp(tokens[0], "EXIT")==0){
        liberar_array_de_comando(tokens,string_array_size(tokens));
        return EXIT;}
    else return -1;
}




/*
cod_ins hash_ins(char* ins){
    if (string_equals_ignore_case(ins, "SET")){return SET;}
    else if (string_equals_ignore_case(ins, "SUM")){return SUM;}
    else if (string_equals_ignore_case(ins, "SUB")){return SUB;}
    else if (string_equals_ignore_case(ins, "MOV_IN")){return MOV_IN;}
    else if (string_equals_ignore_case(ins, "MOV_OUT")){return MOV_OUT;}
    else if (string_equals_ignore_case(ins, "RESIZE")){return RESIZE;}
    else if (string_equals_ignore_case(ins, "JNZ")){return JNZ;}
    else if (string_equals_ignore_case(ins, "COPY_STRING")){return COPY_STRING;}
    else if (string_equals_ignore_case(ins, "IO_GEN_SLEEP")){return IO_GEN_SLEEP;}
    else if (string_equals_ignore_case(ins, "IO_STDIN_READ")){return IO_STDIN_READ;}
    else if (string_equals_ignore_case(ins, "IO_STDOUT_WRITE")){return IO_STDOUT_WRITE;}
    else if (string_equals_ignore_case(ins, "IO_FS_CREATE")){return IO_FS_CREATE;}
    else if (string_equals_ignore_case(ins, "IO_FS_DELETE")){return IO_FS_DELETE;}
    else if (string_equals_ignore_case(ins, "IO_FS_TRUNCATE")){return IO_FS_TRUNCATE;}
    else if (string_equals_ignore_case(ins, "IO_FS_WRITE")){return IO_FS_WRITE;}
    else if (string_equals_ignore_case(ins, "IO_FS_READ")){return IO_FS_READ;}
    else if (string_equals_ignore_case(ins, "WAIT")){return WAIT;}
    else if (string_equals_ignore_case(ins, "SIGNAL")){return SIGNAL;}
    else if (string_equals_ignore_case(ins, "EXIT")){return EXIT;}
    else return -1;
}

*/
char* path_completo(char* path_base, char* path_parcial)
{
    char* path = string_duplicate(path_base);
    string_append(&path, path_parcial);
    return path;
}


t_instruccion* get_ins(t_list* lista_instrucciones, uint32_t PC){
    t_instruccion* instruccion = list_get(lista_instrucciones, PC);
    return instruccion;
}

void enviar_tam_pag(){
    uint32_t tam_pag = tam_pagina;
    t_paquete* paquete = crear_paquete(TAM_PAG);
    agregar_a_paquete_uint32(paquete, tam_pag);
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);            
    //log_debug(logger_debug, "Se envia el tamaño de pagina a CPU");
}




void liberar_array_de_comando(char** array_de_comando, int tamanio) {
    for (int i = 0; i < tamanio; ++i) {
         if (array_de_comando[i] != NULL) {
            free(array_de_comando[i]);
            array_de_comando[i] = NULL;
            }
    }     
    free(array_de_comando);
}
