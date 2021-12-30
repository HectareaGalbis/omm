#ifndef OMM_H_INCLUDED
#define OMM_H_INCLUDED

#include <typeinfo>
#include <type_traits>
#include <algorithm>
#include <iostream>

//---------------------------------------------------------------------------------
//---------------------------------- Data types -----------------------------------
//---------------------------------------------------------------------------------

/**
*   Tipo de dato que representa a un entero
*/
template<int k>
using int_constant = std::integral_constant<int,k>;

//---------------------------------------------------------------------------------

/**
*   Los numeros 0 y 1
*/
using zero = int_constant<0>;
using one = int_constant<1>;

//---------------------------------------------------------------------------------

/**
*   Realiza la suma de varios numeros
*/
template<typename... TS>
struct add : zero{};

template<typename T, typename... TS>
struct add<T,TS...> : int_constant<T::value+add<TS...>::value>{};

template<typename... TS>
using add_t = typename add<TS...>::type;

template<typename... TS>
static constexpr int add_v = add<TS...>::value;

//---------------------------------------------------------------------------------

/**
*   Aumenta en 1 el valor de un entero
*/
template<typename T>
using add1 = add<one,T>;

template<typename T>
using add1_t = typename add1<T>::type;

template<typename T>
static constexpr int add1_v = add1<T>::value;

//---------------------------------------------------------------------------------

/**
*   Disminuye en 1 el valor de un entero
*/
template<typename T>
using sub1 = add<int_constant<-1>,T>;

template<typename T>
using sub1_t = typename sub1<T>::type;

template<typename T>
static constexpr int sub1_v = sub1<T>::value;

//---------------------------------------------------------------------------------

/**
*   Multiplica dos enteros
*/
template<typename... TS>
struct mult : one{};

template<typename T, typename... TS>
struct mult<T,TS...> : int_constant<T::value*mult<TS...>::value>{};

template<typename... TS>
using mult_t = typename mult<TS...>::type;

template<typename... TS>
static constexpr int mult_v = mult<TS...>::value;

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

/**
*   Tipo que representa una coleccion de tipos
*/
template<typename... S>
struct collection{
    using type = collection<S...>;
};

//---------------------------------------------------------------------------------

/**
*   Ejecuta una metafuncion usando como argumentos los tipos de una coleccion
*/
template<template<typename...> typename P, typename C>
struct apply_collection{};

template<template<typename...> typename P, typename... S>
struct apply_collection<P,collection<S...>> : P<S...>{};

template<template<typename...> typename P, typename C>
using apply_collection_t = typename apply_collection<P,C>::type;

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

/**
*   Tipo que representa una lista vacia
*/
struct nil{
    using type = nil;
};

//---------------------------------------------------------------------------------

/**
*   Tipo que representa una estructura cons
*/
template<typename T, typename S>
struct cons{
    using type = cons<T,S>;
};

template<typename T>
struct cons_c{
    template<typename S>
    using type = cons<T,S>;
};

//---------------------------------------------------------------------------------

/**
*   Funcion de ayuda para generar una lista de elementos
*/
template<typename... TS>
struct tlist : nil{};

template<typename T, typename... TS>
struct tlist<T,TS...> : cons<T,typename tlist<TS...>::type>{};

template<typename... TS>
using tlist_t = tlist<TS...>::type;

//---------------------------------------------------------------------------------

/**
*   Transforma una coleccion en una lista
*/
template<typename C>
struct collection_to_tlist{};

template<typename... S>
struct collection_to_tlist<collection<S...>> : tlist<S...>{};

//---------------------------------------------------------------------------------

/**
*   Transforma una lista en una coleccion
*/
template<typename L, typename C>
struct tlist_to_collection_aux{};

template<typename C>
struct tlist_to_collection_aux<nil,C> : C{};

template<typename T, typename R, typename... S>
struct tlist_to_collection_aux<cons<T,R>,collection<S...>> : tlist_to_collection_aux<R,collection<S...,T>>{};

template<typename T>
struct tlist_to_collection : tlist_to_collection_aux<T,collection<>>{};

template<typename T>
using tlist_to_collection_t = typename tlist_to_collection<T>::type;

//---------------------------------------------------------------------------------

/**
*   Comprueba si el tipo es nil o no.
*/
template<typename T>
struct null : std::false_type{};

template<>
struct null<nil> : std::true_type{};

template<typename T>
using null_t = typename null<T>::type;

template<typename T>
static constexpr bool null_v = null<T>::value;

//---------------------------------------------------------------------------------

/**
*   Devuelve el primer elemento de una lista
*/
template<typename L>
struct car{};

template<>
struct car<nil> : nil{};

template<typename T, typename S>
struct car<cons<T,S>>{
    using type = T;
};

template<typename L>
using car_t = typename car<L>::type;

//---------------------------------------------------------------------------------

/**
*   Elimina el primer elemento de una lista
*/
template<typename L>
struct cdr{};

