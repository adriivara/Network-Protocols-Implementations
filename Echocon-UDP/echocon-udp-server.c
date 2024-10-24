#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#define MAX_ARGS 3
#define PORT "5"

int main(int argc, char** argv){

	int serverSocket, i;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t addr_size;
	unsigned short puerto;
	uint16_t puerto_network;
	char cadena[80];

	if(argc != MAX_ARGS-2 && argc != MAX_ARGS){
		fprintf(stderr, "Error, numero incorrecto de args\n");
		exit(1);
	}

	if(argc == MAX_ARGS-2){
		if(sscanf(PORT, "%hu", &puerto) != 1){
			fprintf(stderr, "Error al convertir la cadena a entero\n");
			exit(1);
		}
	}
	else{
		if(strcmp("-p", argv[1])!=0){
			fprintf(stderr, "Primer argumento invalido, prueba con -p\n");
			exit(1);
		}
		if(sscanf(argv[2], "%hu", &puerto) != 1){
			fprintf(stderr, "Error al convertir la cadena a entero\n");
			exit(1);
		}
	}

	puerto_network = htons(puerto);
	
	//Creacion del socket UDP
	if((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "Error creando el socket\n");
		exit(1);
	}

	//Configuracion de la direccion del servidor
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = puerto_network;
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	//Asociacion del socket con la direccion del servidor
	if(bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0){
		fprintf(stderr, "Error en bind\n");
		exit(1);
	}

	printf("Esperando datagramas....\n");

	while(1) {
		addr_size = sizeof(clientAddr);

		//Recepcion de datagramas del cliente
		if(recvfrom(serverSocket, cadena, 80, 0, (struct sockaddr *)&clientAddr, &addr_size) < 0){
			fprintf(stderr, "Error recibiendo el mensaje\n");
			exit(1);
		}

		printf("Mensaje recibido: %s\n", cadena);

		//Cambiar mayusculas por minusculas y viceversa
		for(i=0; i < strlen(cadena); i++){
			if((cadena[i] > 64) && (cadena[i] < 91)){
				cadena[i] = tolower(cadena[i]);
			}

            else{
			    if((cadena[i] > 96) && (cadena[i] < 123)){
				    cadena[i] = toupper(cadena[i]);
                }
			}
		}
		
		printf("Mensaje a enviar: %s\n", cadena);

		//Envio del datagrama
		sendto(serverSocket, cadena, strlen(cadena), 0, (struct sockaddr *)&clientAddr, addr_size);

		printf("Mensaje enviado correctamente\n");
	}

	//Cerrar el socket
	close(serverSocket);

	exit(0);
}
