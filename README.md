# Cliente FTP Concurrente

**Autor**: ReyesL  
**Archivo principal**: `ReyesL-clienteFTP.c`

## Descripción

Este proyecto implementa un cliente FTP concurrente basado en el estándar RFC 959. El programa utiliza procesos (fork) para gestionar las transferencias de datos en segundo plano, permitiendo mantener activo el canal de control para nuevos comandos.

## Estructura del Proyecto

El ejecutable se construye enlazando el código principal con las librerías de conexión proporcionadas en clase:

- **`ReyesL-clienteFTP.c`**: Lógica principal del cliente (Manejo de comandos, concurrencia y UI).

### Librerías de soporte (Requeridas para compilar):

- `connectTCP.c` / `connectsock.c`: Gestión de conexiones salientes.
- `passiveTCP.c` / `passivesock.c`: Gestión de sockets pasivos (si fueran necesarios).
- `errexit.c`: Gestión de errores.

## Instrucciones de Compilación

1. Asegúrese de que todos los archivos `.c` mencionados arriba estén en la misma carpeta.

2. Ejecute el siguiente comando para compilar todo el conjunto:

```bash
make
```

Esto generará el binario `ReyesL-clienteFTP`.

## Instrucciones de Ejecución

```bash
./ReyesL-clienteFTP <HOST> [PUERTO]
```

### Ejemplo:

```bash
./ReyesL-clienteFTP localhost
```

## Funcionalidades

- **Concurrencia**: Soporte para transferencias simultáneas (GET/PUT/LIST) sin bloquear el prompt.
- **Comandos implementados**: 
  - `USER` - Autenticación de usuario
  - `PASS` - Contraseña del usuario
  - `LIST` - Listar archivos del directorio actual
  - `RETR` - Descargar archivo (GET)
  - `STOR` - Subir archivo (PUT)
  - `CWD` - Cambiar directorio de trabajo
  - `MKD` - Crear directorio
  - `PWD` - Mostrar directorio actual
  - `QUIT` - Salir del cliente FTP

## Características Técnicas

- Implementación basada en el estándar RFC 959 para FTP
- Arquitectura concurrente utilizando procesos (fork)
- Gestión separada de canal de control y canal de datos
- Interfaz de usuario interactiva que permanece responsive durante transferencias