template<>
struct cdr<nil> : nil{};

template<typename T, typename S>
struct cdr<cons<T,S>>{
    using type = S;
};

template<typename L>
using cdr_t = typename cdr<L>::type;

//---------------------------------------------------------------------------------

/**
*   Concatena dos listas
*/
template<typename... S>
struct append : nil{};

template<typename... S>
struct append<nil,S...> : append<S...>{};

template<typename T, typename R, typename... S>
struct append<cons<T,R>,S...> : cons<T,typename append<R,S...>::type>{};

template<typename L, typename S>
using append_t = append<L,S>::type;

//---------------------------------------------------------------------------------

/**
*   Devuelve el numero de elementos de una lista
*/
template<typename L>
struct length{};

template<>
struct length<nil> : zero{};

template<typename T, typename S>
struct length<cons<T,S>> : add1<typename length<S>::type>{};

template<typename L>
using length_t = typename length<L>::type;

template<typename L>
static constexpr int length_v = length<L>::value;

//---------------------------------------------------------------------------------

/**
*   Retorna true si encuentra un elemento presente en una lista
*/
template<typename T, typename L>
struct member{};

template<typename T>
struct member<T,nil> : std::false_type{};

template<typename T, typename R, typename S>
struct member<T,cons<R,S>> : member<T,S>{};

template<typename T, typename S>
struct member<T,tlist<T,S>> : std::true_type{};

template<typename T, typename L>
using member_t = typename member<T,L>::type;

template<typename T, typename L>
static constexpr bool member_v = member<T,L>::value;

//---------------------------------------------------------------------------------

/**
*   Retorna el indice de un elemento en una type_list
*/
template<typename T, typename L, typename K>
struct position_aux : position_aux<T,cdr_t<L>,add1_t<K>>{};

template<typename T, typename S, typename K>
struct position_aux<T,cons<T,S>,K> : K{};

template<typename T, typename L>
struct position : position_aux<T,L,zero>{};

template<typename T, typename L>
using position_t = typename position<T,L>::type;

template<typename T, typename L>
static constexpr int position_v = position<T,L>::value;

//---------------------------------------------------------------------------------

/**
*   Devuelve el primer elemento de una lista que cumple con una condicion
*/
template<template<typename> typename P, typename L>
struct find_if{};

template<typename Found, template<typename> typename P, typename T, typename L>
struct found_if : find_if<P,L>{};

template<template<typename> typename P, typename T, typename L>
struct found_if<std::true_type,P,T,L>{
    using type = T;
};

template<template<typename> typename P>
struct find_if<P,nil> : nil{};

template<template<typename> typename P, typename T, typename S>
struct find_if<P,cons<T,S>> : found_if<typename P<T>::type,P,T,S>{};

template<template<typename> typename P, typename L>
using find_if_t = find_if<P,L>::type;

//---------------------------------------------------------------------------------

/**
*   Retorna el elemento que se encuentra en la posicion k-esima de una lista
*/
template<typename L, typename K>
struct nth_aux : nth_aux<cdr_t<L>,sub1_t<K>>{};

template<typename L>
struct nth_aux<L,zero> : car<L>{};

template<typename L, typename K>
struct nth : nth_aux<L,K>{};

template<typename L, typename K>
using nth_t = typename nth<L,K>::type;

//---------------------------------------------------------------------------------

/**
*   Ejecuta una metafuncion sobre cada elemento de la lista
*/
template<template<typename> typename P, typename L>
struct mapcar{};

template<template<typename> typename P>
struct mapcar<P,nil> : nil{};

template<template<typename> typename P, typename T, typename S>
struct mapcar<P,cons<T,S>> : cons<typename P<T>::type,typename mapcar<P,S>::type>{};

template<template<typename> typename P, typename L>
using mapcar_t = typename mapcar<P,L>::type;

//---------------------------------------------------------------------------------

/**
*   Realiza un fold de derecha a izquierda
*/
template<template<typename,typename> typename P, typename I, typename L>
struct reduce_from_end : P<typename reduce_from_end<P,I,cdr_t<L>>::type,car_t<L>>{};

template<template<typename,typename> typename P, typename I, typename T>
struct reduce_from_end<P,I,cons<T,nil>> : P<I,T>{};

template<template<typename,typename> typename P, typename I, typename L>
using reduce_from_end_t = typename reduce_from_end<P,I,L>::type;

//---------------------------------------------------------------------------------

/**
*   Realiza un cons si B es true_type
*/
template<typename B, typename T, typename L>
struct cons_if : cons<T,L>{};

template<typename T, typename L>
struct cons_if<std::false_type,T,L> : L{};

template<typename B, typename T, typename L>
using cons_if_t = typename cons_if<B,T,L>::type;

//---------------------------------------------------------------------------------

/**
*   Elimina los elementos de la lista que no cumplan la condicion
*/
template<template<typename> typename P, typename L>
struct remove_if_not{};

