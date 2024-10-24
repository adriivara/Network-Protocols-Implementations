// Practica Tema 7: Vara Lamua, Adrian

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_ARGS 5

int info = 0;
char buffer[516];

void lectura(char* fileName, int socketfd, struct sockaddr_in serverAddr, socklen_t addrlen, char* ip);
void escritura(char *fileName, int socketfd, struct sockaddr_in serverAddr, socklen_t addrlen, char* ip);
void rellenarRequest(int r, char *fileName);

int main(int argc, char **argv){

    int readOp;
    int socketfd;
    struct sockaddr_in myaddr, serverAddr;
    struct in_addr addr;
    struct servent *service;
    socklen_t addrlen = sizeof(serverAddr);

    readOp = 1;

    // Comprobacion del numero de argumentos correcto
    if(argc != MAX_ARGS-1 && argc != MAX_ARGS){
        fprintf(stderr, "Error, numero incorrecto de argumentos\n");
        exit(EXIT_FAILURE);
    }

    // Transformacion de la direccion IP de cadena a struct
    if(inet_aton(argv[1], &addr) != 1){
        perror("Error al convertir la IP");
        exit(EXIT_FAILURE);
    }

    // Comprobación si segundo argumento es correcto
    if((strcmp(argv[2], "-r") != 0) && (strcmp(argv[2], "-w") != 0)){
        fprintf(stderr, "Error, segundo argumento invalido: Prueba con -r o -w\n");
        exit(EXIT_FAILURE);
    }

    // Si es operación de escritura, cambio en la flag de lectura a 0, por defecto está a 1
    if(strcmp(argv[2], "-w") == 0){
        readOp = 0;
    }

    // Si -v se introduce, activa la flag para dar información
    if((argc == MAX_ARGS) && (strcmp(argv[4], "-v") == 0)){
        info = 1;
    }

    // Creacion del socket udp
    if((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Error creando el socket");
        exit(EXIT_FAILURE);
    }

    // Configuracion de la direccion del cliente
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = 0;
    myaddr.sin_addr.s_addr = INADDR_ANY;

    // Asociacion del socket con la direccion ip del cliente
    if(bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) != 0){
        perror("Error en el bind");
        exit(EXIT_FAILURE);
    }

    // Obtencion de la informacion del servicio tftp
    if((service = getservbyname("tftp", "udp")) == NULL){
        perror("Error obteniendo el servicio");
        exit(EXIT_FAILURE);
    }

    // Configuracion de la direccion del servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = service->s_port;
    serverAddr.sin_addr = addr;

    // Operación de lectura
    if(readOp == 1){
        lectura(argv[3], socketfd, serverAddr, addrlen, argv[1]);
    }
    // Operación de escritura
    else{
        escritura(argv[3], socketfd, serverAddr, addrlen, argv[1]);
    }

    exit(0);
}

