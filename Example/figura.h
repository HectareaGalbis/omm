#ifndef FIGURA_H_INCLUDED
#define FIGURA_H_INCLUDED

#include "omm.h"
#include <string>
#include <iostream>


struct Figura{
    virtual void area(){}   // <-- La clase debe ser polimorfica (al menos un método virtual)
};

struct Circulo : Figura{};

struct Rectangulo : Figura{};

struct Triangulo : Figura{};



struct Animal{
    virtual void walk(){}
};

struct Perro : Animal{};

struct Gato : Animal{};


using intersect_template = void(Virtual<Animal*>,int,Virtual<Figura&>,Virtual<Animal&>,Virtual<Figura*>,float,Virtual<Animal&>);

struct intersect{

    static void call(Perro* c, int k, Rectangulo& r, Perro& p, Rectangulo* r2, float fl, Perro& p2){
        std::cout << "Perro - " << k << " - Rectangulo - Perro - Rectangulo - Perro " << fl << std::endl;
    }

    static void call(Gato* t, int k, Circulo& c, Gato& p, Triangulo* t2, float fl, Perro& p3){
        std::cout << "Gato - " << k << " - Circulo - Gato - Triangulo - Perro " << fl << std::endl;
    }

    static void call(Perro* t, int k, Triangulo& t2, Perro& p, Rectangulo* r, float fl, Gato& g2){
        std::cout << "Perro - " << k << " - Triangulo - Perro - Rectangulo - Gato " << fl << std::endl;
    }

};

using intersect_table = table_omm<intersect,intersect_template,Circulo,Perro,Rectangulo,Gato,Triangulo>;

void intersect(Animal* f, int k, Figura& g, Animal& a, Figura* t, float fl, Animal& a2){
    intersect_table::call(f,k,g,a,t,fl,a2);
}


#endif // FIGURA_H_INCLUDED