template<template<typename> typename P>
struct remove_if_not<P,nil> : nil{};

template<template<typename> typename P, typename T, typename S>
struct remove_if_not<P,cons<T,S>> : cons_if<typename P<T>::type,T,typename remove_if_not<P,S>::type>{};

template<template<typename> typename P, typename L>
using remove_if_not_t = typename remove_if_not<P,L>::type;

//---------------------------------------------------------------------------------

/**
*   Ejecuta una metafuncion usando como argumentos los que se pasen, ademas de usar los elementos de la lista situada al final.
*/
template<typename... T>
struct apply_aux : nil{};

template<typename T, typename... TS>
struct apply_aux<T,TS...> : T{};

template<typename T, typename S, typename... TS>
struct apply_aux<T,S,TS...> : cons<T,typename apply_aux<S,TS...>::type>{};

template<template<typename...> typename P, typename... S>
struct apply : apply_collection<P,tlist_to_collection_t<typename apply_aux<S...>::type>>{};

template<template<typename...> typename P, typename... S>
using apply_t = typename apply<P,S...>::type;

template<template<typename...> typename P>
struct apply_c{
    template<typename... S>
    using type = apply<P,S...>;
};

//---------------------------------------------------------------------------------

/**
*   Retorna false_type si algun elemento de la lista es false_type
*/
template<typename... TS>
struct and_type : std::true_type{};

template<typename T, typename... TS>
struct and_type<T,TS...> : and_type<TS...>{};

template<typename... TS>
struct and_type<std::false_type,TS...> : std::false_type{};

template<typename... TS>
using and_type_t = typename and_type<TS...>::type;

//---------------------------------------------------------------------------------

/**
*   Retorna false_type si algun elemento de la lista es false_type
*/
template<typename... TS>
struct or_type : std::false_type{};

template<typename... TS>
struct or_type<std::true_type,TS...> : std::true_type{};

template<typename... TS>
struct or_type<std::false_type,TS...> : or_type<TS...>{};

template<typename... TS>
using or_type_t = typename or_type<TS...>::type;

//---------------------------------------------------------------------------------

/**
*   Retorna una lista donde cada elemento es una lista que contiene dos elementos que estaban
*   en la misma posicion en las listas iniciales.
*/
template<typename L, typename S>
struct zip : nil{};

template<typename P, typename Q, typename R, typename S>
struct zip<cons<P,Q>,cons<R,S>> : cons<tlist_t<P,R>,typename zip<Q,S>::type>{};

template<typename L, typename S>
using zip_t = typename zip<L,S>::type;

//---------------------------------------------------------------------------------

/**
*   Invierte el orden de los argumentos de una metafuncion
*/
template<template<typename,typename> typename P>
struct flip{
    template<typename T, typename S>
    using type = typename P<S,T>::type;
};

//---------------------------------------------------------------------------------
//----------------------------------- Auxiliar ------------------------------------
//---------------------------------------------------------------------------------

/**
*   Algunas metafunciones de ayuda
*/
namespace std{
    template<typename T>
    using is_reference_t = typename is_reference<T>::type;

    template<typename T>
    using is_pointer_t = typename is_pointer<T>::type;

    template<typename T>
    using is_const_t = typename is_const<T>::type;

    template<typename T>
    using is_volatile_t = typename is_volatile<T>::type;

    template<typename T>
    using is_lvalue_reference_t = typename is_lvalue_reference<T>::type;

    template<typename T>
    using is_rvalue_reference_t = typename is_rvalue_reference<T>::type;

    template<typename T>
    using is_polymorphic_t = typename is_polymorphic<T>::type;

    template<typename B, typename D>
    using is_base_of_t = typename is_base_of<B,D>::type;

    template<typename B>
    struct is_base_of_c{
        template<typename D>
        using type = std::is_base_of_t<B,D>;
    };
}

//---------------------------------------------------------------------------------

/**
*   Elimina referencias, punteros y cualificadores cv de un tipo de dato (int**const volatile*const** -> int)
*/
template<typename IsRef, typename IsPtr, typename T>
struct core_type_aux{
    using type = std::remove_cv_t<T>;
};

template<typename IsPtr, typename T>
struct core_type_aux<std::true_type,IsPtr,T> : core_type_aux<std::is_reference_t<std::remove_cv_t<std::remove_reference_t<T>>>,
                                                             std::is_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>,
                                                             std::remove_cv_t<std::remove_reference_t<T>>>{};

template<typename T>
struct core_type_aux<std::false_type,std::true_type,T> : core_type_aux<std::is_reference_t<std::remove_pointer_t<T>>,
                                                                       std::is_pointer_t<std::remove_pointer_t<T>>,
                                                                       std::remove_pointer_t<T>>{};

template<typename T>
struct core_type : core_type_aux<std::is_reference_t<T>,std::is_pointer_t<T>,T>{};

template<typename T>
using core_type_t = typename core_type<T>::type;

//---------------------------------------------------------------------------------

