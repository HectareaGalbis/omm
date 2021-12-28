#ifndef OMM_H_INCLUDED
#define OMM_H_INCLUDED

#include <typeinfo>
#include <type_traits>
#include <algorithm>

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
static constexpr add1_v = add1<T>::value;

//---------------------------------------------------------------------------------

/**
*   Disminuye en 1 el valor de un entero
*/
template<typename T>
using sub1 = add<int_constant<-1>,T>;

template<typename T>
using sub1_t = typename sub1<T>::type;

template<typename T>
static constexpr sub1_v = sub1<T>::value;

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

//---------------------------------------------------------------------------------

/**
*   Funcion de ayuda para generar una lista de elementos
*/
template<typename... TS>
struct tlist : nil{};

template<typename T, typename... TS>
struct tlist<T,TS...> : cons<T,tlist<TS...>::type>{};

template<typename... TS>
using tlist_t = tlist<TS...>::type;

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
struct append<cons<T,R>,S...> : cons<T,append<R,S...>::type>{};

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
struct length<cons<T,S>> : add1<length<S>::type>{};

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
template<typename Found, template<typename> typename P, typename T, typename L>
struct found_if : find_if<P,L>{};

template<template<typename> typename P, typename T, typename L>
struct found_if<std::true_type,P,T,L>{
    using type = T;
};

template<template<typename> typename P, typename L>
struct find_if{};

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
using nth_t = typename nth<L,k>::type;

//---------------------------------------------------------------------------------

/**
*   Ejecuta una metafuncion sobre cada elemento de la lista
*/
template<template<typename> typename P, typename L>
struct mapcar{};

template<template<typename> typename P>
struct mapcar<P,nil> : nil{};

template<template<typename> typename P, typename T, typename S>
struct mapcar<P,cons<T,S>> : cons<P<T>::type,mapcar<P,S>>{};

template<template<typename> typename P, typename L>
using mapcar_t = typename mapcar<P,L>::type;

//---------------------------------------------------------------------------------

/**
*   Realiza un fold de derecha a izquierda
*/
template<template<typename,typename> typename P, typename I, typename L>
struct reduce_from_end : cons<typename P<reduce_from_end<P,I,cdr_t<L>>::type,car_t<L>>{};

template<template<typename,typename> typename P, typename I, typename L>
using reduce_from_end_t = typename reduce_from_end<P,I,L>::type;

//---------------------------------------------------------------------------------

/**
*   Realiza un cons si B es true_type
*/
template<typename B, typename T, typename S>
struct cons_if : cons<T,L>{};

template<typename T, typename S>
struct cons_if<std::false_type,T,S> : S{};

template<typename B, typename T, typename L>
using cons_if_t = typename cons_if<B,T,L>::type;

//---------------------------------------------------------------------------------

/**
*   Elimina los elementos de la lista que no cumplan la condicion
*/
template<template<typename> typename P, typename L>
struct remove_if_not{};

template<template<typename> typename P, typename L>
struct remove_if_not<P,nil> : nil{};

template<template<typename> typename P, typename T, typename S>
struct remove_if_not<P,cons<T,S>> : cons_if<typename P<T>::type,T,typename remove_if_not<P,S>::type>{};

template<template<typename> typename P, typename L>
using remove_if_not_t = typename remove_if_not<P,L>::type;

//---------------------------------------------------------------------------------

/**
*   Ejecuta una metafuncion usando como argumentos los que se pasen, ademas de usar los elementos de la lista situada al final.
*/
template<template<typename...> typename P, typename L, typename... S>
struct apply_list{};

template<template<typename...> typename P, typename... S>
struct apply_list<P,nil,S...> : P<S...>{};

template<template<typename...> typename P, typename T, typename R, typename... S>
struct apply_list<P,cons<T,R>,S...> : apply_list<P,R,S...,T>{};

template<typename... T>
struct apply_collect : nil{};

template<typename T, typename... TS>
struct apply_collect<T,TS...> : T{};

template<typename T, typename S, typename... TS>
struct apply_collect<T,S,TS...> : cons<T,apply_collect<S,TS...>::type>{};

template<typename... T>
using apply_collect_t = typename apply_collect<T...>::type;

template<template<typename...> typename P, typename... S>
struct apply : apply_list<P,apply_collect_t<S...>>{};

