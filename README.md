# abismo // próximo

##### GettingData 
Proyecto desarrollado para comunicar el sensor neuronal muse a otros programas a traves del protocolo OSC

##### Funcionalidad requerida
- Envío de múltiples datos vía OSC,
- Conexión con dos sensores MUSE simultáneos, 

##### ToDo list
* Posibilidad de enviar múltiples datos OSC de manera simultánea al activar las casillas
... De momento solo envía un dato a la vez, *nos falta hacerlo simultáneo* al seleccionar las casillas.

* Capacidad para conectar dos sensores

* Enviar datos a dos direcciones IP y dos puertos (Funciona solo a localhost)

##### Pruebas realizadas y sus resultados
* Prueba de envío a múltiple IP y puerto
... Funciona correctamente enviando al localhost, *al intentar enviar a otro equipo no funciona* (El envío con la misma conexión fue probada con processing dando un resultado positivo).
... La aplicación funciona al seleccionar una nueva IP y al quitarla


##### Bugs detectados
* Al iniciar la aplicación y dar click en connect sin el dispositivo Bluetooth conectado arroja una excepcion, sería bueno evitar ese break.

##### Colaboradores:
Desarrollado a partir del SDK libMuse for Windows.
* Eduardo H Obieta
* Rodrigo Torres. Xumo.
