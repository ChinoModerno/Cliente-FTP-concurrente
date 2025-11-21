#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

extern int  errno;

int  errexit(const char *format, ...);
int  connectTCP(const char *host, const char *service);
int  passiveTCP(const char *service, int qlen);

#define  LINELEN    128
#define  BUFSIZE    4096

/* Manejador de señal para evitar procesos zombies */
void sigchld_handler(int s) {
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

/* Envia los comandos FTP al servidor */
void sendCmd(int s, char *cmd) {
  int n;
  n = strlen(cmd);
  cmd[n] = '\r';
  cmd[n+1] = '\n';
  write(s, cmd, n+2);
}

/* Lee y muestra respuesta del servidor */
int getReply(int s, char *res) {
    int n = read(s, res, LINELEN);
    if (n > 0) {
        res[n] = '\0';
        printf("%s", res); // El servidor ya suele enviar \n
        return (res[0] - '0') * 100 + (res[1] - '0') * 10 + (res[2] - '0');
    }
    return 0;
}

/* Configura modo PASV y retorna el socket de datos conectado */
int pasivo(int s) {
  int sdata;
  int nport;
  char cmd[128], res[128], *p;
  char host[64], port[8];
  int h1,h2,h3,h4,p1,p2;

  sprintf(cmd, "PASV");
  sendCmd(s, cmd);
  
  // Debemos leer la respuesta de PASV bloqueando momentáneamente
  int n = read(s, res, LINELEN);
  res[n] = '\0';
  printf("%s", res);

  p = strchr(res, '(');
  if (p == NULL) return -1;
  
  sscanf(p+1, "%d,%d,%d,%d,%d,%d", &h1,&h2,&h3,&h4,&p1,&p2);
  snprintf(host, 64, "%d.%d.%d.%d", h1,h2,h3,h4);
  nport = p1*256 + p2;
  snprintf(port, 8, "%d", nport);
  
  // Llamada a connectTCP (definida externamente)
  sdata = connectTCP(host, port);
  return sdata;
}

void ayuda() {
  printf("\nCliente FTP Concurrente. Comandos:\n"
    "  dir           - Lista directorio (concurrente)\n"
    "  get <archivo> - Descarga archivo (concurrente)\n"
    "  put <archivo> - Sube archivo (concurrente)\n"
    "  cd <dir>      - Cambia directorio\n"
    "  pwd           - Muestra directorio actual\n"
    "  mkd <dir>     - Crea directorio\n"
    "  quit          - Salir\n\n");
}

int main(int argc, char *argv[]) {
  char  *host = "localhost";
  char  *service = "ftp";
  char  cmd[128], res[LINELEN+1];
  char  user[32], *pass;
  char  input_buf[128];
  char  *ucmd, *arg;
  int   s, sdata, n, code;
  
  fd_set readfds;
  int    maxfd;

  // Manejo de hijos muertos
  struct sigaction sa;
  sa.sa_handler = sigchld_handler; 
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
      perror("sigaction");
      exit(1);
  }

  switch (argc) {
    case 1: host = "localhost"; break;
    case 3: service = argv[2]; 
    case 2: host = argv[1]; break;
    default: fprintf(stderr, "Uso: ReyesL-clienteFTP [host [port]]\n"); exit(1);
  }

  s = connectTCP(host, service);
  getReply(s, res); 

  // LOGIN
  while (1) {
    printf("User: ");
    scanf("%s", user);
    sprintf(cmd, "USER %s", user);
    sendCmd(s, cmd);
    getReply(s, res);

    pass = getpass("Password: ");
    sprintf(cmd, "PASS %s", pass);
    sendCmd(s, cmd);
    code = getReply(s, res);
    
    if (code == 230) break; 
    printf("Login fallido, intente de nuevo.\n");
  }

  // Limpiar buffer de stdin después del scanf/getpass
  fgets(input_buf, sizeof(input_buf), stdin); 
  ayuda();
  printf("ftp> ");
  fflush(stdout);

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(0, &readfds); // stdin (usuario)
    FD_SET(s, &readfds); // socket control (servidor)
    maxfd = s;

    // Esperar actividad en teclado o socket
    if (select(maxfd + 1, &readfds, NULL, NULL, NULL) == -1) {
        if (errno == EINTR) continue;
        perror("select");
        break;
    }

    // 1. MENSAJES DEL SERVIDOR (Asíncronos)
    if (FD_ISSET(s, &readfds)) {
        n = read(s, res, LINELEN);
        if (n <= 0) {
            printf("\nConexión cerrada por el servidor.\n");
            break;
        }
        res[n] = '\0';
        printf("\n%sftp> ", res); 
        fflush(stdout);
    }

    // 2. INPUT DEL USUARIO
    if (FD_ISSET(0, &readfds)) {
        if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) break;
        input_buf[strcspn(input_buf, "\n")] = 0;
        
        if (strlen(input_buf) == 0) {
            printf("ftp> ");
            fflush(stdout);
            continue;
        }

        ucmd = strtok(input_buf, " ");
        arg = strtok(NULL, " ");

        if (strcmp(ucmd, "quit") == 0) {
            sprintf(cmd, "QUIT");
            sendCmd(s, cmd);
            break;

        } else if (strcmp(ucmd, "cd") == 0) {
            if(arg) sprintf(cmd, "CWD %s", arg);
            else continue;
            sendCmd(s, cmd); 

        } else if (strcmp(ucmd, "pwd") == 0) {
            sprintf(cmd, "PWD");
            sendCmd(s, cmd);

        } else if (strcmp(ucmd, "mkd") == 0) {
            if(arg) sprintf(cmd, "MKD %s", arg);
            else { printf("Falta argumento\n"); printf("ftp> "); fflush(stdout); continue; }
            sendCmd(s, cmd);

        } else if (strcmp(ucmd, "dir") == 0) {
            sdata = pasivo(s); 
            if (sdata < 0) continue;

            sprintf(cmd, "LIST");
            sendCmd(s, cmd);
            // Sincronización inicial necesaria
            getReply(s, res); 

            if (fork() == 0) { 
                close(s); 
                char data[BUFSIZE];
                while ((n = read(sdata, data, BUFSIZE)) > 0) {
                    write(1, data, n); 
                }
                close(sdata);
                exit(0);
            }
            // PADRE
            close(sdata); 
            printf("Transferencia iniciada en background...\n");

        } else if (strcmp(ucmd, "get") == 0) {
            if (!arg) { printf("Falta nombre de archivo\n"); printf("ftp> "); fflush(stdout); continue; }
            
            sdata = pasivo(s);
            if (sdata < 0) continue;

            sprintf(cmd, "RETR %s", arg);
            sendCmd(s, cmd);
            getReply(s, res);

            if (fork() == 0) { // HIJO
                close(s);
                FILE *fp = fopen(arg, "wb");
                if (!fp) { perror("fopen"); exit(1); }
                
                char data[BUFSIZE];
                while ((n = recv(sdata, data, BUFSIZE, 0)) > 0) {
                    fwrite(data, 1, n, fp);
                }
                fclose(fp);
                close(sdata);
                exit(0);
            }
            close(sdata);
            printf("Descargando %s en background...\n", arg);

        } else if (strcmp(ucmd, "put") == 0) {
            if (!arg) { printf("Falta nombre de archivo\n"); printf("ftp> "); fflush(stdout); continue; }
            
            FILE *test = fopen(arg, "r");
            if (!test) { perror("Archivo local no encontrado"); printf("ftp> "); fflush(stdout); continue; }
            fclose(test);

            sdata = pasivo(s);
            if (sdata < 0) continue;

            sprintf(cmd, "STOR %s", arg);
            sendCmd(s, cmd);
            getReply(s, res);

            if (fork() == 0) { 
                close(s);
                FILE *fp = fopen(arg, "rb");
                char data[BUFSIZE];
                while ((n = fread(data, 1, BUFSIZE, fp)) > 0) {
                    send(sdata, data, n, 0);
                }
                fclose(fp);
                close(sdata);
                exit(0);
            }
            close(sdata);
            printf("Subiendo %s en background...\n", arg);

        } else if (strcmp(ucmd, "help") == 0) {
            ayuda();
        } else {
            printf("Comando desconocido.\n");
        }
        
        printf("ftp> ");
        fflush(stdout);
    }
  }
  close(s);
  return 0;
}