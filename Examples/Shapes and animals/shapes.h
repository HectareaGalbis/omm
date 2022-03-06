#ifndef FIGURA_H_INCLUDED
#define FIGURA_H_INCLUDED

#include "../../omm.h"
#include <string>
#include <iostream>


struct Shape{
    virtual  ~Shape(){}   // <-- The class must be polymorphic (at least one virtual method)
};

struct Ellipse : Shape{};

struct Circle : Ellipse{};     // <-- Accepts complex inheritance hierarchy.

struct Rectangle : Shape{};

struct Triangle : Shape{};



struct Animal{
    virtual void walk(){}   // <-- The class must be polymorphic (at least one virtual method)
};

struct Dog : Animal{};

struct Cat : Animal{};


struct example_implementations{

    static void implementation(Dog* c, int k,volatile Rectangle* r2, float fl, const Circle& p2){
        std::cout << "Dog - Rectangle - Circle" << std::endl;
    }

    static void implementation(Cat* t, int k, volatile Triangle* t2, float fl, const Ellipse& p3){
        std::cout << "Cat - Triangle - Ellipse" << std::endl;
    }

    static void implementation(Cat* t, int k, volatile Circle* t2, float fl, const Rectangle& p3){
        std::cout << "Cat - Circle - Rectangle" << std::endl;
    }

    static void implementation(Dog* t, int k, volatile Rectangle* r, float fl, const Rectangle& g2){
        std::cout << "Dog - Rectangle - Rectangle" << std::endl;
    }

};

using table_example = table_omm<WithImplementations<example_implementations>,
                                WithSignature<void(Virtual<Animal*>,int,Virtual<volatile Shape*>,float,Virtual<const Shape&>)>,
                                WithDerivedTypes<Circle,Dog,Rectangle,Cat,Triangle,Ellipse>>;

void example_function(Animal* a, int n, volatile Shape* f1, float k, const Shape& f2){
    table_example::call(a,n,f1,k,f2);
}


#endif // FIGURA_H_INCLUDED
