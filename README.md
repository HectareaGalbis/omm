# omm (Open Multi-Methods)
This project is an unique file which offers open multi-methods. I was inspired by the library made by Jean-Louis Leroy named [yomm2](https://github.com/jll63/yomm2).

## Index

* [Why omm?](https://github.com/Hectarea1996/omm#why-omm)
* [Installation](https://github.com/Hectarea1996/omm#installation)
* [A simple tutorial](https://github.com/Hectarea1996/omm#a-simple-tutorial)
* [Template Open Multi-Methods](https://github.com/Hectarea1996/omm#template-open-multi-methods)

## Why omm?
The best features of omm are:
* It has no dependencies. You only need a C++17 compiler.
* omm creates the necessary tables of pointers in compile time. So, no runtime overhead creating the tables.
* omm offers template open multi-methods. See [here](https://github.com/Hectarea1996/omm#template-open-multi-methods) for more information. 

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

## Template open multi-methods
All the parameters that the omm table needs are types. So, we can use template parameters in our table. To illustrate how to do this, we will show how to create a method that adds two vectors of dependent inner type and size. Also, we will indicate the files where the code is written and we will see how to avoid cyclic dependencies.

```C++
// Vector.h

template<typename T, unsigned int N>
class Vector{
    //...
    public:
        virtual ~Vector(){}   // We need at least a virtual method.
        
        Vector<T,N> operator+(const Vector<T,N>& other) const;

};
```

Our objective will be to implement the `+` operator using multiple dispatch.

Another two vectors that specialize the first one:

```C++
// CanonVector.h

#include "Vector.h"

template<typename T, unsigned int N>
class CanonVector : public Vector<T,N>{
    //...
};
```

```C++
// UnitVector.h

#include "Vector.h"

template<typename T, unsigned int N>
class UnitVector : public Vector<T,N>{
    //...
};
```

Note that the implementation of the `+` operator must be in the header because `Vector` is a template class. However, we may need to include the rest of the daughter classes for being able to implement all the specializations. So, the next code will cause a cyclic dependency and the project will not compile:

```C++
// Vector.h

#include "omm.h"
#include "CanonVector.h"
#include "UnitVector"

template<typename T, unsigned int N>
class Vector{
    //...
    public:
        virtual ~Vector(){}   // We need at least a virtual method.
        
        Vector<T,N> operator+(const Vector<T,N>& other) const;

};

// ----------------------
// Implementation here???
// ----------------------
```

We can solve this by putting the implementations in a separate file. 

```C++
// VectorOperations.h

#include "omm.h"
#include "Vector.h"
#include "UnitVector.h"
#include "CanonVector.h"

// -------------------
// Implementation here
// -------------------
```

Only one problem left. If we want to use the functions defined in this file, we must to include it. Ideally, we should include just the `Vector.h` file. For this, we only need to add this include at the end of the `Vector.h` file.


```C++
// Vector.h

template<typename T, unsigned int N>
class Vector{
    //...
    public:
        virtual ~Vector(){}   // We need at least a virtual method.
        
        Vector<T,N> operator+(const Vector<T,N>& other) const;

};

#include "VectorOperations.h"
```

Finally, let's complete the implementations. The struct containing the implementations must be a template struct with the desired arguments.

```C++
// VectorOperations.h

#include "omm.h"
#include "Vector.h"
#include "UnitVector.h"
#include "CanonVector.h"

template<typename T, unsigned int N>
struct add_vectors_impl{

    static Vector<T,N> implementation(const Vector<T,N>& v1, const Vector<T,N>& v2){
        std::cout << "Adding two simple vectors" << std::endl;
        return v1;
    }

    static Vector<T,N> implementation(const CanonVector<T,N>& v1, const Vector<T,N>& v2){
        std::cout << "Adding a canon vector and a simple vector" << std::endl;
        return v1;
    }

    static Vector<T,N> implementation(const UnitVector<T,N>& v1, const UnitVector<T,N>& v2){
        std::cout << "Adding two unit vectors" << std::endl;
        return v1;
    }

};

// ----------------------
// The table will be here
// ----------------------

// -----------------------
// The method will be here
// -----------------------
```

Lastly, we create the table with the rest of the information and we implement the `+` operator.

```C++
// VectorOperations.h

#include "omm.h"
#include "Vector.h"
#include "UnitVector.h"
#include "CanonVector.h"

template<typename T, unsigned int N>
struct add_vectors_impl{

   //...

};

template<typename T, unsigned int N>
using add_vectors_table = table_omm<WithImplementations<add_vectors_impl<T,N>>,
                                    WithSignature<Vector<T,N>(Virtual<const Vector<T,N>&>,Virtual<const Vector<T,N>&>)>,
                                    WithDerivedTypes<UnitVector<T,N>, CanonVector<T,N>>>;

template<typename T, unsigned int N>
Vector<T,N> Vector<T,N>::operator+(const Vector<T,N>& other) const{
    return add_vectors_table<T,N>::call(*this,other);
}
```

Now we can add vectors using multiple dispatch.

```C++
// main.cpp

#include "Vector.h"
#include "CanonVector.h"
#include "UnitVector.h"

int main(){

    CanonVector<int,3> v1;
    CanonVector<int,3> v2;
    v1+v2;
    
    UnitVector<float,5> v3;
    UnitVector<float,5> v4;
    v3+v4;

    return 0;

}
```

```
Output:
Adding a canon vector and a simple vector
Adding two unit vectors
```
