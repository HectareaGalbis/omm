#ifndef FIGURA_H_INCLUDED
#define FIGURA_H_INCLUDED

#include "omm.h"
#include <string>
#include <iostream>


struct Figura{
    virtual  ~Figura(){}   // <-- La clase debe ser polimorfica (al menos un método virtual)
};

struct Elipse : Figura{};

struct Circulo : Elipse{};

struct Rectangulo : Figura{};

struct Triangulo : Figura{};



struct Animal{
    virtual void walk(){}
};

struct Perro : Animal{};

struct Gato : Animal{};


using intersect_template = void(Virtual<Animal*>,int,Virtual<Figura*>,float,Virtual<Figura&>);

struct intersect{

    static void implementation(Perro* c, int k,Rectangulo* r2, float fl, Circulo& p2){
        std::cout << "Perro - Rectangulo - Circulo" << std::endl;
    }

    static void implementation(Gato* t, int k, Triangulo* t2, float fl, Elipse& p3){
        std::cout << "Gato - Triangulo - Elipse" << std::endl;
    }

//    static void implementation(Gato* t, int k, Elipse& c, Gato& p, Triangulo* t2, float fl, Perro& p3){
//        std::cout << "Gato - " << k << " - Elipse - Gato - Triangulo - Perro " << fl << std::endl;
//    }
//
//    static void implementation(Perro* t, int k, Triangulo& t2, Perro& p, Rectangulo* r, float fl, Gato& g2){
//        std::cout << "Perro - " << k << " - Triangulo - Perro - Rectangulo - Gato " << fl << std::endl;
//    }

};

using intersect_table = table_omm<intersect,intersect_template,WithTypes<Circulo,Perro,Rectangulo,Gato,Triangulo,Elipse>>;

void intersect(Animal* f, int k, Figura* t, float fl, Figura& a2){
    intersect_table::call(f,k,t,fl,a2);
}


#endif // FIGURA_H_INCLUDED