/**
*   Traslada referencias, punteros y cualificadores cv de un tipo origen a un destino
*   ( [int*const**volatile*&, float] -> float*const**volatile*& )
*/
template<typename IsConst, typename IsVol, typename IsLval, typename IsRval, typename IsPtr, typename N, typename S>
struct slice_type_aux{
    using type = S;
};

template<typename IsVol, typename IsLval, typename IsRval, typename IsPtr, typename N, typename S>
struct slice_type_aux<std::true_type,IsVol,IsLval,IsRval,IsPtr,N,S>{
    using newN = std::remove_const_t<N>;
    using type = std::add_const_t<typename slice_type_aux<std::is_const_t<newN>,std::is_volatile_t<newN>,std::is_lvalue_reference_t<newN>,
                                                          std::is_rvalue_reference_t<newN>,std::is_pointer_t<newN>,newN,S>::type>;
};

template<typename IsLval, typename IsRval, typename IsPtr, typename N, typename S>
struct slice_type_aux<std::false_type,std::true_type,IsLval,IsRval,IsPtr,N,S>{
    using newN = std::remove_volatile_t<N>;
    using type = std::add_volatile_t<typename slice_type_aux<std::is_const_t<newN>,std::is_volatile_t<newN>,std::is_lvalue_reference_t<newN>,
                                                          std::is_rvalue_reference_t<newN>,std::is_pointer_t<newN>,newN,S>::type>;
};

template<typename IsRval, typename IsPtr, typename N, typename S>
struct slice_type_aux<std::false_type,std::false_type,std::true_type,IsRval,IsPtr,N,S>{
    using newN = std::remove_reference_t<N>;
    using type = std::add_lvalue_reference_t<typename slice_type_aux<std::is_const_t<newN>,std::is_volatile_t<newN>,std::is_lvalue_reference_t<newN>,
                                                          std::is_rvalue_reference_t<newN>,std::is_pointer_t<newN>,newN,S>::type>;
};

template<typename IsPtr, typename N, typename S>
struct slice_type_aux<std::false_type,std::false_type,std::false_type,std::true_type,IsPtr,N,S>{
    using newN = std::remove_reference_t<N>;
    using type = std::add_rvalue_reference_t<typename slice_type_aux<std::is_const_t<newN>,std::is_volatile_t<newN>,std::is_lvalue_reference_t<newN>,
                                                          std::is_rvalue_reference_t<newN>,std::is_pointer_t<newN>,newN,S>::type>;
};

template<typename N, typename S>
struct slice_type_aux<std::false_type,std::false_type,std::false_type,std::false_type,std::true_type,N,S>{
    using newN = std::remove_pointer_t<N>;
    using type = std::add_pointer_t<typename slice_type_aux<std::is_const_t<newN>,std::is_volatile_t<newN>,std::is_lvalue_reference_t<newN>,
                                                          std::is_rvalue_reference_t<newN>,std::is_pointer_t<newN>,newN,S>::type>;
};

template<typename N, typename S>
struct slice_type{
    using type = typename slice_type_aux<std::is_const_t<N>,std::is_volatile_t<N>,std::is_lvalue_reference_t<N>,
                                         std::is_rvalue_reference_t<N>,std::is_pointer_t<N>,N,S>::type;
};

template<typename N, typename S>
using slice_type_t = typename slice_type<N,S>::type;

//---------------------------------------------------------------------------------
//----------------------------------- Virtual -------------------------------------
//---------------------------------------------------------------------------------

/**
*   Contenedor de un tipo virtual usado para los tipos que participan en el multiple dispatch.
*   El tipo T debe verificar is_virtual_valid.
*/
template<typename T>
struct virtual_type{
    using type = virtual_type<T>;
};

//---------------------------------------------------------------------------------

/**
*   Extrae el tipo dentro de virtual_type
*/
template<typename T>
struct open_virtual_type{};

template<typename T>
struct open_virtual_type<virtual_type<T>>{
    using type = T;
};

template<typename T>
using open_virtual_type_t = typename open_virtual_type<T>::type;

//---------------------------------------------------------------------------------

/**
*   Extrae el tipo de un virtual type si, efectivamente, es un virtual_type.
*   En otro caso devuelve el tipo recibido.
*/
template<typename T>
struct open_virtual_type_if_virtual{
    using type = T;
};

template<typename T>
struct open_virtual_type_if_virtual<virtual_type<T>>{
    using type = T;
};

template<typename T>
using open_virtual_type_if_virtual_t = typename open_virtual_type_if_virtual<T>::type;

//---------------------------------------------------------------------------------

/**
*   Traslada virtual_type, punteros, referencias y cualificadores a otro tipo.
*/
template<typename VT, typename S>
struct slice_virtual_type : virtual_type<slice_type_t<open_virtual_type_t<VT>,S>>{};

template<typename VT, typename S>
using slice_virtual_type_t = typename slice_virtual_type<VT,S>::type;

