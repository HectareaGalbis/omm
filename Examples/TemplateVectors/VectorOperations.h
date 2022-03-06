#ifndef VECTOROPERATIONS_H_INCLUDED
#define VECTOROPERATIONS_H_INCLUDED

#include "../../omm.h"
#include "Vector.h"
#include "UnitVector.h"
#include "CanonVector.h"
#include <iostream>


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

template<typename T, unsigned int N>
using add_vectors_table = table_omm<WithImplementations<add_vectors_impl<T,N>>,
                                    WithSignature<Vector<T,N>(Virtual<const Vector<T,N>&>,Virtual<const Vector<T,N>&>)>,
                                    WithDerivedTypes<UnitVector<T,N>, CanonVector<T,N>>>;

template<typename T, unsigned int N>
Vector<T,N> Vector<T,N>::operator+(const Vector<T,N>& other) const{
    return add_vectors_table<T,N>::call(*this,other);
}


#endif // VECTOROPERATIONS_H_INCLUDED
