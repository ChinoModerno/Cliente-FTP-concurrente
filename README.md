# Cliente FTP Concurrente

**Autor:** ReyesL  
**Archivo fuente:** `ReyesL-clienteFTP.c`

## Descripción

Este proyecto implementa un cliente FTP concurrente basado en el estándar RFC 959. El programa permite realizar transferencias de archivos (subidas y bajadas) y listar directorios en segundo plano (background), permitiendo al usuario seguir interactuando con la consola de comandos mientras la transferencia ocurre.

## Archivos del Proyecto

- `ReyesL-clienteFTP.c`: Código fuente completo (versión monolítica)
- `Makefile`: Archivo de automatización de compilación
- `README.md`: Documentación del proyecto

## Requisitos

- Compilador GCC
- Entorno Linux o Unix-like (macOS, WSL)

## Instrucciones de Compilación

Para compilar el proyecto, abra una terminal en la carpeta y ejecute:

-make

Para limpiar los archivos generados:

-make clean


## Instrucciones de Ejecución
Sintaxis:
./ReyesL-clienteFTP <IP_SERVIDOR> [PUERTO]


Ejemplos:
-Servidor local (puerto 21): ./ReyesL-clienteFTP localhost

-Servidor externo: ./ReyesL-clienteFTP 192.168.1.50


## Comandos FTP Soportados
-user <usuario>: Enviar usuario.
-pass <clave>: Enviar contraseña.
-dir: Listar archivos (Concurrente).
-get <archivo>: Descargar archivo (Concurrente).
-put <archivo>: Subir archivo (Concurrente).
-cd <dir>: Cambiar directorio.
-mkd <dir>: Crear directorio.
-pwd: Ver directorio actual.
-quit: Salir.
