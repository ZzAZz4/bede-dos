# bede-dos: proyecto
literalmente bede-dos

## Integrantes

- Esteban Villacorta  201910336
- Jean Paul Huby Tuesta 201910194
- Daniela Abril Vento Bustamante 201910331

## Introducción

### Objetivos

El objetivo de este proyecto es implementar y analizar dos estructuras de datos para el uso de base de datos en el disco: Dynamic Hash y Sequential File.

Los operaciones que se implementaran en nuestras estructuras son:
- Búsqueda puntual
- Inserción
- Eliminación

### Descripción de Dominios de datos a user
Para testear las base de datos usaremos un archivo CSV con datos de la vida real. Los datos que usamos se obtuvieron de [Red Wine Quality](https://www.kaggle.com/uciml/red-wine-quality-cortez-et-al-2009). Todos los atributos de la base de datos son valores numéricos, por lo que nuestra base de datos guardara records de fixed-length.

### Resultados esperados
Se espera que al comprar las distintas operaciones con las estructuras propuestas, hash tengo una mayor ventaja sobre el sequential al en todas operaciones.

## Fundamente y describa las técnicas

### File pointer
Es una estructura de datos que simula el comportamiento de un puntero para archivos, guarda el path del archivo y la posición donde se encuentra dentro de este. Se usará esta estructura dentro de del hash y el sequential para poder simplificar el codigo y mejorar su redibilidad.

### Dynamic hash (Extendible Hashing)

La estructura se separa en dos componentes principales:
- La función hash, que evalua la posicion de cada registro en la estructura.
- El **bucket**, que mantiene registros de forma secuencial sin ningun orden. Todos los buckets en un file evaluan de forma igual para la funcion hash. Todos los buckets tienen un tamaño máximo desde su creación. Cada bucket tiene ademas un miembro **depth** que determina su profundidad local.
- El **directorio** (directory), que mantiene el indice con los punteros a todos los buckets creados. El directory tambien tiene un miembro **depth**, que determina la profundidad global.

Existen varias propiedades para la estructura:
- Todos los buckets tienen el mismo tamaño máximo, que debe ser constante. Ningun bucket puede tener más registros que lo permitido por su tamaño máximo.
- Ningun bucket puede tener una depth mayor a la depth del directorio.
- No se repiten las keys.
- Un bucket con prefijo 1 solo puede existir si existe un bucket analogo con prefijo 0.

#### Insertion
Se utiliza la funcion hash para mappear el registro a insertar al bucket correspondiente. Luego, se busca secuencialmente en el bucket para chequear que no haya otro elemento con la misma key. Si no existía, se inserta. En ese caso, nos fijamos si este se encuentra lleno o no. Si se encuentra lleno, se chequea que se mantenga la segunda propiedad. Si no lo hace, se duplica el tamaño máximo del array de indices del directorio y se aumenta en 1 la depth del directorio. Luego, se divide el bucket en dos buckets con depth incrementada en 1, y se distribuyen las keys hasta que ya no hayan violaciones a las dos propiedades.

#### Search
Consiste de buscar el registro que contiene la key que buscamos dentro del index file. Obtenemos el bucket usando la funcion hash. Una vez obtenido, se itera secuencialmente entre todos los registros. Si se encuentra un registro con key igual al input, se retorna el registro. Caso contrario, no se retorna nada.

#### Search por rango
Se itera sobre todos los registros en todos los buckets, y se retorna todos los que se tienen una key que se encuentra en el rango especificado.

#### Delete
Se utiliza la funcion hash para mappear el registro a insertar al bucket correspondiente. Luego, se busca el elemento en el bucket. Si existe, se intercambia con el ultimo y se poppea. En ese caso, nos fijamos si este se encuentra vacio o no. Si se encuentra vacio, se mergea al bucket analogo. Si el merge lleva a que no haya ningun bucket con la misma depth que el directorio, se divide el tamaño de la estructura.


### Sequential File
Este se apoya de:
- un **índice (main)** donde se ubican registros ordenados físicamente por un campo, y
- un archive **auxiliar (aux)** en el que se inserta el próximo espacio disponible y utiliza punteros para mantener el orden del archivo principal.
Para la implementación que se está realizando, la cabecera de esta estructura cuenta con punteros cuales apuntan a la posición inicial de los arrays que representan a ambos “archivos”, así como sus respectivos tamaños.

Propiedades importantes por recalcar del sequential file incluyen que: 
- Para todos los nodos del main file, se mantiene el orden físico entre un elemento y el siguiente no eliminado.
- Para todos los nodos, el siguiente nodo de acuerdo al miembro **next** mantiene el orden entre un elemento y el siguiente.
- Para cualquier nodo, puede tener un nodo next en el aux file si es que la siguiente entrada no esta elmiminada. Caso contrario, el registro vacío es reemplazado por el siguiente registro.
- El aux file no está ordenado físicamente, y funciona como una heap. Sin embargo, el orden por next igual se mantiene. 
- Una cadena en el aux debe terminar en otro nodo del main file.

#### Insert

> Para insertar un registro, debe localizarse la posición en la que será insertado el nuevo registro. Si el espacio está libre (es el dato que sigue secuencialmente o se ha eliminado un registro y queda un espacio para colocar el nuevo dato), insertarlo directamente. Caso contrario, insertarlo en un espacio adicional (usualmente en un archivo aparte auxiliar, del tamaño de un buffer, que almacena los nuevos valores a insertar como un heap) y actualizar los punteros para saber quién sigue.

Comienza iterando el vector del índice para buscar su posible ubicación. Luego considera cinco casos:
- Si el elemento **va al final del array**, lo inserta.
- Si el elemento **solía existir**, pero ha sido eliminado, lo añade en esa posición.
- Si el elemento **ya existe**, no lo inserta.
- Si el elemento **va al inicio**, lo inserta.
- Caso contrario (**va en alguna posición intermedia**), intenta colocarlo en la posición anterior.

#### Search

> Se realiza una búsqueda binaria O(log N). Se modifica tomando la posición inicial 0 y la longitud de archivo (considerando que se trabaja con registros de longitud fija no debería haber problema calculando esto). Se lee el registro en la posición del medio, se obtiene su key, se compara y se va buscando.

Se aplica una búsqueda binaria en el índice. Esto puede darse debido a que los datos están ordenados de manera física. 

#### Delete

> Para la eliminación, podemos apoyarnos de los punteros y usarlos para saltar las tuplas equivocadas. También se pueden colocar marcadores. Ha de fijarse en el anterior y seguir buscando desde ahí. Después copiar el puntero al que apunta la posición anterior a la que va el archivo y actualizar el puntero de la posición anterior al con la del nuevo registro. El paso a seguir es la reconstrucción del archivo considerando el orden de los punteros. Se ordenan en la RAM y se procede a eliminar el archivo auxiliar.

Primero se ubica la posición del nodo a eliminar. Dependiendo de esta posición se toman acciones diferentes:
- Si **está al inicio**, simplemente se marca como eliminado.
- Si **está al final**, ir a la posición anterior y continuar el array.
- Si **está en alguna posición al medio**, cambiar los punteros. Reestructurar/mover punteros si es necesario.
- Si es que **está en el auxiliar**, eliminarlo, manejando los punteros que se relacionan con este. Reducir el tamaño del archivo auxiliar.



## Resultados Experimentales

A continuación se ofrece un poco de justificación detrás de los resultados evidenciados en los resultados, los cuales se observan en las gráficas.

Para ver los resultados completos, referirse al siguiente [spreadsheet](https://docs.google.com/spreadsheets/d/1D8Vs-A2jXmPx6BBgiAn1sSYjnHuEIwvE4Q-v3OXPK5I/edit?usp=sharing). 

### Dynamic hash

#### Insert

O(1). Se emplean punteros para la inserción.

#### Find

O(1 + colisión)). Si bien se mapean los elementos, es probable que se produzcan colisiones, cuales generalmente van al archivo auxiliar.

#### Delete

O(1). Similar al insert, se ubica la posición a borrar empleando punteros.

### Sequential file

#### Insert

O(1). Solo se realiza una inserción directa empleando la posición física del registro.

#### Find

O(log n). Aplica búsqueda binaria, operación cuyo costo es el indicado.

#### Delete

O(1). Solo se está removiendo el elemento indicado empleando punteros.

## Pruebas de uso y presentación

- [Explicación del proyecto](https://www.youtube.com/watch?v=dQw4w9WgXcQ)
- [Pruebas funcionales](https://www.youtube.com/watch?v=Jui-Td1HbXI)
