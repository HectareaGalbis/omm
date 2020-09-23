# omm (Open Multi-Methods)
Este repositorio consta de un único fichero que nos da la opción de usar open multi-methods en C++. Jean-Louis Leroy explica de maravilla en su repositorio [yomm2](https://github.com/jll63/yomm2) el por qué es necesario este tipo de herramienta, aquí escribiré un resumen.

## El problema de expresión y el multiple dispatch
En muchas ocasiones creamos clases que deben usarse mutuamente y nos encontramos con diversas opciones válidas de implementar una funcionalidad. Supón el caso de una librería con matrices. Existen numerosos tipos de matrices y nos gustaría aprovechar al máximo las características de cada uno para optimizar las operaciones que hagamos con ellas. Y es que no es lo mismo sumar dos matrices de manera 'cotidiana' (sumar cada uno de los valores de ambas matrices) que sumar dos matrices diagonales (sumar únicamente los valores de las diagonales). Otro problema a tratar es en qué clase implementamos cada método. ¿Implementamos el método suma en la clase matriz_diagonal y que reciba un argumento de tipo matriz_invertible, o lo implementamos en la clase matriz_invertible y que reciba un argumento de tipo matriz_diagonal? Todo esto se puede solucionar con los open multi-methods. Ahí va un ejemplo de cómo implementaríamos la función suma para las matrices:

```C++
class Matriz{...};

class Diagonal{...};
class Invertible{...};
class Ortogonal{...};

Matriz suma(virtual Matriz* m1, virtual Matrix* m2);   // <-- Plantilla de la función suma. Usamos virtual para indicar qué 
                                                     //     parametros son los que deseamos especializar

Matriz suma(Diagonal* d1, Diagonal* d2){
  // Implementacion de la suma de dos matrices ortogonales
}

Matriz suma(Ortogonal* o, Diagonal* d){
  // Implementacion de la suma de una matriz ortogonal y una matriz diagonal
}

Matriz suma(Invertible* i, Ortogonal* o){
  // Implementacion de la suma de una matriz invertible y una matriz ortogonal 
}

// ...

int main(){

  Matriz* m1 = new Diagonal();
  Matriz* m2 = new Ortogonal();

  std::cout << matrix_to_string(suma(m1,m2)) << std::endl;

}
```
Observa que gracias a los open multi-methods estamos resolviendo también el problema del multiple dispatch. Además hay que comentar que se admiten cualquier cantidad de argumentos virtuales y no virtuales, y sin un orden en concreto.

## Open multi-methods en C++
No hay. Así que hay que usar librerías alternativas.

## ¿yomm2 ó omm?
Las principales diferencias son las siguientes:
* yomm2 lleva mucho tiempo actualizándose y mejorándose, mientras que omm acaba de salir del horno. Si vas a usar este tipo de soluciones en un programa donde los fallos pueden ser fatales usa yomm2.
* yomm2 usa C++17. omm usa C++20.
* yomm2 depende de 3 librerías de Boost. omm no depende de nada.
* yomm2 crea sus tablas de punteros a funciones en tiempo de ejecución. omm las crea en tiempo de compilación.
* yomm2 usa una forma natural de implementar los métodos, como en el ejemplo de las matrices de arriba. omm es feillo y artificioso.
Como verás cada librería tiene sus ventajas e inconvenientes. Usa la que más te convenga.

## Usando omm




