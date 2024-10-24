#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_ARGS 5
#define PORT 5
#define MAX_BUFFER 80

int main(int argc, char** argv){

    int clientSocket;
    struct sockaddr_in clientAddr, serverAddr;
    struct in_addr addr;
    socklen_t addr_size = sizeof(serverAddr);
    unsigned short puerto;
    uint16_t puerto_network;
    char* cadena;

    // Comprobacion del numero de argumentos
    if(argc != MAX_ARGS-2 && argc != MAX_ARGS){
        fprintf(stderr, "Error, numero incorrecto de argumentos\n");
        exit(EXIT_FAILURE);
    }

    // Caso para puerto no especificado, por defecto puerto 5
    if(argc == MAX_ARGS-2){
        if(inet_aton(argv[1], &addr) != 1){
            perror("Error al convertir la IP");
            exit(EXIT_FAILURE);
        }
        puerto_network = htons(PORT);
        puerto = PORT;
        cadena = argv[2];
    }

    // Caso para puerto especificado
    else{
        if(inet_aton(argv[1], &addr) != 1){
            perror("Error al convertir la IP");
            exit(EXIT_FAILURE);
        }
        if(strcmp("-p", argv[2]) != 0){
            perror("Segundo argumento invalido, prueba con -p");
            exit(EXIT_FAILURE);
        }
        if(sscanf(argv[3], "%hu", &puerto) != 1){
            perror("Error al convertir la cadena a entero");
            exit(EXIT_FAILURE);
        }
        puerto_network = htons(puerto);
        cadena = argv[4];
    }

    // Creacion del socket TCP
    if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error creando el socket");
        exit(EXIT_FAILURE);
    }

    // Configuracion de la direccion IP del cliente
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = 0;
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    // Asociacion del socket con la direccion IP del cliente
    if(bind(clientSocket, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) != 0){
        perror("Error en el bind");
        exit(EXIT_FAILURE);
    }

    // Configuracion de la direccion IP del servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = puerto_network;
    serverAddr.sin_addr = addr;

    // Creacion de una conexion virtual con el servidor
    if(connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size) < 0){
        perror("Error conectando con servidor");
        exit(EXIT_FAILURE);
    }

    // Envio de la cadena al servidor
    if(send(clientSocket, cadena, MAX_BUFFER, 0) < 0){
        perror("Error enviando el mensaje");
        exit(EXIT_FAILURE);
    }

    printf("Envio del mensaje \"%s\" a direccion %s y puerto %d\n", cadena, argv[1], puerto);
    printf("Esperando respuesta....\n");

    // Espera de la respuesta del servidor
    if(recv(clientSocket, cadena, MAX_BUFFER, 0) < 0){
        perror("Error recibiendo el mensaje");
        exit(EXIT_FAILURE);
    }

    printf("Mensaje recibido del servidor: %s\n", cadena);

    // Cierre de conexion con el servidor
    if(shutdown(clientSocket, SHUT_RDWR) != 0){
        perror("Error cerrando la conexion");
        exit(EXIT_FAILURE);
    }

    // Cerrar socket TCP
    close(clientSocket);

    exit(0);

}