template<template<typename...> typename P, typename... S>
using apply_t = typename apply<P,S...>::type;

template<template<typename...> typename P>
struct apply_c{
    template<typename... S>
    using type = apply_t<P,S...>;
};

//---------------------------------------------------------------------------------

/**
*   Retorna false_type si algun elemento de la lista es false_type
*/
template<typename... TS>
struct and : std::true_type{};

template<typename T, typename... TS>
struct and<T,TS...> : and<TS...>{};

template<typename... TS>
struct and<std::false_type,TS...> : std::false_type{};

template<typename... TS>
using and_t = typename and<L>::type;

//---------------------------------------------------------------------------------

/**
*   Retorna false_type si algun elemento de la lista es false_type
*/
template<typename... TS>
struct or : std::false_type{};

template<typename T, typename... TS>
struct or<T,TS...> : std::true_type{};

template<typename... TS>
struct or<std::false_type,TS...> : or<TS...>{};

template<typename... TS>
using or_t = typename or<L>::type;

//---------------------------------------------------------------------------------

/**
*   Retorna una lista donde cada elemento es una lista que contiene dos elementos que estaban
*   en la misma posicion en las listas iniciales.
*/
template<typename L, typename S>
struct zip : nil{};

template<typename P, typename Q, typename R, typename S>
struct zip<cons<P,Q>,cons<R,S>> : cons<list_t<P,R>,typename zip<Q,S>::type>{};

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
    using type = std::add_const_t<typename slice_type_aux<std::is_const_t<newN>,std::is_volatile_t<newN>,std::is_lvalue_reference_t<newN>,
                                                          std::is_rvalue_reference_t<newN>,std::is_pointer_t<newN>,newN,S>::type>;
};

template<typename IsRval, typename IsPtr, typename N, typename S>
struct slice_type_aux<std::false_type,std::false_type,std::true_type,IsRval,IsPtr,N,S>{
    using newN = std::remove_reference_t<N>;
    using type = std::add_const_t<typename slice_type_aux<std::is_const_t<newN>,std::is_volatile_t<newN>,std::is_lvalue_reference_t<newN>,
                                                          std::is_rvalue_reference_t<newN>,std::is_pointer_t<newN>,newN,S>::type>;
};

template<typename IsPtr, typename N, typename S>
struct slice_type_aux<std::false_type,std::false_type,std::false_type,std::true_type,IsPtr,N,S>{
    using newN = std::remove_reference_t<N>;
    using type = std::add_const_t<typename slice_type_aux<std::is_const_t<newN>,std::is_volatile_t<newN>,std::is_lvalue_reference_t<newN>,
                                                          std::is_rvalue_reference_t<newN>,std::is_pointer_t<newN>,newN,S>::type>;
};

