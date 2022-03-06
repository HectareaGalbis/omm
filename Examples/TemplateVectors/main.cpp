

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
