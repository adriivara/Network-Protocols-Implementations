#include "ip-icmp-ping.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#define MAX_ARGS 3

EchoRequest request;
EchoReply reply;

void fillICMPRequest();
unsigned short checksum(unsigned short *pointer, int size);
void printErrMsg(unsigned char type,unsigned char code);

int main(int argc, char **argv){

    int socketfd;
    int info=0;
    struct sockaddr_in myaddr, serverAddr;
    struct in_addr addr;
    socklen_t addrlen = sizeof(serverAddr);
    
    // Comprobacion del numero de argumentos correcto
    if(argc!= MAX_ARGS-1 && argc != MAX_ARGS){
        fprintf(stderr, "Error. Numero de argumentos inválido\n");
        exit(EXIT_FAILURE);
    }

    // Transformacion de la direccion IP de cadena a struct
    if(inet_aton(argv[1], &addr) != 1){
        perror("Error al convertir la IP");
        exit(EXIT_FAILURE);
    }

    // Si -v se introduce, activa la flag para dar información
    if((argc == MAX_ARGS) && (strcmp(argv[2], "-v") == 0)){
        info = 1;
    }

    // Creacion del socket RAW
    if((socketfd=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP))<0){
        perror("Error creando el socket");
        exit(EXIT_FAILURE);
    }

    // Configuracion de mi dirección ip
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = 0;
    myaddr.sin_addr.s_addr = INADDR_ANY;

    // Configuracion de la ip del servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = 0;
    serverAddr.sin_addr = addr;

    // Asociacion del socket con mi dirección ip
    if(bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) != 0){
        perror("Error en el bind");
        exit(EXIT_FAILURE);
    }

    fillICMPRequest();

    if(info){
        printf("-> Generando cabecera ICMP...\n");
        printf("-> Type: %hu\n", request.icmpHdr.type);
        printf("-> Code: %hu\n", request.icmpHdr.code);
        printf("-> PID : %hu\n", request.pid);
        printf("-> Sequence Number: %hu\n", request.sequence);
        printf("-> Cadena a enviar: %s\n", request.payload);
        printf("-> Checksum: %hx\n", request.icmpHdr.checksum);
        printf("-> Tamaño total del datagrama: %lu\n", sizeof(request));
    }
    
    // Envio del datagrama ICMP al servidor
    if(sendto(socketfd, &request, sizeof(request), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
        perror("Error enviando el ping");
        exit(EXIT_FAILURE);
    }

    printf("Mensaje ICMP enviado al host : %s\n", argv[1]);

    // Recepción del datagrama reply del servidor
    if(recvfrom(socketfd, &reply, sizeof(reply), 0, (struct sockaddr *)&serverAddr, &addrlen) < 0){
        perror("Error recibiendo la respuesta");
        exit(EXIT_FAILURE);
    }

    printf("\nRespuesta recibida desde : %s\n", argv[1]);

    printErrMsg(reply.icmpMsg.icmpHdr.type, reply.icmpMsg.icmpHdr.code);

    // Comprobar de vuelta el checksum para ver la integridad del mensaje
    if(checksum((unsigned short*)&request, (int)sizeof(request)/2) != 0){
        fprintf(stderr, "Error, el checksum no es 0\n");
        exit(EXIT_FAILURE);
    }

    if(info){
        printf("Tamaño de la respuesta: %lu\n", sizeof(reply));
        printf("Cadena recibida: %s\n", reply.icmpMsg.payload);
        printf("PID: %hu\n", reply.icmpMsg.pid);
        printf("TTL: %hu\n", reply.ipHdr.TTL);
    }

    printf("Respuesta correcta (Type 0, Code 0)\n");


    exit(EXIT_SUCCESS);

}

// Funcion que se encarga de inicializar el request con los valores para un ping
void fillICMPRequest(){

    char *payload = "Este es el payload";

    request.icmpHdr.type =(unsigned char) 8;
    request.icmpHdr.code = (unsigned char) 0;
    request.icmpHdr.checksum=(unsigned char) 0;
    request.pid=(unsigned short) getpid();
    request.sequence=0;
    strcpy(request.payload, payload);

    request.icmpHdr.checksum=checksum((unsigned short*)&request, (int)sizeof(request)/2);

}

// Funcion que calcula el checksum de un datagrama. El primer arguemento es un puntero al datagrama y el segundo su tamaño entre 2
unsigned short checksum(unsigned short *pointer, int size){
    int i = 0;
    unsigned int acum = 0;

    for(i=0; i<size; i++){
        acum += (unsigned short) *pointer;
        pointer++;
    }
    acum = (acum >> 16) + (acum & 0x0000ffff);
	acum = (acum >> 16) + (acum & 0x0000ffff);
	return ~(acum & 0x0000ffff);
}

// Funcion que imprime el mensaje de error si hay un error. El primer argumento es el tipo y el segundo el codigo
void printErrMsg(unsigned char type,unsigned char code){
	switch(type){
        case 0:
            break;
		case 3:
			fprintf(stderr,"Destination Unreachable: ");
			switch(code){
				case 0:
					fprintf(stderr,"Net Unreachable (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 1:
					fprintf(stderr,"Host Unreachable (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 2:
					fprintf(stderr,"Protocol Unreachable (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 3:
					fprintf(stderr,"Port Unreachable (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 4:
					fprintf(stderr,"Fragmentation Needed (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 5:
					fprintf(stderr,"Source Route Failed (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 6:
					fprintf(stderr,"Destination Network Unknown (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 7:
					fprintf(stderr,"Destination Host Unknown (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 8:
					fprintf(stderr,"Source Host Isolated (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 11:
					fprintf(stderr,"Destination Network Unreachable for Type of Service (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 12:
					fprintf(stderr,"Destination Host Unreachable for Type of Service (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 13:
					fprintf(stderr,"Communication Administratively Prohibited (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 14:
					fprintf(stderr,"Host Precedence Violation (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 15:
					fprintf(stderr,"Precedence Cutoff in Effect (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
			}
            
		case 5:
			fprintf(stderr,"Redirect: ");
			switch(code){
				case 1:
					fprintf(stderr,"Redirect for Destination Host (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 3:
					fprintf(stderr,"Redirect for Destination Host Based on Type-of-Service (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
			}

		case 11:
			fprintf(stderr,"Time Exceeded: ");
			switch(code){
				case 0:
					fprintf(stderr,"Time-to-Live Exceeded in Transit (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 1:
					fprintf(stderr,"Fragment Reasssembly Time Exceeded (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
			}

		case 12:
			fprintf(stderr,"Parameter Problem: ");
			switch(code){
				case 0:
					fprintf(stderr,"Pointer indecates the error (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 1:
					fprintf(stderr,"Missing a Required Option (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
				case 2:
					fprintf(stderr,"Bad Length (Type: %hhu, Code %hhu\n", type, code);
    					exit(EXIT_FAILURE);
			}

	}
}