//---------------------------------------------------------------------------------

/**
*   Comprueba si un tipo de dato es un puntero o una referencia a un tipo polimorfico (que contiene al menos un metodo virtual).
*/
template<typename T>
struct is_polymorphic_pr : or_type<std::is_polymorphic_t<std::remove_pointer_t<T>>,std::is_polymorphic_t<std::remove_reference_t<T>>>{};

template<typename T>
using is_polymorphic_pr_t = typename is_polymorphic_pr<T>::type;

template<typename T>
static constexpr bool is_polymorphic_pr_v = is_polymorphic_pr<T>::value;

//---------------------------------------------------------------------------------

/**
*   Interfaz para el usuario.
*   El tipo usado debe ser un tipo virtual, es decir, un puntero o una referencia a un tipo polimorfico (que tenga al menos un metodo virtual).
*/
template<typename T>
using Virtual = std::enable_if_t<is_polymorphic_pr_v<T>,virtual_type<T>>;


//---------------------------------------------------------------------------------
//--------------------------------- Base of many ----------------------------------
//---------------------------------------------------------------------------------

/**
*   Un base_of_many es una lista cuyo primer elemento es una clase base del resto de elementos.
*/

//---------------------------------------------------------------------------------

/**
*   Genera un base of many a partir de un tipo base y una lista de tipos cualquiera.
*/
template<typename B, typename DL>
struct create_base_of_many : cons<B,remove_if_not_t<std::is_base_of_c<B>::template type,DL>>{};

template<typename B, typename DL>
using create_base_of_many_t = typename create_base_of_many<B,DL>::type;

template<typename DL>
struct create_base_of_many_c_inv{
    template<typename B>
    using type = create_base_of_many_t<B,DL>;
};

//---------------------------------------------------------------------------------

/**
*   Retorna la cantidad de tipos derivados de un base_of_many
*/
template<typename BA>
using length_of_derived = length<cdr_t<BA>>;

template<typename BA>
using length_of_derived_t = length_of_derived<BA>::type;

template<typename BA>
static constexpr int length_of_derived_v = length_of_derived<BA>::value;

//---------------------------------------------------------------------------------

/**
*   Dado un indice, devuelve el tipo derivado que ocupa dicha posicion
*/
template<typename BA, typename K>
using nth_derived = nth<cdr_t<BA>,K>;

template<typename BA, typename K>
using nth_derived_t = typename nth_derived<BA,K>::type;

//---------------------------------------------------------------------------------

/**
*   Dado un indice, devuelve el tipo derivado que ocupa dicha posicion
*/
template<typename DL, typename D, typename K>
struct position_derived_aux{};

template<typename T, typename TS, typename D, typename K>
struct position_derived_aux<cons<T,TS>,D,K> : position_derived_aux<TS,D,add1_t<K>>{};

template<typename TS, typename D, typename K>
struct position_derived_aux<cons<D,TS>,D,K> : K{};

template<typename BM, typename D>
struct position_derived : position_derived_aux<cdr_t<BM>,D,zero>{};

template<typename BM, typename D>
using position_derived_t = typename position_derived<BM,D>::type;

template<typename BM, typename D>
static constexpr int position_derived_v = position_derived<BM,D>::value;

//---------------------------------------------------------------------------------

/**
*   Recibe un type_info de algun objeto y devuelve el indice que ocupa su tipo en un
*   base_of_many.
*/
template<typename BM>
struct position_derived_runtime_aux{
    static int call(const std::type_info& info){
        return 0;
    }
};

template<typename... T>
struct Print{};

template<typename D, typename S>
struct position_derived_runtime_aux<cons<D,S>>{
    static int call(const std::type_info& info){
        std::cout << "1: " << info.name() << " 2: " << typeid(D).name() << std::endl;
        if (info == typeid(D))
            return 0;
        else
            return 1+position_derived_runtime_aux<S>::call(info);

    }
};

template<typename BM>
struct position_derived_runtime{
    static int call(const std::type_info& info){
        return position_derived_runtime_aux<cdr_t<BM>>::call(info);
    }
};

//---------------------------------------------------------------------------------
//---------------------------------- Signature ------------------------------------
//---------------------------------------------------------------------------------

/**
*   La signatura es una lista cuyo primer elemento es el tipo de retorno de una funcion
*   y el resto de elementos son los tipos de los argumentos.
*   Ademas, puede tener estos modificadores:
*       - Virtual: Que contiene virtual_type.
*       - Base: Los tipos que contienen los virtual_type son tipo polimorficos.
*       - Derived: Los tipos que contienen los virtual_type son tipos derivados de un Base signature.
*   De esta forma podemos distinguir entre Virtual Signature, Base Signature, Virtual Derived Signature, etc.
*/

//---------------------------------------------------------------------------------

/**
*   Transforma el tipo de una funcion en un signature.
*/
template<typename T>
struct ftype_to_sign{};