template<typename N, typename S>
struct slice_type_aux<std::false_type,std::false_type,std::false_type,std::false_type,std::true_type,N,S>{
    using newN = std::remove_pointer_t<N>;
    using type = std::add_const_t<typename slice_type_aux<std::is_const_t<newN>,std::is_volatile_t<newN>,std::is_lvalue_reference_t<newN>,
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
*   Comprueba si un tipo de dato es virtual.
*   Debe ser una referencia o un puntero a un tipo polimorfico (que contiene algun metodo virtual).
*/
template<typename T>
struct is_virtual : or<std::is_polymorphic_t<std::remove_pointer_t<T>>,std::is_polymorphic_t<std::remove_reference_t<T>>>{};

template<typename T>
using is_virtual_t = is_virtual_valid<T>::type;

template<typename T>
static constexpr bool is_virtual_v = is_virtual_valid<T>::value;

//---------------------------------------------------------------------------------

/**
*   Contenedor de un tipo virtual usado para la plantilla de la tabla omm (method_template).
*   El tipo T debe verificar is_virtual_valid.
*/
template<typename T>
struct virtual_type{
    using type = virtual_type<T>;
};

//---------------------------------------------------------------------------------

/**
*   Extrae el tipo dentro virtual_type
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
*   Interfaz para el usuario.
*   El tipo usado debe ser un tipo virtual, es decir, un puntero o una referencia a un tipo polimorfico (que tenga al menos un metodo virtual).
*/
template<typename T>
using Virtual = std::enable_if_t<is_virtual_v<T>,virtual_type<T>>;

//---------------------------------------------------------------------------------

/**
*   Traslada virtual_type, punteros, referencias y cualificadores a otro tipo.
*/
template<typename VT, typename S>
struct slice_virtual_type : virtual_type<slyce_type_t<open_virtual_type_t<VT>,S>>{};

//---------------------------------------------------------------------------------

/**
*   Elimina los elementos de la lista D que no sean tipos derivados de B.
*/
template<typename B, typename D>
struct filter_derived_types_of : remove_if_not<is_base_of_c<B>::template type,D>{};


//---------------------------------------------------------------------------------
//--------------------------------- Base of many ----------------------------------
//---------------------------------------------------------------------------------

/**
*   Un base_of_many es una lista cuyo primer elemento es una clase base del resto de elementos.
*/

//---------------------------------------------------------------------------------

/**
*   Retorna la cantidad de tipos derivados de un base_of_many
*/
template<typename BA>
using length_of_derived = length<cdr_t<BA>>;

template<typename BA>
using length_of_derived_t = length_of_derived<BA>::type;

template<typename BA>
using length_of_derived_v = length_of_derived<BA>::value;

//---------------------------------------------------------------------------------

/**
*   Dado un indice, devuelve el tipo derivado que ocupa dicha posicion
*/
template<typename BA, typename K>
using nth_derived = nth<cdr_t<BA>>;

template<typename BA>
using nth_derived_t = typename nth_derived<BA>::type;

//---------------------------------------------------------------------------------

/**
*   Busca el tipo que se corresponde con un type_info y devuelve su posicion
*/
template<typename BA>
struct search_index_base{
    static int call(const std::type_info& info){
        return 0;
    }
};


template<typename B, typename D>
struct search_index_base<cons<B,D>>{
    static int call(const std::type_info& info){
        if (info == typeid(car_t<D>))
            return 0;
        else
            return 1+search_index_base<cons<B,cdr_t<D>>>::call(info);
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

//---------------------------------------------------------------------------------

/**
*   Transforma un virtual signature en un template
*/
template<typename VT>
struct vsign_to_sign : mapcar<open_virtual_type_if_virtual,VT>{};

template<typename VT>
using vsign_to_sign_t = typename vsign_to_sign<VT>::type;

//---------------------------------------------------------------------------------


/**
*   Transforma un virtual base signature en un virtual derived signature
*/
template<typename VBS, typename D>   // TODO : Hacer unzip
struct vbsign_to_vdsign : zip_t<VT>{
    using type = virtual_method_template<typename M::type_ret,typename slice_virtual_types<typename M::type_args,D>::type>;
};

//
///**
//*   Cambia los tipos virtuales de un virtual_method_template por los tipos derivados de D y los extrae
//*/
//template<Virtual_method_template M, Type_list D>
//struct extract_slice_method_virtual_types{
//    using type = method_template<typename M::type_ret,typename extract_slice_virtual_types<typename M::type_args,D>::type>;
//};
//
//---------------------------------------------------------------------------------
//------------------------------- Method template ---------------------------------
//---------------------------------------------------------------------------------

/**
*   Un template es una lista cuyo primer elemento es el tipo de retorno de una funcion
*   y el resto de elementos son los tipos de los argumentos sin incluir ningun virtual_type.
*/

//---------------------------------------------------------------------------------

/**
*   Retorna el tipo de una funcion a partir de un template
*/
template<typename R, typename... A>
struct collection_to_function_type{
    using type = R(A...);
};

template<typename L>
struct template_to_function_type{
    using type = apply_t<collection_to_function_type,L>;
};

template<typename L>
using template_to_function_type_t = typename template_to_function_type<L>::type;

//---------------------------------------------------------------------------------
//
///**
//*   Comprueba si es posible llamar a una funcion con ciertos argumentos
//*/
//template<typename F, typename... NS>
//concept has_valid_implementation = requires (NS... args){
//    F::call(args...);
//};
//
///**
//*   Genera un puntero a una funcion que llama al metodo correspondiente tras realizar un casting
//*/
//template<Method_template O, Method_template N, typename F>
//struct make_function_cell;
//
//template<typename R, typename... OS, typename... NS, typename F>
//requires has_valid_implementation<F,NS...>
//struct make_function_cell<method_template<R,type_list<OS...>>,method_template<R,type_list<NS...>>,F>{
//    static R value(OS... args){
//        return F::call(static_cast<NS>(args)...);
//    }
//};
//
//template<typename R, typename... OS, typename... NS, typename F>
//requires (!has_valid_implementation<F,NS...>)
//struct make_function_cell<method_template<R,type_list<OS...>>,method_template<R,type_list<NS...>>,F>{
//    static constexpr std::nullptr_t value = nullptr;
//};
//


//---------------------------------------------------------------------------------
//-------------------------------- Combinations -----------------------------------
//---------------------------------------------------------------------------------

/**
*   Inserta un elemento en cada lista que se encuentra en una lista
*/
template<typename T, typename LL>
using cons_all = mapcar<cons_c<T>::template type,LL>{};

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
struct cartesian_product_aux{} : apply<append,mapcar_t<cons_all_c_inv<M>::template type,L>>{};

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
template<typename M>
struct make_combinations : apply<cartesian_product,mapcar_t<less_numbers,M>>{};

template<typename M>
using make_combinations_t = typename make_combinations<M>::type;

//---------------------------------------------------------------------------------
//------------------------------- Type id converter -------------------------------
//---------------------------------------------------------------------------------

/**
*   Un tipe_id es una lista de base_of_many
*/

//---------------------------------------------------------------------------------

/**
*   Retorna una lista de tipos cuyos indices en el type_id son los que se pasan como parametro
*/
template<typename Tid, typename L>      // usar zip + apply_c
struct indices_to_types : mapcar<apply_c<nth_derived>::template type,zip_t<Tid,L>>{};

template<typename Tid, typename L>
using indices_to_types_t = typename indices_to_types<Tid,L>::type;

template<typename Tid>
struct indices_to_types_c{
    template<typename L>
    using type = indices_to_types_t<Tid,L>;
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
template<typename Tid>
struct make_derived_combinations : mapcar<indices_to_types_c<Tid>::template type,make_combinations_t<length_of_each_base_t<Tid>>>{};

template<typename Tid>
using make_derived_combinations_t = typename make_derived_combinations<Tid>::type;


////---------------------------------------------------------------------------------
//
//template<Type_id C>
//struct get_index_multiplier_aux{
//    static constexpr int value = 1;
//};
//
//template<Base_of_any B, Base_of_any... BS>
//struct get_index_multiplier_aux<type_id<B,BS...>>{
//    static constexpr int value = length_of_base<B>::value*get_index_multiplier_aux<type_id<BS...>>::value;
//};
//
//template<Type_id C>
//struct get_index_multiplier;
//
//template<Base_of_any B, Base_of_any... BS>
//struct get_index_multiplier<type_id<B,BS...>>{
//    static constexpr int value = get_index_multiplier_aux<type_id<BS...>>::value;
//};
//
////---------------------------------------------------------------------------------
////---------------------------------------------------------------------------------
////--------------------------------- Get index -------------------------------------
////---------------------------------------------------------------------------------
////---------------------------------------------------------------------------------
//
//template<Type_id C, typename M>
//struct get_index_aux_3;
//
//template<typename M>
//struct get_index_aux_3<type_id<>,M>{
//    template<typename... AS>
//    static int call(int curr_ind, AS&&... as){
//        return curr_ind;
//    }
//};
//
//template<Base_of_any B, Base_of_any... BS, typename R,typename T, typename... TS>
//requires (!is_virtual_type<T>::value)
//struct get_index_aux_3<type_id<B,BS...>,virtual_method_template<R,type_list<T,TS...>>>{
//    template<typename A, typename... AS>
//    static int call(int curr_ind, A&& a, AS&&... as){
//        return get_index_aux_3<type_id<B,BS...>,virtual_method_template<R,type_list<TS...>>>::call(curr_ind,std::forward<AS>(as)...);
//    }
//};
//
//template<Base_of_any B, Base_of_any... BS, typename R, Virtual_type T, typename... TS>
//requires (std::is_reference<typename T::type>::value)
//struct get_index_aux_3<type_id<B,BS...>,virtual_method_template<R,type_list<T,TS...>>>{
//    template<typename A, typename... AS>
//    static int call(int curr_ind, A&& a, AS&&... as){
//        return get_index_aux_3<type_id<BS...>,virtual_method_template<R,type_list<TS...>>>::call(curr_ind+search_index_base<B>::call(typeid(a))*get_index_multiplier<type_id<B,BS...>>::value,std::forward<AS>(as)...);
//    }
//};
//
//template<Base_of_any B, Base_of_any... BS, typename R,Virtual_type T, typename... TS>
//requires (std::is_pointer<typename T::type>::value)
//struct get_index_aux_3<type_id<B,BS...>,virtual_method_template<R,type_list<T,TS...>>>{
//    template<typename A, typename... AS>
//    static int call(int curr_ind, A&& a, AS&&... as){
//        return get_index_aux_3<type_id<BS...>,virtual_method_template<R,type_list<TS...>>>::call(curr_ind+search_index_base<B>::call(typeid(*a))*get_index_multiplier<type_id<B,BS...>>::value,std::forward<AS>(as)...);
//    }
//};
//
//template<Type_id C, Virtual_method_template M>
//struct get_index_aux_2;
//
//template<Base_of_any... BS, Virtual_method_template M>
//struct get_index_aux_2<type_id<BS...>,M>{
//    template<typename A, typename... AS>
//    static int call(A&& a, AS&&... as){
//        return get_index_aux_3<type_id<BS...>,M>::call(0,std::forward<A>(a),std::forward<AS>(as)...);
//    }
//};
//
//template<Type_list D, Virtual_method_template M>
//struct get_index_aux{
//    template<typename A, typename... AS>
//    static int call(A&& a, AS&&... as){
//        return get_index_aux_2<typename create_type_id<typename get_virtual_core_types<typename M::type_args>::type,D>::type,M>::call(std::forward<A>(a),std::forward<AS>(as)...);
//    }
//};
//
//template<typename T, typename... DS>
//struct get_index{
//    template<typename A, typename... AS>
//    static int call(A&& a, AS&&... as){
//        return get_index_aux<typename create_type_list<DS...>::type,typename create_virtual_method_template<T>::type>::call(std::forward<A>(a),std::forward<AS>(as)...);
//    }
//};
//
////---------------------------------------------------------------------------------
////---------------------------------------------------------------------------------
////------------------------------- Generate table ----------------------------------
////---------------------------------------------------------------------------------
////---------------------------------------------------------------------------------
//
//template<Type_list D, Virtual_method_template O, typename F>
//struct generate_table_aux3;
//
//template<Type_list... DS, Virtual_method_template O, typename F>
//struct generate_table_aux3<type_list<DS...>,O,F>{
//    static constexpr std::add_pointer<typename method_template_to_function_type<typename create_method_template<O>::type>::type>::type value[] =
//                    {make_function_cell<typename create_method_template<O>::type,typename extract_slice_method_virtual_types<O,DS>::type,F>::value...};
//};
//
//template<Type_id C, Virtual_method_template O, typename F>
//struct generate_table_aux2{
//    static constexpr auto value = generate_table_aux3<typename make_derived_combinations<C>::type,O,F>::value;
//};
//
//template<Virtual_method_template O, Type_list D, typename F>
//struct generate_table_aux{
//    static constexpr auto value = generate_table_aux2<typename create_type_id<typename get_virtual_core_types<typename O::type_args>::type,D>::type,O,F>::value;
//
//};
//
//template<typename F, typename T, typename... DS>
//struct generate_table{
//    static constexpr auto value = generate_table_aux<typename create_virtual_method_template<T>::type,typename create_type_list<DS...>::type,F>::value;
//};
//
//
////---------------------------------------------------------------------------------
////---------------------------------------------------------------------------------
////---------------------------------------------------------------------------------
////--------------------------------- Table omm -------------------------------------
////---------------------------------------------------------------------------------
////---------------------------------------------------------------------------------
////---------------------------------------------------------------------------------
//
//template<typename F, typename T, typename... DS>
//struct table_omm{
//    template<typename... AS>
//    static auto call(AS&&... as){
//        return generate_table<F,T,DS...>::value[get_index<T,DS...>::call(std::forward<AS>(as)...)](std::forward<AS>(as)...);
//    }
//};


#endif // OMM_H_INCLUDED
