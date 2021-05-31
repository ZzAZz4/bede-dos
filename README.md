# bede-dos: proyecto
literalmente bede-dos

## integrantes

- Esteban Villacorta  201910336
- Jean Paul Huby Tuesta 201910194
- Daniela Abril Vento Bustamante 201910331

## Introduccion

### objetivos

El objetivo del proyecto tiene como objetivo implementar y analizar dos estructuras de datos para el uso de base de datos en el disco. Dinamic Hash y Sequential File.

Los operaciones que se implementaran en nuestras estructuras son:
- Busqueda puntual
- Insercion
- Eliminacion

### Descripcion de Dominios de datos a user
Para testear las base de datos usaremos un archivo csv con datos de la vida real. Los datos que usamos se obtuvieron de [Red Wine Quality](https://www.kaggle.com/uciml/red-wine-quality-cortez-et-al-2009). todos los atributos de la base de datos son valores numericos, por lo que nuestra base de datos guardara records de fixed-lenght.

### Resultados esperados
Se espera que al comprar las distintas operaciones con las estructuras propuestas. Hash tengo una mayor ventaja sobre el sequential al en todas operaciones.

## Fundamente y describa las técnicas




### File pointer
Es una estructura de datos que simula el comportamiento de un puntero para archivos, guarda el path del archivo y la posicion donde se encuentra dentro de este. Se usara esta estructura dentro de del hash y el sequential ,para poder simplificar el codigo y mejorar su redabilidad.

### Dinamic hash

#### insertion
consiste de buscar el puntero que contiene la key que buscamos dentro del index file. Si el bucket con una llave, si este bucket se encuentra lleno. creamos otro con una operacion split, la cual  crea un hash key distinto y distribuimos los elementos ahi. De caso contrario insertamos el hash en el bucket original.

#### search
consiste de buscar el puntero que contiene la key que buscamos dentro del index file. Obtenemos el bucket usando el file pointer, recorremos por todos los record dentro del bucket y comparamos jey por key. 


#### delete
buscamos dentro del el puntero que contiene la key que buscamos dentro del index file. Si el bucket con una llave, buscamos el registro con la key dentro del bucket, si lo encuentra lo elimina. Si el bucket se encuentra vacio despues de la eliminacion, eliminamos esa key dentro del hash table.

### Sequential file


#### insertion

#### search

#### delete


## Resultados Experimentales


## Pruebas de uso y presentación


[Explicación del proyecto](https://www.youtube.com/watch?v=dQw4w9WgXcQ)
[Pruebas funcionales](https://www.youtube.com/watch?v=Jui-Td1HbXI)
