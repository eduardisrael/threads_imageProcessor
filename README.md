
Threads

./procesador ruta_fuente ruta_destino -n <# hilos>

Las imágenes están representadas por una cuadrilla de puntos llamadas pixeles. Por ejemplo, una imagen de 1920x1080 tiene 1080 filas de 1920 pixeles cada uno

Cada pixel se represente con 4 bytes, cada uno de los cuales son los canales RGBA (red, green, blue y alpha). Las combinaciones de estos 32 bits, da como resultado el color del pixel y su transparencia.
El programa hace uso de la librería libpng.so para leer la información de la imagen. El programa lee una imagen a color (ruta_fuente), y calcula la imagen de escala de grises, y la guarda en ruta_destino. La librería representa la imagen como un arreglo de arreglos de tipo png_bytep.

La imagen de escala de grises solo usa 2 bytes por pixel (1 para el tono de gris y el otro para la transparencia)
La función procesar_archivo_png recorre la imagen fila por fila, y aplica la formula a cada pixel de la fila.

la opción -n # que permitirá especificar el numero de hilos a crear. Para dividir el trabajo entre los hilos, a cada hilo le asignará un entero de 0 hasta n -1. Entonces si hay n hilos, cada hilo procesará:
El hilo 0 procesará las filas 0, n, 2n, ...
El hilo 1 procesará las filas 1, 1 + n, 1 + 2n, ...
El hilo n – 1 procesará las filas n -1, n – 1 + n, n – 1 + 2n, ...

Tiempo de ejecución del programa
crea los hilos de procesamiento hasta que todos terminen
