La práctica consiste en desarrollar un Cliente UDP que permita hacer uso del servicio TFTP. El
cliente se implementará en el fichero tftp-client.c utilizando el lenguaje C y se iniciará de
la siguiente forma desde la terminal de comandos:

tftp-client ip_servidor {-r|-w} archivo [-v]

Donde:
  - ip_servidor: dirección IP del servidor TFTP.
  - {-r|-w}: indica que se desea leer o escribir un fichero del servidor.
  - archivo: nombre del fichero.
  - -v (opcional): cliente informa de los paquetes enviados para la lectura/escritura del fichero
