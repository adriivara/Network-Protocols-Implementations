La práctica consiste en desarrollar un cliente llamado miping que haga uso de datagramas ICMP para
reproducir el comportamiento del servicio ping. El código se implementará en el fichero miping.c
utilizando el lenguaje C y se ejecutará desde la terminal de comandos tal como se muestra a continuación:

./miping direccion_ip [-v]

Donde:
  - direccion_ip: dirección IPv4 de la máquina a la que se hace ping.
  - -v (opcional): permite informar de los pasos realizados (cada uno irá precedido por: ->). Además, el
  - programa deberá mostrar el valor del campo TTL de la cabecera IP del datagrama ICMP recibido.
