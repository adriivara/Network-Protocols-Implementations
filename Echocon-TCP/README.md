La práctica consiste en desarrollar un cliente y un servidor (iterativo) que permita hacer uso de un nuevo servicio utilizando sockets TCP. El nuevo servicio, denominado EchoCon, añade una nueva funcionalidad al servicio estándar echo. En este caso, el servidor recibirá una cadena de caracteres, realizará la conversión de mayúsculas a minúsculas (y viceversa) y devolverá el resultado al cliente. Dado que se trata de un servicio no estándar, no aparece listado en el fichero /etc/services. Por tanto, se le asociará el puerto: 5/tcp.

El cliente, implementado en el fichero echocon-tcp-client.c, se iniciará desde la terminal de comandos de la siguiente forma:

./echocon-tcp-client ip_servidor [-p puerto_servidor] cadena

Siendo ip_servidor la ip del servidor al que se lo queremos mandar, puerto_servidor el puerto tcp por donde se lo quieres mandar y cadena, la cadena de caracteres que le quieres mandar al servidor.

El servidor, implementado en el fichero echocon-tcp-server.c, se iniciará del siguiente modo:

./echocon-tcp-server-apellidos [-p puerto-servidor]

Si no se indica el número de puerto, tanto el cliente como el servidor utilizarán por defecto el 5.
