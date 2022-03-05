# omm (Open Multi-Methods)
This project is an unique file which offers open multi-methods. I was inspired by the library made by Jean-Louis Leroy named [yomm2](https://github.com/jll63/yomm2).

## Why omm?
The best features of omm are:
* It has no dependencies. You only need a C++17 compiler.
* omm creates the necessary tables of pointers in compile time. So, no runtime overhead creating the tables.
* omm offers template open multi-methods. See [below] for more information. 

## A simple tutorial
As an example, we will use matrices. For each method and their implementations we need to create a table, an 'omm table'. This table needs 3 ingredients, a template telling what the 'virtual types' are, a struct containint the implementations of the method, and all the classes that participate in the selection of the correct implementation once the method is called. 

First, consider the following matrix classes:

```
class Matrix{
  virtual ~Matrix(){}     //<---- The base class must have a virtual method.
  //...
};

class Diagonal : public Matrix{
  //...
}

class Orthogonal : public Matrix}{
  //...
}

class Invertible : public Matrix{
  //...
}
```

Note that the base class must have a virtual method. In fact, every class that could appear in the template (see the next section) must be polymorphic, in other words, must have at least a virtual method.

### 1st ingredient, the template
The template is the function signature of the open multi-methods we are creating. In this case we want to add two matrices and return the result. We need to indicate this signature using 'WithSignature':

```C++
using add_template = WithSignature<Matrix*(Virtual<Matrix*>,Virtual<Matrix*>)>;
```

This signature is telling that the open multi-method will receive two pointers to objects of type Matrix or some og their daughter classes (Diagonal, Orthogonal or Invertible), and it will return a pointer to a Matrix. Note that we wrote `Virtual<Matriz*>` to indicate the parameters could be a derived class of Matrix. Only the parameters could be `Virtual` and these types can appear in any order. A more complex example could involve `Virtual` and non-`Virtual` types:

```C++
using complex_template = WithSignature<int(Virtual<BaseClass*>,int,float,Virtual<BaseClass2&>,Virtual<const BaseClass1&>,char)>
```

As you can see, `Virtual` types can be pointer or reference, and they can have cv-qualifiers too. 

### 2nd ingredient, the implementations
De alguna forma tenemos que indicar a `table_omm` donde están las funciones que debe usar. Para ello le pasaremos una clase que contenga cada especialización. La visibilidad de cada método debe ser pública para que `table_omm` pueda acceder, por lo que recomiendo usar `struct` en lugar de `class`. Asegúrate también de que cada especialización tenga el modificador `static` para poder acceder a ellas sin tener que crear ningún objeto. Por último, el nombre las especializaciones debe llamarse implementation.
```C++
struct suma_matrices{ // <-- Asegúrate de tener visibilidad pública, por eso recomiendo usar struct

  static Matriz* implementation(Diagonal* d1, Diagonal* d2){    // <-- Recuerda usar static en cada especialización
    // Implementacion de la suma de dos matrices ortogonales
  }
  
  static Matriz* implementation(Ortogonal* o, Diagonal* d){     // <-- Recuerda usar static en cada especialización
    // Implementacion de la suma de una matriz ortogonal y una matriz diagonal
  }
  
  static Matriz* implementation(Invertible* i, Ortogonal* o){   // <-- Recuerda usar static en cada especialización
    // Implementacion de la suma de una matriz invertible y una matriz ortogonal 
  }
  
  //...
  
}

using struct_implementations = WithImplementations<suma_matrices>;
```
Observa que utilizamos `WithImplementations` para indicar el struct donde están las implementaciones.

### Ingrediente 3, las clases hijas que especializan el método
En este caso estamos usando 3 clases hijas de Matriz, que son `Diagonal`,`Ortogonal` e `Invertible`. 
```
using clases_hijas = WithDerivedTypes<Diagonal,Ortogonal,Invertible>;
```

### Cocinando la tabla
Usamos los 3 ingredientes anteriores en el siguiente orden:
1. El struct con las especializaciones
2. La plantilla
3. Las clases hijas
```C++
using suma_table = table_omm<struct_implementations,suma_template,clases_hijas>;
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


