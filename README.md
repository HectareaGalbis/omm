# omm (Open Multi-Methods)
This project is an unique file which offers open multi-methods. I was inspired by the library made by Jean-Louis Leroy named [yomm2](https://github.com/jll63/yomm2).

## Index

* [Why omm?](https://github.com/Hectarea1996/omm#why-omm)
* [Installation](https://github.com/Hectarea1996/omm#installation)
* [A simple tutorial](https://github.com/Hectarea1996/omm#a-simple-tutorial)

## Why omm?
The best features of omm are:
* It has no dependencies. You only need a C++17 compiler.
* omm creates the necessary tables of pointers in compile time. So, no runtime overhead creating the tables.
* omm offers template open multi-methods. See [below] for more information. 

## Installation
Put the omm.h file in your project and include it.

## A simple tutorial
As an example, we will use matrices. For each method and their implementations we need to create a table, an 'omm table'. This table needs 3 ingredients, a function signature telling what the 'virtual types' are, a struct containing the implementations of the method, and all the classes that participate in the selection of the correct implementation once the method is called. 

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

Note that the base class must have a virtual method. In fact, every virtual class that could appear in the signature (see the next section) must be polymorphic, in other words, must have at least a virtual method.

### 1st ingredient: WithSignature
In this case we want to add two matrices and return the result. We need to indicate this signature using 'WithSignature':

```C++
using add_template = WithSignature<Matrix*(Virtual<Matrix*>,Virtual<Matrix*>)>;
```

This signature is telling that the open multi-method will receive two pointers to objects of type `Matrix` or some of their daughter classes (`Diagonal`, `Orthogonal` or `Invertible`), and it will return a pointer to a `Matrix`. Note that we wrote `Virtual<Matrix*>` to indicate the parameters could be a derived class of `Matrix`. Only the parameters can be `Virtual` and these types can appear in any order. A more complex example could involve `Virtual` and non-`Virtual` types:

```C++
using complex_template = WithSignature<int(Virtual<BaseClass*>,int,float,Virtual<BaseClass2&>,Virtual<const BaseClass1&>,char)>
```

As you can see, `Virtual` types can be a pointer or a reference, and they can have cv-qualifiers too. 

### 2nd ingredient: WithImplementations
Somehow, we need to tell omm where the implementations are. To do that, we are going to create a struct containing all the implementations and we will pass it to omm later. The visibility of each implementation must be public, so we use a struct instead of a class (you can create a class if you want of course). Also, make sure every function is `static`. Lastly, the name of each implementation must be `implementation`:

```C++
struct add_matrices{ // <-- Make sure the visibility of all the implementations is public

  static Matrix* implementation(Diagonal* d1, Diagonal* d2){    // <-- Use static in each implementation
    // Add two diagonal matrices
  }
  
  static Matrix* implementation(Orthogonal* o, Diagonal* d){     // <-- Use static in each implementation
    // Add orthogonal and diagonal matrices
  }
  
  static Matrix* implementation(Invertible* i, Orthogonal* o){   // <-- Use static in each implementation
    // Add invertible and orthogonal matrices
  }
  
  //...
  
}

// We store the struct using WithImplementations
using struct_implementations = WithImplementations<add_matrices>;
```

Note that we use `WithImplementations` to store the struct containing all the implementations. Also, observe that the signature of each implementation respects the signature passed to `WithSignature` in the previous section. The parameters are pointers to derived types of `Matrix` and the result is of type a pointer to `Matrix`.

### 3rd ingredient: WithDerivedTypes
In this case we are using 3 daughter classes of `Matrix`: `Diagonal`, `Orthogonal` and `Invertible`. 

```
using derived_matrices = WithDerivedTypes<Diagonal,Ortogonal,Invertible>;
```

The order is irrelevant.

### Creating the table
We use the 3 ingredients in the next order:
1. The implementations (`WithImplementations`)
2. The signature (`WithSignature`)
3. The derived types (`WithDerivedTypes`)

```C++
using add_matrices_table = table_omm<struct_implementations,add_template,derived_matrices>;
```

### Accessing to the table
The table contains a `call` method that looks for the correct implementation and calls it. We can create a function that uses the `call` method:

```C++
Matriz* add(Matriz* m1, Matriz* m2){ // <-- Make sure the parameters and the result types respect the signature of `WithSignature`.
  return add_matrices_table::call(m1,m2);     // <-- We call the `call` method with all the arguments.
}
```

Note that `add` and `call` has the same signature as the implementations. 

You can see another example in the examples directory using two base classes, `Animal` and `Shape`.


