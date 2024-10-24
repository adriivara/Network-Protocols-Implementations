#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <errno.h>

#define MAX_ARGS 3
#define PORT 5
#define MAX_BUFFER 80

int serverSocket;

// Función que modifica la cadena para luego mandarla al cliente
void tratarCadena(char* cadena);

// Funcion que cierra la conexión y socket del servidor y libera el puerto cuando se pulsa Ctrl+c
void signal_handler(int signal);

int main(int argc, char** argv){

	int pid, conexionSocket;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t addr_size;
	unsigned short puerto;
	uint16_t puerto_network;
	char cadena[80];

	// Registramos las señales que se puedan hacer, si se hace Ctrl+c se llama a la funcion signal_handler
	signal(SIGINT,signal_handler);

	// Comprobar que el numero de argumentos sea el correcto
	if(argc != MAX_ARGS-2 && argc != MAX_ARGS){
		fprintf(stderr, "Error, numero incorrecto de args\n");
		exit(EXIT_FAILURE);
	}

	// Si el puerto no está especificado usa el 5 por defecto
	if(argc == MAX_ARGS-2){
		puerto = PORT;
	}
	// Usa el puerto que se le especifique por parámetros y comprueba que se haya puesto de forma correcta
	else{
		if(strcmp("-p", argv[1])!=0){
			fprintf(stderr, "Primer argumento invalido, prueba con -p\n");
			exit(EXIT_FAILURE);
		}
		if(sscanf(argv[2], "%hu", &puerto) != 1){
			fprintf(stderr, "Error al convertir la cadena a entero\n");
			exit(EXIT_FAILURE);
		}
	}

	puerto_network = htons(puerto);
	
	// Creacion del socket TCP
	if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Error creando el socket");
		exit(EXIT_FAILURE);
	}

	// Configuracion de la direccion ip del servidor junto al puerto servidor
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = puerto_network;
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	// Asociacion del socket con la direccion ip del servidor para poder recibir y enviar paquetes
	if(bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0){
		perror("Error en el bind");
		exit(EXIT_FAILURE);
	}

	addr_size = sizeof(clientAddr);

	// Creacion de una cola de espera de 40 para conexiones simultaneas
	if(listen(serverSocket, 40) < 0){
		perror("Error creando cola de conexiones");
		exit(EXIT_FAILURE);
	}

	printf("Esperando mensajes....\n");

	while(1) {
		
		// Espera de conexion. Cuando una solicitud de conexión llega, se acepta y se crea un socket para atender a esa conexion
		if((conexionSocket=accept(serverSocket, (struct sockaddr*)&clientAddr, &addr_size)) < 0){
			perror("Error aceptando una conexion");
			exit(EXIT_FAILURE);
		}

		// Creacion de un proceso hijo
		if((pid=fork()) < 0){
			perror("Error creando un proceso hijo");
			exit(EXIT_FAILURE);
		}

		// Trabajo que tiene que hacer el proceso hijo
		if(pid==0){
			// Recepcion del mensaje enviado por el cliente
			if(recv(conexionSocket, cadena, MAX_BUFFER, 0) < 0){
				perror("Error recibiendo el mensaje");
				exit(EXIT_FAILURE);
			}

			printf("\nMensaje recibido: %s\n", cadena);

			tratarCadena(cadena);

			printf("Mensaje a enviar: %s\n", cadena);

			// Envio del mensaje modificado al cliente
			if(send(conexionSocket, cadena, MAX_BUFFER, 0) < 0){
				perror("Error enviando el mensaje");
				exit(EXIT_FAILURE);
			}

			// Cierre del socket de conexion con el cliente
			if(shutdown(conexionSocket, SHUT_RDWR) != 0){
				perror("Error cerrando la conexion");
        		exit(EXIT_FAILURE);
			}
			close(conexionSocket);

			exit(0);

		}
	}

	exit(0);
}

void tratarCadena(char* cadena){
	int i;

	// Cambiar mayusculas por minusculas y viceversa
	for(i=0; i < strlen(cadena); i++){
		if((cadena[i] > 64) && (cadena[i] < 91)){
			cadena[i] = tolower(cadena[i]);
		}

        else if((cadena[i] > 96) && (cadena[i] < 123)){
			    cadena[i] = toupper(cadena[i]);
        }
	}
}

void signal_handler(int signal){

	// Cierra el socket del servidor para que este se libere
	printf("\nCerrando servidor...\n");
	if(shutdown(serverSocket, SHUT_RDWR) != 0){
		perror("Error cerrando la conexion");
        exit(EXIT_FAILURE);
	}
	close(serverSocket);
	exit(0);
}