template<typename R, typename... AS>
struct ftype_to_sign<R(AS...)> : tlist<R,AS...>{};

template<typename T>
using ftype_to_sign_t = typename ftype_to_sign<T>::type;

//---------------------------------------------------------------------------------

/**
*   Devuelve a partir de una Virtual Base Signature los tipo de datos virtuales en su forma core.
*/
template<typename VBS>
struct get_base_core_types{};

template<>
struct get_base_core_types<nil> : nil{};

template<typename B, typename BS>
struct get_base_core_types<cons<B,BS>> : get_base_core_types<BS>{};

template<typename B, typename BS>
struct get_base_core_types<cons<virtual_type<B>,BS>> : cons<core_type_t<B>,typename get_base_core_types<BS>::type>{};

template<typename VBS>
using get_base_core_types_t = typename get_base_core_types<VBS>::type;

//---------------------------------------------------------------------------------

/**
*   Transforma un virtual signature en un signature
*/
template<typename VT>
struct vsign_to_sign : mapcar<open_virtual_type_if_virtual,VT>{};

template<typename VT>
using vsign_to_sign_t = typename vsign_to_sign<VT>::type;

//---------------------------------------------------------------------------------

/**
*   Transforma un virtual base signature en un derived signature sustituyendo cada
*   tipo virtual base por un tipo derivado en la lista D.
*/
template<typename VBS, typename DL>
struct vbsign_to_dsign{};

template<typename VBS>
struct vbsign_to_dsign<VBS,nil> : VBS{};

template<typename T, typename S, typename DL>
struct vbsign_to_dsign<cons<virtual_type<T>,S>,DL> : cons<slice_type_t<T,car_t<DL>>,typename vbsign_to_dsign<S,cdr_t<DL>>::type>{};

template<typename T, typename S, typename DL>
struct vbsign_to_dsign<cons<T,S>,DL> : cons<T,typename vbsign_to_dsign<S,DL>::type>{};

template<typename VBS, typename DL>
using vbsign_to_dsign_t = typename vbsign_to_dsign<VBS,DL>::type;

template<typename VBS>
struct vbsign_to_dsign_c{
    template<typename DL>
    using type = vbsign_to_dsign_t<VBS,DL>;
};

//---------------------------------------------------------------------------------

/**
*   Transforma un virtual base signature en un derived signature sustituyendo cada
*   tipo virtual base por un tipo derivado en la lista D.
*/
template<typename VBS, typename DComb>
struct vbsign_to_dsign_combinations : mapcar<vbsign_to_dsign_c<VBS>::template type,DComb>{};

template<typename VBS, typename DComb>
using vbsign_to_dsign_combinations_t = typename vbsign_to_dsign_combinations<VBS,DComb>::type;

//---------------------------------------------------------------------------------

/**
*   Transforma un virtual base signature en un derived signature sustituyendo cada
*   tipo virtual base por un tipo derivado en la lista D.
*/
template<typename VBS>
struct vbsign_to_bsign{};

template<>
struct vbsign_to_bsign<nil> : nil{};

template<typename B, typename BS>
struct vbsign_to_bsign<cons<virtual_type<B>,BS>> : cons<B,typename vbsign_to_bsign<BS>::type>{};

template<typename B, typename BS>
struct vbsign_to_bsign<cons<B,BS>> : cons<B,typename vbsign_to_bsign<BS>::type>{};

template<typename VBS>
using vbsign_to_bsign_t = typename vbsign_to_bsign<VBS>::type;

//---------------------------------------------------------------------------------

/**
*   Retorna el tipo de una funcion a partir de una coleccion
*/
template<typename C>
struct collection_to_function_type{};

template<typename R, typename... AS>
struct collection_to_function_type<collection<R,AS...>>{
    using type = R(AS...);
};

template<typename C>
using collection_to_function_type_t = typename collection_to_function_type<C>::type;

//---------------------------------------------------------------------------------

/**
*   Retorna el tipo de una funcion a partir de una signature
*/
template<typename L>
struct signature_to_function_type : collection_to_function_type<tlist_to_collection_t<L>>{};

template<typename L>
using signature_to_function_type_t = typename signature_to_function_type<L>::type;

//---------------------------------------------------------------------------------

/**
*   Comprueba que el struct con las implementaciones tenga alguna valida para la signatura CD pasada
*   mediante una coleccion.
*/
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

template<typename T, typename CB, typename CD>
struct has_implementation{};

template<typename T, typename R, typename... Bargs, typename... Dargs>
struct has_implementation<T,collection<R,Bargs...>,collection<R,Dargs...>> : decltype(check_implementation<T,Bargs...>::check(std::declval<Dargs>()...)){};

template<typename T, typename CB, typename CD>
using has_implementation_t = typename has_implementation<T,CB,CD>::type;

template<typename T, typename CB, typename CD>
static constexpr bool has_implementation_v = has_implementation<T,CB,CD>::value;

//---------------------------------------------------------------------------------

