

//#include "omm.h"
//#include "figura.h"
#include <iostream>


struct A{};

struct B : A{};

struct C : B{};

struct Prueba{
    static int implementation(C* b, B& c){
        return 0;
    }

    static int implementation(C* c, C& d){
        return 0;
    }

    static int implementation(B* c, C& d){
        return 0;
    }
};


template<typename T>
struct always_true : std::true_type{};

template<typename T, typename... Bargs>
struct check_implementation{

    static std::false_type check(Bargs... bargs){
        return std::false_type();
    }

    template<typename... Dargs>
    static auto check(Dargs... dargs) -> always_true<decltype(T::implementation(dargs...))>::type{
        return std::true_type();
    }

};


//using type = decltype(PrePrueba::call(std::declval<B*>(),std::declval<C&>()));
template<typename... C>
struct collection{
    using type = collection<C...>;
};

template<typename T, typename CB, typename CD>
struct has_implementation{};

template<typename T, typename... Bargs, typename... Dargs>
struct has_implementation<T,collection<Bargs...>,collection<Dargs...>> : decltype(check_implementation<T,Bargs...>::check(std::declval<Dargs>()...)){};

template<typename T, typename CB, typename CD>
static constexpr bool has_implementation_v = has_implementation<T,CB,CD>::value;

template<typename T>
struct Print{};

int main(){

    std::cout << has_implementation_v<Prueba,collection<A*,A&>,collection<B*,C&>> << std::endl;
    //Print<preinf::type>::type;

//    Elipse e;
//    Circulo c;
//    Rectangulo r;
//    Triangulo t;
//
//    Figura& f1 = c;
//    Figura& f2 = r;
//    Figura& f3 = t;
//    Figura& f4 = e;
//
//    Perro p;
//    Gato g;
//
//    Animal& a1 = p;
//    Animal& a2 = g;
//
//    intersect(&a1,1000,f2,a1,&f2,3.0,a1);
//    intersect(&a2,500,f1,a2,&f3,5.0,a1);
//    intersect(&a2,500,f4,a2,&f3,5.0,a1);
//    intersect(&a1,250,f3,a1,&f2,7.0,a2);


    return 0;

}