// Funcion que implementa la operación de lectura del cliente
void lectura(char *fileName, int socketfd, struct sockaddr_in serverAddr, socklen_t addrlen, char* ip){
    FILE *file;
    unsigned short blockN, blockN2;
    int numBytesRecv;

    // Invocación de la función que rellena el request de lectura con los bytes correspondientes
    rellenarRequest(1, fileName);

    // Envío de la request de lectura al servidor
    if(sendto(socketfd, buffer, 516, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
        perror("Error enviando el mensaje");
        exit(EXIT_FAILURE);
    }

    if(info){
        printf("Enviada solicitud de lectura de \"%s\" a servidor tftp (%s)\n", fileName, ip);
    }

    if((file = fopen(fileName, "wb")) == NULL){
        perror("Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    do
    {
        // Recepción del datagrama del servidor
        if((numBytesRecv = recvfrom(socketfd, buffer, 516, 0, (struct sockaddr *)&serverAddr, &addrlen)) < 0){
            perror("Error recibiendo el mensaje");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Comprobación de si la respuesta enviada por el servidor es un error o no
        if(buffer[1] == 5){
            fprintf(stderr, "Error: Errcode %c%c: %s\n", buffer[2], buffer[3], buffer+4);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Calculo del número de bloque que es
        blockN = ((unsigned char)buffer[2])*256 + (unsigned char)buffer[3];

        // Imprimir información
        if(info){
            printf("Recibido bloque del servidor tftp\n");
            printf("Es el bloque numero %d\n", blockN);
        }

        // Comprobación si los paquetes llegan en orden
        if(blockN != blockN2+1){
            fprintf(stderr, "Error. Los paquetes no se han recibido en orden\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        blockN2 = blockN;

        // Escritura de los datos recibido en el fichero especificado
        if(fwrite(buffer+4, sizeof(char), numBytesRecv-4, file) < 1){
            perror("Error escribiendo en el archivo");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Cambio de la cabecera del paquete para que envíe un ACK
        buffer[1] = 4;

        // Envío del ACK al servidor
        if(sendto(socketfd, buffer, 4, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
            perror("Error enviando el mensaje");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        if(info){
            printf("Enviamos el ACK del bloque %d\n", blockN);
        }
            
    } while(numBytesRecv-4 == 512);

    if(info){
        printf("Ese era el ultimo bloque: Cerramos el fichero\n");
    }

    fclose(file);
}

// Funcion que implementa la operación de escritura del cliente
void escritura(char *fileName, int socketfd, struct sockaddr_in serverAddr, socklen_t addrlen, char* ip){
    FILE *file;
    unsigned short blockN, blockN2=0;
    int numBytesSent;

    // Invocación de la función que rellena el request de escritura con los bytes correspondientes
    rellenarRequest(2, fileName);

    // Envío de la request de lectura al servidor
    if(sendto(socketfd, buffer, 516, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
        perror("Error enviando el mensaje");
        exit(EXIT_FAILURE);
    }

    if(info){
        printf("Enviada solicitud de escritura de \"%s\" a servidor tftp (%s)\n", fileName, ip);
    }

    if((file = fopen(fileName, "rb")) == NULL){
        perror("Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    // Recepción del ACK del servidor
    if(recvfrom(socketfd, buffer, 516, 0, (struct sockaddr *)&serverAddr, &addrlen) < 0){
        perror("Error recibiendo el mensaje");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Comprobación de si la respuesta enviada por el servidor es un error o no
    if(buffer[1] == 5){
        fprintf(stderr, "Error: Errcode %c%c: %s\n", buffer[2], buffer[3], buffer+4);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    do{
        // Calculo del número de bloque que es
        blockN = ((unsigned char)buffer[2])*256 + (unsigned char)buffer[3];

        // Comprobación si los paquetes llegan en orden
        if(blockN++ != blockN2++){
            fprintf(stderr, "Error. Los paquetes no se han recibido en orden\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Escritura de la cabecera del datagrama de datos
        buffer[0] = 0;
        buffer[1] = 3;
        buffer[2] = blockN/256;
        buffer[3] = blockN%256;

        // Escribimos los datos del fichero en el datagrama. Como mucho 512 bytes
        if((numBytesSent = fread(buffer+4, sizeof(char), 512, file)) < 0){
            perror("Error leyendo el fichero");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Envío del datagrama de datos
        if(sendto(socketfd, buffer, numBytesSent+4, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
            perror("Error enviando el mensaje");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        if(info){
            printf("Bloque numero %d enviado\n", blockN);
        }

        // Recepción del ACK del servidor
        if(recvfrom(socketfd, buffer, 516, 0, (struct sockaddr *)&serverAddr, &addrlen) < 0){
            perror("Error recibiendo el mensaje");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Comprobación de si la respuesta enviada por el servidor es un error o no
        if(buffer[1] == 5){
            fprintf(stderr, "Error: Errcode %c%c: %s\n", buffer[2], buffer[3], buffer+4);
            fclose(file);
            exit(EXIT_FAILURE);
        }

    } while(numBytesSent == 512);

    if(info){
        printf("Ese era el ultimo bloque: Cerramos el fichero\n");
    }

    fclose(file);    
}

/* Funcion que escribe el datagrama de Request de lectura o escritura dependiendo de sus parámetros
    El primer argumento es un entero que dependiendo de si es 1 o 2 será un request de lectura o escritura
    El segundo argumento es una cadena que contiene el nombre del fichero que se quiere leer o escribir
*/
void rellenarRequest(int r, char *fileName){
    buffer[0] = 0;
    buffer[1] = r;

    strcpy(buffer+2, fileName);
    buffer[2+strlen(fileName)] = 0;
    strcpy(buffer+3+strlen(fileName), "octet\0");
}