/**
*   Genera un puntero a una funcion que llama al metodo correspondiente tras realizar un casting.
*   La signatura de la funcion value y la del casting son pasados mediante una coleccion
*/
template<typename HasImplementation, typename F, typename BC, typename DC>
struct make_function_cell_aux{};

template<typename F, typename R, typename... Bargs, typename... Dargs>
struct make_function_cell_aux<std::false_type,F,collection<R,Bargs...>,collection<R,Dargs...>>{
    static constexpr std::nullptr_t value = nullptr;
};

template<typename F, typename R, typename... Bargs, typename... Dargs>
struct make_function_cell_aux<std::true_type,F,collection<R,Bargs...>,collection<R,Dargs...>>{
    static R value(Bargs... args){
        return F::implementation(static_cast<Dargs>(args)...);
    }
};

template<typename F, typename BS, typename DS>
struct make_function_cell : make_function_cell_aux<has_implementation_t<F,tlist_to_collection_t<BS>,tlist_to_collection_t<DS>>,
                                                   F,tlist_to_collection_t<BS>,tlist_to_collection_t<DS>>{};

//---------------------------------------------------------------------------------
//-------------------------------- Combinations -----------------------------------
//---------------------------------------------------------------------------------

/**
*   Inserta un elemento en cada lista que se encuentra en una lista
*/
template<typename T, typename LL>
using cons_all = mapcar<cons_c<T>::template type,LL>;

template<typename T, typename LL>
using cons_all_t = cons_all<T,LL>::type;

template<typename LL>
struct cons_all_c_inv{
    template<typename T>
    using type = cons_all_t<T,LL>;
};

//---------------------------------------------------------------------------------

/**
*   Realiza el producto cartesiano de dos listas
*/
template<typename L, typename M>
struct cartesian_product_aux : apply<append,mapcar_t<cons_all_c_inv<M>::template type,L>>{};

template<typename... LS>
struct cartesian_product : reduce_from_end<flip<cartesian_product_aux>::template type,tlist_t<nil>,tlist_t<LS...>>{};

template<typename... LS>
using cartesian_product_t = typename cartesian_product<LS...>::type;

//---------------------------------------------------------------------------------

/**
*   Retorna una lista con los numeros menores que uno dado hasta el 0.
*/
template<typename K, typename M>
struct less_numbers_aux : cons<K,typename less_numbers_aux<add1_t<K>,M>::type>{};

template<typename M>
struct less_numbers_aux<M,M> : nil{};

template<typename M>
using less_numbers = less_numbers_aux<zero,M>;

template<typename M>
using less_numbers_t = less_numbers<M>::type;

//---------------------------------------------------------------------------------

/**
*   Crea una lista de todas las combinaciones de numeros entre 0 y cada numero de la lista.
*/
template<typename TID>
struct make_indices : apply<cartesian_product,mapcar_t<less_numbers,mapcar_t<length_of_derived,TID>>>{};

template<typename TID>
using make_indices_t = typename make_indices<TID>::type;

//---------------------------------------------------------------------------------
//------------------------------- Type id converter -------------------------------
//---------------------------------------------------------------------------------

/**
*   Un tipe_id es una lista de base_of_many
*/

//---------------------------------------------------------------------------------

/**
*   Genera un type_id a partir de un virtual base signature y una lista de tipos derivados.
*/
template<typename BCL, typename DL>
struct create_type_id : mapcar<create_base_of_many_c_inv<DL>::template type,BCL>{};

template<typename BCL, typename DL>
using create_type_id_t = typename create_type_id<BCL,DL>::type;

//---------------------------------------------------------------------------------

/**
*   Retorna una lista de tipos cuyos indices en el type_id son los que se pasan como parametro
*/
template<typename Tid, typename KS>
struct indices_to_types : mapcar<apply_c<nth>::template type,zip_t<mapcar_t<cdr,Tid>,KS>>{};

template<typename Tid, typename KS>
using indices_to_types_t = typename indices_to_types<Tid,KS>::type;

template<typename Tid>
struct indices_to_types_c{
    template<typename KS>
    using type = indices_to_types_t<Tid,KS>;
};

//---------------------------------------------------------------------------------

/**
*   Retorna una lista con el numero de tipos derivados que contiene cada base
*/
template<typename Tid>
struct length_of_each_base : mapcar<length_of_derived,Tid>{};

template<typename Tid>
using length_of_each_base_t = typename length_of_each_base<Tid>::type;

//---------------------------------------------------------------------------------

/**
*   Devuelve una lista con todas las posibles combinaciones de tipos derivados
*/
template<typename Tid, typename IND>
struct make_derived_combinations : mapcar<indices_to_types_c<Tid>::template type,IND>{};

template<typename Tid, typename IND>
using make_derived_combinations_t = typename make_derived_combinations<Tid,IND>::type;

//---------------------------------------------------------------------------------

