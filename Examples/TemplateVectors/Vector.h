#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

template<typename T, unsigned int N>
class Vector{

    public:

        virtual ~Vector(){}

        Vector<T,N> operator+(const Vector<T,N>& other) const;

};


#include "VectorOperations.h"


#endif // VECTOR_H_INCLUDED
