

#include "../../omm.h"
#include "shapes.h"
#include <iostream>


int main(){

    Ellipse e;
    Circle c;
    Rectangle r;
    Triangle t;

    Shape& f1 = e;
    Shape& f2 = c;
    Shape& f3 = r;
    Shape& f4 = t;


    Dog d;
    Cat g;

    Animal& a1 = d;
    Animal& a2 = g;

    example_function(&a1,500,&f3,5.0,f2);
    example_function(&a2,1000,&f4,3.0,f1);
    example_function(&a2,500,&f2,5.0,f3);
    example_function(&a1,250,&f3,7.0,f3);


    return 0;

}