/**
*   Devuelve el typeid de la clase hija subyacente
*/
template<typename IsPtr, typename A>
struct get_type_id_aux{
    static const std::type_info& call(A&& a){
        return typeid(a);
    }
};

template<typename A>
struct get_type_id_aux<std::true_type,A>{
    static const std::type_info& call(A&& a){
        return typeid(*a);
    }
};

template<typename A>
struct get_type_id{
    static const std::type_info& call(A&& a){
        return get_type_id_aux<std::is_pointer_t<std::remove_reference_t<A>>,A>::call(std::forward<A>(a));
    }
};

//---------------------------------------------------------------------------------

/**
*   Devuelve el indice del puntero a la funcion que se debe llamar a partir de los argumentos
*/
template<typename Tid, typename VBS, typename... AS>
struct get_index_aux{
    static constexpr int current_multiplier = 1;
    static int call(AS&&... as){
        return 0;
    }
};

template<typename Tid, typename B, typename BS, typename A, typename... AS>
struct get_index_aux<Tid,cons<B,BS>,A,AS...>{
    static constexpr int current_multiplier = get_index_aux<Tid,BS,AS...>::current_multiplier;
    static int call(A&& a, AS&&... as){
        return get_index_aux<Tid,BS,AS...>::call(std::forward<AS>(as)...);
    }
};

template<typename T, typename TS, typename B, typename BS, typename A, typename... AS>
struct get_index_aux<cons<T,TS>,cons<virtual_type<B>,BS>,A,AS...>{
    static constexpr int current_multiplier = get_index_aux<TS,BS,AS...>::current_multiplier*length_of_derived_v<T>;
    static int call(A&& a, AS&&... as){
        return get_index_aux<TS,BS,AS...>::call(std::forward<AS>(as)...)
               + position_derived_runtime<T>::call(get_type_id<A>::call(std::forward<A>(a)))*get_index_aux<TS,BS,AS...>::current_multiplier;
    }
};

template<typename Tid, typename VBS, typename... AS>
struct get_index{
    static int call(AS&&... as){
        return get_index_aux<Tid,cdr_t<VBS>,AS...>::call(std::forward<AS>(as)...);
    }
};


//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//------------------------------- Generate table ----------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

template<typename F, typename BS, typename DSComb>
struct create_table_omm_aux{};

template<typename F, typename BS, typename... DS>
struct create_table_omm_aux<F,BS,collection<DS...>>{
    static constexpr std::add_pointer_t<signature_to_function_type_t<BS>> value[] = {make_function_cell<F,BS,DS>::value...};
};

template<typename F, typename BS, typename DSComb>
struct create_table_omm : create_table_omm_aux<F,BS,tlist_to_collection_t<DSComb>>{};

template<typename F, typename BS, typename DSComb>
static constexpr auto create_table_omm_v = create_table_omm<F,BS,DSComb>::value;


//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//--------------------------------- Table omm -------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//                              vbsign_to_bsign  -------------> BS = tlist<R,BArgs> ---------------------
//                                              |                                                        |
//                                              |                                    *create_table_omm*  |
//                                              |                                    make_function_cell  |
//                        ftype_to_sign         |                                                        v
//funtion_type = R(VBArgs...) ---> VBS = tlist<R,VBArgs...> *--->* DSCOMB = tlist<R,DArgs...> *--->* table_omm  <----- type with implementations
//                                         |                                *
//                                         | *vbsign_to_dsign_combinations* ^
//                    get_base_core_types  |               vbsign_to_dsign  |
//                                         |                                |
//                                         v                                *
//                                  BCL = tlist<BCArgs...>      Combination Derived Type
//                                         |                      ^         *
//                                         |  *make_derived_combinations*   ^
//                                         |        ------------/           |
//                         create_type_id  |       /  indices_to_types      |
//                                         v      /                         *
//                              DCL ----> TID    ------------------>     Indices
//                                                 *make_indices*



/**
*
*/
template<typename F, typename ftype, typename DCL>
struct table_omm{

    using VBS                   = ftype_to_sign_t<ftype>;
    using BCL                   = get_base_core_types_t<VBS>;
    using BS                    = vbsign_to_bsign_t<VBS>;
    using TID                   = create_type_id_t<BCL,DCL>;
    using IND                   = make_indices_t<TID>;
    using DCOMB                 = make_derived_combinations_t<TID,IND>;
    using DSCOMB                = vbsign_to_dsign_combinations_t<VBS,DCOMB>;
    static constexpr auto table = create_table_omm_v<F,BS,DSCOMB>;

    template<typename... AS>
    static auto call(AS&&... as){
        std::cout << get_index<TID,VBS,AS...>::call(std::forward<AS>(as)...) << std::endl;
        return table[get_index<TID,VBS,AS...>::call(std::forward<AS>(as)...)](std::forward<AS>(as)...);
    }
};


// Interfaz

template<typename... DS>
using WithTypes = tlist_t<DS...>;


#endif // OMM_H_INCLUDED
