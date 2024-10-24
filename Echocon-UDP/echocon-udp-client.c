// Practica Tema 5: Vara Lamua, Adrian

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_ARGS 5
#define PORT "5"

int main(int argc, char** argv){

    int clientSocket;
    struct sockaddr_in clientAddr, serverAddr;
    struct in_addr addr;
    socklen_t addr_size = sizeof(serverAddr);
    unsigned short puerto;
    uint16_t puerto_network;
    char* cadena;

    //Comprobacion del numero de argumentos
    if(argc != MAX_ARGS-2 && argc != MAX_ARGS){
        fprintf(stderr, "Error, numero incorrecto de argumentos\n");
        exit(1);
    }

    //Caso para puerto no especificado
    if(argc == MAX_ARGS-2){
        if(inet_aton(argv[1], &addr) != 1){
            fprintf(stderr, "Error al convertir la IP\n");
            exit(1);
        }
        if(sscanf(PORT, "%hu", &puerto) != 1){
            fprintf(stderr, "Error al convertir la cadena a entero\n");
            exit(1);
        }
        puerto_network = htons(puerto);
        cadena = argv[2];
    }

    //Caso para puerto especificado
    else{
        if(inet_aton(argv[1], &addr) != 1){
            fprintf(stderr, "Error al convertir la IP\n");
            exit(1);
        }
        if(strcmp("-p", argv[2]) != 0){
            fprintf(stderr, "Segundo argumento invalido, prueba con -p\n");
            exit(1);
        }
        if(sscanf(argv[3], "%hu", &puerto) != 1){
            fprintf(stderr, "Error al convertir la cadena a entero\n");
            exit(1);
        }
        puerto_network = htons(puerto);
        cadena = argv[4];
    }

    //Creacion del socket udp
    if((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        fprintf(stderr, "Error creando el socket\n");
        exit(1);
    }

    //Configuracion de la direccion del cliente
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = 0;
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    //Asociacion del socket con la direccion ip del cliente
    if(bind(clientSocket, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) != 0){
        fprintf(stderr, "Error en el bind\n");
        exit(1);
    }

    //Configuracion de la direccion del servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = puerto_network;
    serverAddr.sin_addr = addr;

    //Envio de la cadena al servidor
    if(sendto(clientSocket, cadena, 80, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
        fprintf(stderr, "Error enviando el mensaje\n");
        exit(1);
    }

    printf("Envio del mensaje \"%s\" a direccion %s y puerto %d\n", cadena, argv[1], puerto);
    printf("Esperando respuesta....\n");

    //Espera de la respuesta del servidor
    if(recvfrom(clientSocket, cadena, 80, 0, (struct sockaddr *)&serverAddr, &addr_size) < 0){
        fprintf(stderr, "Error recibiendo el mensaje\n");
        exit(1);
    }

    printf("Mensaje recibido del servidor: %s\n", cadena);

    //Cerrar socket UDP
    close(clientSocket);

    exit(0);

}
