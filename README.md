# omm (Open Multi-Methods)
Este repositorio consta de un único fichero que nos da la opción de usar open multi-methods en C++. Jean-Louis Leroy explica de maravilla en su repositorio [yomm2](https://github.com/jll63/yomm2) el por qué es necesario este tipo de herramienta, aquí escribiré un resumen.

## El problema de expresión
En muchas ocasiones creamos clases que deben usarse mutuamente y nos encontramos con diversas opciones válidas de implementar una funcionalidad. Supón el caso de una librería con matrices. Existen numerosos tipos de matrices y nos gustaría aprovechar al máximo las características de cada uno para optimizar las operaciones que hagamos con ellas. Y es que no es lo mismo sumar dos matrices de manera 'cotidiana' (sumar cada uno de los valores de ambas matrices) que sumar dos matrices diagonales (sumar únicamente los valores de las diagonales). Otro problema a tratar es en qué clase implementamos cada método. ¿Implementamos el método suma en la clase matriz_diagonal y que reciba un argumento de tipo matriz_invertible, o lo implementamos en la clase matriz_invertible y que reciba un argumento de tipo matriz_diagonal? Todo esto se puede solucionar con los open multi-methods. Ahí va un ejemplo de cómo implementaríamos la función suma para las matrices:

```C++
class Matriz{...};

class Diagonal : public Matriz{...};
class Invertible : public Matriz{...};
class Ortogonal : public Matriz{...};

Matriz* suma(virtual Matriz* m1, virtual Matrix* m2);   // <-- Plantilla de la función suma. Usamos virtual para indicar qué 
                                                     //     parametros son los que deseamos especializar

Matriz* suma(Diagonal* d1, Diagonal* d2){
  // Implementacion de la suma de dos matrices ortogonales
}

Matriz* suma(Ortogonal* o, Diagonal* d){
  // Implementacion de la suma de una matriz ortogonal y una matriz diagonal
}

Matriz* suma(Invertible* i, Ortogonal* o){
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
* yomm2 depende de 3 librerías de Boost. omm no depende de nada.
* yomm2 crea sus tablas de punteros a funciones en tiempo de ejecución. omm las crea en tiempo de compilación.
* yomm2 usa una forma natural de implementar los métodos, como en el ejemplo de las matrices de arriba. omm es feillo y artificioso.

Como verás cada librería tiene sus ventajas e inconvenientes. Usa la que más te convenga.

## Usando omm
Como ejemplo usaremos las matrices. Para cada método y sus especializaciones necesitamos crear una tabla, una `table_omm`. Esta tabla necesita 3 ingredientes, una plantilla para indicar que parámetros son virtuales y cuáles no, una clase que contenga las especializaciones del método, y todas las clases que van a participar en la elección de la especialización correcta.

### Ingrediente 1, la plantilla
La plantilla es el tipo de función que tenemos pensado especializar. En este caso queremos sumar dos matrices y devolver el resultado, por lo que el tipo de función será la siguiente:
```C++
using suma_template = Matriz*(Virtual<Matriz*>,Virtual<Matriz*>);
```
Observa que usamos `Virtual` para indicar que cada parámetro, en este caso un puntero a matriz, se va a sustituir en cada especialización por la clase hija correspondiente.

### Ingrediente 2, las especializaciones dentro de una clase o struct
De alguna forma tenemos que indicar a `table_omm` donde están las funciones que debe usar. Para ello le pasaremos una clase que contenga cada especialización. La visibilidad de cada método debe ser pública para que `table_omm` pueda acceder, por lo que recomiendo usar `struct` en lugar de `class`. Asegúrate también de que cada especialización tenga el modificador `static` para poder acceder a ellas sin tener que crear ningún objeto.
```C++
struct suma_matrices{ // <-- Asegúrate de tener visibilidad pública, por eso recomiendo usar struct

  static Matriz* suma(Diagonal* d1, Diagonal* d2){    // <-- Recuerda usar static en cada especialización
    // Implementacion de la suma de dos matrices ortogonales
  }
  
  static Matriz* suma(Ortogonal* o, Diagonal* d){     // <-- Recuerda usar static en cada especialización
    // Implementacion de la suma de una matriz ortogonal y una matriz diagonal
  }
  
  static Matriz* suma(Invertible* i, Ortogonal* o){   // <-- Recuerda usar static en cada especialización
    // Implementacion de la suma de una matriz invertible y una matriz ortogonal 
  }
  
  //...
  
}
```

### Ingrediente 3, las clases hijas que especializan el método
En este caso estamos usando 3 clases hijas de Matriz, que son `Diagonal`,`Ortogonal` e `Invertible`. 

### Cocinando la tabla
Usamos los 3 ingredientes anteriores en el siguiente orden:
1. El struct con las especializaciones
2. La plantilla
3. Las clases hijas
```C++
using suma_table = table_omm<suma_matrices,suma_template,Diagonal,Ortogonal,Invertible>;
```

### Un último paso, acceder a la tabla
La tabla contiene un método `call` que al recibir los argumentos busca en la tabla de funciones y llama a la especialización correcta. La mejor opción es usarla dentro de la esperada función `suma`:
```C++
Matriz* suma(Matriz* m1, Matriz* m2){ // <-- Asegúrate de que el tipo de retorno y el tipo de los argumentos sea igual a la plantilla
  return suma_table::call(m1,m2);     // <-- Llamamos al método call de table_omm pasándole todos los argumentos
}
```

## Conclusión
Mediante esta librería podemos usar open multi-methods aunque sea de una forma un poco fea por culpa de los templates. Aun así, el poder resolver el problema de expresión, y sobre todo el problema del multiple dispatch sin ningún tipo de intrusión es algo que merece la atención de cualquiera. 


