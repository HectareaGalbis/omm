# omm (Open Multi-Methods)
Este repositorio consta de un único fichero que nos da la opción de usar open multi-methods en C++. Jean-Louis Leroy explica de maravilla en su repositorio [yomm2](https://github.com/jll63/yomm2) el por qué es necesario este tipo de herramienta, aquí escribiré un resumen.

## El problema de expresión
En muchas ocasiones creamos clases que deben usarse mutuamente y nos encontramos con diversas opciones válidas de implementar una funcionalidad. Supón el caso de una librería con matrices. Existen numerosos tipos de matrices y nos gustaría aprovechar al máximo las características de cada uno para optimizar las operaciones que hagamos con ellas. Y es que no es lo mismo sumar dos matrices de manera 'cotidiana' (sumar cada uno de los valores de ambas matrices) que sumar dos matrices diagonales (sumar únicamente los valores de las diagonales). Otro problema a tratar es en qué clase implementamos cada método. ¿Implementamos el método suma en la clase matriz_diagonal y que reciba un argumento de tipo matriz_invertible, o lo implementamos en la clase matriz_invertible y que reciba un argumento de tipo matriz_diagonal? Todo esto se puede solucionar con los open multi-methods. Ahí va un ejemplo de cómo implementaríamos la función suma para las matrices:

```C++

```
