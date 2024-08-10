#include "../include/entradasalida_FS.h"

void inicializar_FS()
{
    path_metadata = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_metadata, "metadata/");

    path_bloques = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_bloques, "bloques.dat");

    path_bitmap = string_duplicate(PATH_BASE_DIALFS);
    string_append(&path_bitmap, "bitmap.dat");

    // log_debug(logger_debug, "Paths - bloques: %s, bitmap:%s", path_bloques, path_bitmap);

    inicializar_bitmap();
    inicializar_bloques();
}

void inicializar_bloques()
{
    uint32_t tam_archivo_bloques = BLOCK_COUNT*BLOCK_SIZE;

    int fd = open(path_bloques, O_RDWR | O_CREAT, 0777);

    if (fd == -1)
    {
        // perror("No se pudo abrir el archivo bloques.dat");
        log_error(logger, "No se pudo abrir el archivo bloques.dat");
        printf("path bloques: %s\n", path_bloques);
    }
    else
    {
        int trunc = ftruncate(fd, tam_archivo_bloques);
        if (trunc == -1)
        {
            perror("No se pudo truncar el archivo bloques.dat");
        }
        
        bloques = mmap(NULL, tam_archivo_bloques, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
    }
    if (bloques == MAP_FAILED)
    {
        log_error(logger ,"No se pudo mapear el archivo de bloques");
        exit(1);
    }
    // else
    // {
    //     char* dump_bloques = mem_hexstring(bloques, tam_archivo_bloques);
    //     log_info(logger, "Archivo bloques.dat inicializado, hexdump: %s", dump_bloques);
    //     free(dump_bloques);
    // }
}

void inicializar_bitmap()
{
    uint32_t tam_bitmap = BLOCK_COUNT/8;
    
    int fd = open(path_bitmap, O_RDWR | O_CREAT, 0777);
    if (fd == -1)
    {
        // perror("No se pudo abrir el archivo bitmap.dat");
        log_error(logger, "No se pudo abrir el archivo bitmap.dat");
        printf("path bitmap: %s\n", path_bitmap);
        exit(1);
    }
    else
    {
        int trunc = ftruncate(fd, tam_bitmap);
        if (trunc == -1)
        {
            perror("No se pudo truncar el archivo bitmap.dat");
        }
        array_bitmap = mmap(NULL, tam_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
    }
    
    bitmap_bloques = bitarray_create_with_mode(array_bitmap, tam_bitmap, MSB_FIRST);
    if (bitmap_bloques == NULL)
    {
        log_error(logger ,"No se pudo mapear el bitarray");
        exit(1);
    }
    // else
    // {
    //     char* dump_bitmap = mem_hexstring(bitmap_bloques->bitarray, tam_bitmap);
    //     log_info(logger, "Bitmap inicializado, hexdump: %s", dump_bitmap);
    //     free(dump_bitmap);
    // }
}

bool crear_archivo(char* nombre_archivo)
{
    if (existe_archivo(nombre_archivo))
    {
        log_info(logger, "Se intento crear un archivo que ya existe");
        return true;
    }
    
    int32_t bloque = buscar_bloque_libre();
    if (bloque != -1)
    {
        crear_metadata(bloque, nombre_archivo);
        log_info(logger, "Se crea metadata del archivo: %s, bloque inicial: %d", nombre_archivo, bloque);
        return true;
    }
    else
    {
        log_error(logger, "No hay espacio disponible para crear el archivo: %s", nombre_archivo);
        return false;
    }
}

int32_t buscar_bloque_libre()
{
    int32_t bloque_libre = -1;

    for (int32_t i = 0; i < BLOCK_COUNT; ++i) {
        if (!bitarray_test_bit(bitmap_bloques, i)) { // Si el bit está en 0, el bloque está libre
            bloque_libre = i;
            bitarray_set_bit(bitmap_bloques, i); // Marcar el bit como ocupado
            break;
        }
    }
    msync(bitmap_bloques->bitarray, BLOCK_COUNT/8, MS_SYNC);
    
    return bloque_libre; // No hay bloques libres
}

void crear_metadata(int32_t bloque, char* nombre_archivo)
{
    char* path_archivo_metadata = string_duplicate(path_metadata);
    string_append(&path_archivo_metadata, nombre_archivo);
    int fd = open(path_archivo_metadata, O_RDWR | O_CREAT, 0777);
    if (fd == -1)
    {
        perror("No se pudo crear metadata");
        exit(1);
    }

    t_config* metadata = config_create(path_archivo_metadata);
    char* bloque_inicial = string_itoa(bloque);
    config_set_value(metadata, "BLOQUE_INICIAL", bloque_inicial);
    config_set_value(metadata, "TAMANIO_ARCHIVO", "0");
    free(bloque_inicial);

    config_save_in_file(metadata, path_archivo_metadata);
    config_destroy(metadata);
    free(path_archivo_metadata);
}

bool eliminar_archivo(char* nombre_archivo)
{
    if (existe_archivo(nombre_archivo))
    {
        char* path_archivo_metadata = string_duplicate(path_metadata);
        string_append(&path_archivo_metadata, nombre_archivo);

        liberar_bloques(path_archivo_metadata);

        remove(path_archivo_metadata); //Eliminar archivo de metadata
        free(path_archivo_metadata);
        log_info(logger, "Se elimino el archivo: %s con exito", nombre_archivo);
        return true;
    }
    else
    {
        log_warning(logger, "Se trato de eliminar un archivo que no existe: %s", nombre_archivo);
        return false;
    }
}

bool existe_archivo(char* nombre_archivo)
{
    char* path_archivo_metadata = string_duplicate(path_metadata);
    string_append(&path_archivo_metadata, nombre_archivo);
    int fd = open(path_archivo_metadata, O_RDONLY);
    if (fd == -1)
    {
        free(path_archivo_metadata);
        return false;
    }
    else
    {
        free(path_archivo_metadata);
        close(fd);
        return true;
    }
}

void liberar_bloques(char* path_archivo_metadata)
{
    t_config* metadata = config_create(path_archivo_metadata);
    int32_t tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
    int32_t bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
    int32_t cant_bloques = cantidad_de_bloques(tamanio_archivo);
    // log_debug(logger_debug, "archivo: %s, debe liberar %d bloques", path_archivo_metadata, cant_bloques);

    for (int32_t i = 0; i < cant_bloques; ++i)
    {
        // log_debug(logger_debug, "archivo: %s, libera 1 bloque", path_archivo_metadata);
        bitarray_clean_bit(bitmap_bloques, bloque_inicial+i);
    }
    msync(bitmap_bloques->bitarray, BLOCK_COUNT/8, MS_SYNC);
    config_destroy(metadata);
}

bool truncar_archivo(uint32_t PID, char* nombre_archivo, uint32_t nuevo_tamanio)
{
    if (!existe_archivo(nombre_archivo))
    {
        log_error(logger, "PID: %u, trato de truncar un archivo que no existe: %s", PID, nombre_archivo);
        return false;
    }
    else
    {
        char* path_archivo_metadata = string_duplicate(path_metadata);
        string_append(&path_archivo_metadata, nombre_archivo);
        t_config* metadata = config_create(path_archivo_metadata);
        int32_t bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");
        int32_t tamanio_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");

        int32_t cant_bloques = cantidad_de_bloques(tamanio_archivo);
        int32_t nueva_cant_bloques = cantidad_de_bloques(nuevo_tamanio);
        int32_t diferencia_cant_bloques = nueva_cant_bloques - cant_bloques;
        // log_debug(logger_debug, "cant bloques: %d, nueva cant bloques: %d, diferencia: %d", cant_bloques, nueva_cant_bloques, diferencia_cant_bloques);
        free(path_archivo_metadata);

        if (diferencia_cant_bloques<=0)
        {
            char* itoa = string_itoa(nuevo_tamanio);
            config_set_value(metadata, "TAMANIO_ARCHIVO", itoa);
            liberar_n_bloques(bloque_inicial+nueva_cant_bloques, 0-diferencia_cant_bloques);
            config_save_in_file(metadata, path_archivo_metadata);
            config_destroy(metadata);
            free(itoa);
            return true;
        }
        else
        {
            bool asignacion = intentar_asignar_bloques(PID, nombre_archivo, metadata, bloque_inicial, cant_bloques, nueva_cant_bloques);

            if (asignacion)
            {
                char* itoa = string_itoa(nuevo_tamanio);
                config_set_value(metadata, "TAMANIO_ARCHIVO", itoa);
                config_save(metadata);
                config_destroy(metadata);
                free(itoa);
                log_info(logger, "PID: %u - Se trunco con exito el archivo: %s, nuevo tamaño: %u", PID, nombre_archivo, nuevo_tamanio);
            }
            else
            {
                config_destroy(metadata);
                log_warning(logger, "PID: %u - No se pudo truncar el archivo: %s por falta de espacio", PID, nombre_archivo);
                return false;
            }
        }

        return true;
    }
}

bool intentar_asignar_bloques(uint32_t PID, char* nombre_archivo, t_config* metadata, int32_t bloque_inicial, int32_t cant_bloques, int32_t nueva_cant_bloques)
{
    int32_t diferencia_cant_bloques = nueva_cant_bloques - cant_bloques;

    bool asignacion1 = asignar_n_bloques(bloque_inicial+cant_bloques, diferencia_cant_bloques);//Si hay bloques contiguos disponibles, los asigna

    if (asignacion1)
    {
        log_trace(logger_debug, "Exito en la primera asignacion");
        return true;
    }
    else
    {
        log_trace(logger_debug, "Fallo primer intento de asignacion");
        bool asignacion2 = reasignar_bloques(metadata, cant_bloques, nueva_cant_bloques);//Si no, intenta reasignarle bloques al final del bitmap

        if (asignacion2)
        {
            log_trace(logger_debug, "Exito en el segundo intento de asignacion");
            return true;
        }
        else
        {
            log_trace(logger_debug, "Fallo segundo intento de asignacion");
            int32_t nuevo_inicio = compactacion(PID, nombre_archivo, nueva_cant_bloques);//Si no puede, hace compactacion y lo intenta de nuevo
            int32_t n = contar_bloques_libres();
            log_debug(logger_debug, "Archivo: %s, necesita: %d bloques a partir del bloque %d y hay: %d disponibles", 
                                        nombre_archivo, diferencia_cant_bloques, nuevo_inicio+1, n);

            bool asignacion3 = asignar_n_bloques(nuevo_inicio+cant_bloques-1, diferencia_cant_bloques);
            if (asignacion3)
            {
                log_trace(logger_debug, "Exito en el tercer intento de asignacion");
                return true;
            }
            {
                log_trace(logger_debug, "Fallo tercer intento de asignacion");
                return false;
            }
        }
    }
}

int32_t contar_bloques_libres()
{
    int32_t c = 0;
    for (int32_t i = 0; i < BLOCK_COUNT; ++i)
    {if (!bitarray_test_bit(bitmap_bloques, i))
        {++c;}
    }
    return c;
}

int32_t cantidad_de_bloques(int32_t tamanio_archivo)
{
    int32_t a = tamanio_archivo/BLOCK_SIZE;
    int32_t b = tamanio_archivo%BLOCK_SIZE > 0 ? 1 : 0;//Si la cuenta da redonda es +0 si no es +1
    return a + b;
}

void liberar_n_bloques(int32_t bloque_inicial, int32_t bloques_a_liberar)
{
    log_info(logger_debug, "Se deben liberar %d bloques desde el bloque inicial: %d", bloques_a_liberar, bloque_inicial);
    for (int32_t i = 0; i < bloques_a_liberar; ++i)
    {
        // log_info(logger_debug, "Se libera 1 bloque del bitmap");
        bitarray_clean_bit(bitmap_bloques, bloque_inicial+i);
    }
    msync(bitmap_bloques->bitarray, BLOCK_COUNT/8, MS_SYNC);
}

bool asignar_n_bloques(int32_t bloque_inicial, int32_t bloques_a_asignar)
{
    int32_t bloques_disponibles = 0;
    // log_trace(logger_debug, "Ultimo_bloque: %d", bloque_inicial);
    for (int32_t i = 1; i <= bloques_a_asignar; ++i)
    {
        if (bitarray_test_bit(bitmap_bloques, bloque_inicial+i))
            {
                // log_warning(logger_debug, "El bit: %d esta ocupado", bloque_inicial+i);
                break;
            }
        else 
            {bloques_disponibles++;}
    }
    // log_trace(logger_debug, "disponibles: %d", bloques_disponibles);
    if (bloques_disponibles<bloques_a_asignar) 
        {
        log_debug(logger_debug, "El archivo necesitaba %d bloques desde la posicion siguiente a %d y encontro %d bloques", bloques_a_asignar, bloque_inicial, bloques_disponibles);
        return false;
        }
    else
    {
        for (int32_t i = 1; i <= bloques_a_asignar; i++)
        {
            bitarray_set_bit(bitmap_bloques, bloque_inicial+i);
        }
        msync(bitmap_bloques->bitarray, BLOCK_COUNT/8, MS_SYNC);
        return true;
    }
}

bool reasignar_bloques(t_config* metadata, int32_t cant_bloques, int32_t nueva_cant_bloques)
{
    int32_t bloques_disponibles = 0;
    int32_t nuevo_inicio = 0;

    for (int32_t i = 0; i < BLOCK_COUNT; i++)
    {
        if (!bitarray_test_bit(bitmap_bloques, i))
        {
            ++bloques_disponibles;
            if (bloques_disponibles>=nueva_cant_bloques)
                {break;}
        }
        else {
            bloques_disponibles=0;
            nuevo_inicio=i+1;
        }
    }
    if (bloques_disponibles < nueva_cant_bloques)
    {
        log_info(logger_debug, "No hay suficiente espacio para asignar al archivo");
        return false;
    }
    else
    {
        int32_t bloque_inicial = config_get_int_value(metadata, "BLOQUE_INICIAL");

        void* contenido = malloc(cant_bloques*BLOCK_SIZE);
        FS_READ(bloques, bloque_inicial, 0, cant_bloques*BLOCK_SIZE, contenido);
        FS_WRITE(bloques, nuevo_inicio, 0, cant_bloques*BLOCK_SIZE, contenido);
        free(contenido);

        for (int32_t i = 0; i < cant_bloques; ++i)
        {
            bitarray_clean_bit(bitmap_bloques, bloque_inicial+i);
        }
        char* itoa = string_itoa(nuevo_inicio);
        config_set_value(metadata, "BLOQUE_INICIAL", itoa);
        config_save(metadata);
        free(itoa);

        for (int32_t i = 0; i < nueva_cant_bloques; ++i)
        {
            bitarray_set_bit(bitmap_bloques, nuevo_inicio+i);
        }
        msync(bitmap_bloques->bitarray, BLOCK_COUNT/8, MS_SYNC);
        log_info(logger_debug, "Se reasigna el archivo a la nueva posicion: %d, con: %d bloques", nuevo_inicio, nueva_cant_bloques);
        return true;
    }
}

void FS_WRITE(void* bloques, uint32_t bloque_inicial, uint32_t puntero, uint32_t tamanio_total, char* datos_a_escribir)
{
    uint32_t inicio_escritura = (bloque_inicial*BLOCK_SIZE) + puntero;
    memcpy(bloques+inicio_escritura, datos_a_escribir, tamanio_total);
    msync(bloques, BLOCK_SIZE*BLOCK_COUNT, MS_SYNC);
    log_info(logger, "Escritura exitosa");
}

void FS_READ(void* bloques, uint32_t bloque_inicial, uint32_t puntero, uint32_t tamanio_total, void* datos_leidos)
{
    uint32_t inicio_lectura = (bloque_inicial*BLOCK_SIZE) + puntero;
    memcpy(datos_leidos, bloques+inicio_lectura, tamanio_total);
    // msync(bloques, BLOCK_SIZE*BLOCK_COUNT, MS_SYNC);
    log_info(logger, "Lectura exitosa");
}

int32_t compactacion(uint32_t PID, char* nombre_archivo, uint32_t nueva_cant_bloques)
{
    log_info(logger, "PID: %u - Inicio Compactación.", PID);
    limpiar_bitmap();
    // log_debug(logger_debug, "Luego de limpiar el bitmap quedan: %d bloques disponibles", contar_bloques_libres());

    void* nuevos_bloques = malloc(BLOCK_COUNT*BLOCK_SIZE);
    if (nuevos_bloques==NULL)
    {
        log_error(logger_debug, "Error al asignar memoria para nuevos bloques");
        exit(1);
    }
    memset(nuevos_bloques, 0, BLOCK_COUNT*BLOCK_SIZE); 

    struct dirent *de;// Puntero a la entrada (archivo) del directorio

    DIR *dr = opendir(path_metadata);// Puntero al directorio
    char* archivo_actual;
    if (dr == NULL)
    {
        log_error(logger_debug, "No se pudo abrir el directorio" );
    }

    int32_t nuevo_inicio;

    while ((de = readdir(dr)) != NULL){
        archivo_actual = de->d_name;
        if (string_contains(archivo_actual, ".txt") && !string_equals_ignore_case(archivo_actual, nombre_archivo))
        {
            nuevo_inicio = compactar_archivo(archivo_actual, nuevos_bloques);
            log_trace(logger_debug, "Archivo: %s - nuevo inicio: %d", archivo_actual, nuevo_inicio);
        }
    }
    closedir(dr);

    nuevo_inicio = compactar_archivo(nombre_archivo, nuevos_bloques);// El archivo que se quiere truncar se ubica al final de los bloques
    
    memcpy(bloques, nuevos_bloques, BLOCK_COUNT*BLOCK_SIZE);
    if (msync(bloques, BLOCK_SIZE*BLOCK_COUNT, MS_SYNC) == -1)
    {
        log_error(logger_debug, "Error al sincronizar la memoria con el archivo");
    }

    log_debug(logger_debug, "Esperando retraso compactacion");
    usleep(RETRASO_COMPACTACION*1000);
    log_info(logger, "PID: %u - Fin Compactación.", PID);
    free(nuevos_bloques);
    return nuevo_inicio;
}

void limpiar_bitmap()
{
    for (int32_t i = 0; i < BLOCK_COUNT; i++)
    {
        bitarray_clean_bit(bitmap_bloques, i);
    }
    msync(bitmap_bloques->bitarray, BLOCK_COUNT/8, MS_SYNC);
    // char* dump = mem_hexstring(bitmap_bloques, BLOCK_COUNT/8);
    // log_error(logger_debug, "bitmap luego de limpieza: %s", dump);
}

int32_t compactar_archivo(char* nombre_archivo, void* nuevos_bloques)
{
    log_trace(logger_debug, "Proximo archivo a ser compactado: %s", nombre_archivo);
    char* path_archivo_metadata = string_duplicate(path_metadata);
    string_append(&path_archivo_metadata, nombre_archivo);
    t_config* metadata_c = config_create(path_archivo_metadata);
    
    int32_t tamanio_archivo = config_get_int_value(metadata_c, "TAMANIO_ARCHIVO");
    int32_t bloque_inicial = config_get_int_value(metadata_c, "BLOQUE_INICIAL");
    int32_t cant_bloques = cantidad_de_bloques(tamanio_archivo);
    int32_t nuevo_inicio = buscar_bloque_libre();

    log_trace(logger_debug, "Nuevo inicio del archivo: %d, cant bloques: %d", nuevo_inicio, cant_bloques);
    // Se copian los datos en un contenedor, luego se los graba en un nuevo puntero de bloques (el cambio se hace en la funcion compactar)
    void* contenido = malloc(cant_bloques*BLOCK_SIZE);
    FS_READ(bloques, bloque_inicial, 0, cant_bloques*BLOCK_SIZE, contenido);
    FS_WRITE(nuevos_bloques, nuevo_inicio, 0, cant_bloques*BLOCK_SIZE, contenido);
    
    asignar_bloques_compactacion(nuevo_inicio, cant_bloques);
    
    char* itoa = string_itoa(nuevo_inicio);

    config_set_value(metadata_c, "BLOQUE_INICIAL", itoa);
    config_save_in_file(metadata_c, path_archivo_metadata);
    config_destroy(metadata_c);
    free(itoa);
    free(path_archivo_metadata);
    free(contenido);

    return nuevo_inicio;
}

void asignar_bloques_compactacion(int32_t nuevo_inicio, int32_t cant_bloques)
{
    for (int32_t i = 1; i < cant_bloques; i++)
    {
        bitarray_set_bit(bitmap_bloques, nuevo_inicio+i);
    }
    msync(bitmap_bloques->bitarray, BLOCK_COUNT/8, MS_SYNC);
}