#ifndef OMM_H_INCLUDED
#define OMM_H_INCLUDED

#include <typeinfo>
#include <algorithm>

//---------------------------------------------------------------------------------
//----------------------------------- Auxiliar ------------------------------------
//---------------------------------------------------------------------------------

/**
*   Elimina referencias, punteros y cualificadores cv de un tipo de dato (int**const volatile*const** -> int)
*/
template<bool isReference, bool isPointer, typename T>
struct core_type_aux{
    using type = typename std::remove_cv<T>::type;
};

template<bool isPointer, typename T>
struct core_type_aux<true,isPointer,T>{
    using newType = std::remove_cv_t<std::remove_reference_t<T>>;
    using type = typename core_type_aux<std::is_reference_v<newType>,std::is_pointer_v<newType>,newType>::type;
};

template<typename T>
struct core_type_aux<false,true,T>{
    using newType = std::remove_pointer_t<T>;
    using type = typename core_type_aux<std::is_reference_v<newType>,std::is_pointer_v<newType>,newType>::type;
};

template<typename T>
struct core_type{
    using type = typename core_type_aux<std::is_reference_v<T>,std::is_pointer_v<T>,T>::type;
};

template<typename T>
using core_type_t = typename core_type<T>::type;

//---------------------------------------------------------------------------------



/**
*   Traslada referencias, punteros y cualificadores cv de un tipo origen a un destino
*   ( [int*const**volatile*&, float] -> float*const**volatile*& )
*/
template<bool isConst, bool isVolatile, bool isLvalue, bool isRvalue, bool isPointer, typename N, typename S>
struct slice_type_aux{
    using type = S;
};

template<bool isVolatile, bool isLvalue, bool isRvalue, bool isPointer, typename N, typename S>
struct slice_type_aux<true,isVolatile,isLvalue,isRvalue,isPointer,N,S>{
    using newN = std::remove_const_t<N>;
    using type = std::add_const_t<typename slice_type_aux<std::is_const_v<newN>,std::is_volatile_v<newN>,std::is_lvalue_reference_v<newN>,
                                                          std::is_rvalue_reference_v<newN>,std::is_pointer_v<newN>,newN,S>::type>;
};

template<bool isLvalue, bool isRvalue, bool isPointer, typename N, typename S>
struct slice_type_aux<false,true,isLvalue,isRvalue,isPointer,N,S>{
    using newN = std::remove_volatile_t<N>;
    using type = std::add_volatile_t<typename slice_type_aux<std::is_const_v<newN>,std::is_volatile_v<newN>,std::is_lvalue_reference_v<newN>,
                                                          std::is_rvalue_reference_v<newN>,std::is_pointer_v<newN>,newN,S>::type>;
};

template<bool isRvalue, bool isPointer, typename N, typename S>
struct slice_type_aux<false,false,true,isRvalue,isPointer,N,S>{
    using newN = std::remove_reference_t<N>;
    using type = std::add_lvalue_reference_t<typename slice_type_aux<std::is_const_v<newN>,std::is_volatile_v<newN>,std::is_lvalue_reference_v<newN>,
                                                                     std::is_rvalue_reference_v<newN>,std::is_pointer_v<newN>,newN,S>::type>;
};

template<bool isPointer, typename N, typename S>
struct slice_type_aux<false,false,false,true,isPointer,N,S>{
    using newN = std::remove_reference_t<N>;
    using type = std::add_rvalue_reference_t<typename slice_type_aux<std::is_const_v<newN>,std::is_volatile_v<newN>,std::is_lvalue_reference_v<newN>,
                                                                     std::is_rvalue_reference_v<newN>,std::is_pointer_v<newN>,newN,S>::type>;
};

template<typename N, typename S>
struct slice_type_aux<false,false,false,false,true,N,S>{
    using newN = std::remove_pointer_t<N>;
    using type = std::add_pointer_t<typename slice_type_aux<std::is_const_v<newN>,std::is_volatile_v<newN>,std::is_lvalue_reference_v<newN>,
                                                            std::is_rvalue_reference_v<newN>,std::is_pointer_v<newN>,newN,S>::type>;
};

template<typename N, typename S>
struct slice_type{
    using type = typename slice_type_aux<std::is_const_v<N>,std::is_volatile_v<N>,std::is_lvalue_reference_v<N>,
                                         std::is_rvalue_reference_v<N>,std::is_pointer_v<N>,N,S>::type;
};

template<typename N, typename S>
using slice_type_t = typename slice_type<N,S>::type;

////---------------------------------------------------------------------------------
////----------------------------------- Virtual -------------------------------------
////---------------------------------------------------------------------------------

/**
*   Comprueba si un tipo de dato es valido como tipo virtual en la tabla
*   Debe ser una referencia o un puntero a un tipo polimorfico (que contiene algun metodo virtual)
*/
template<typename T>
struct is_virtual_valid{
    static constexpr bool value = std::is_polymorphic_v<std::remove_pointer_t<T>> || std::is_polymorphic_v<std::remove_reference_t<T>>;
};

template<typename T>
static constexpr bool is_virtual_valid_v = is_virtual_valid<T>::value;

////---------------------------------------------------------------------------------

/**
*   Contenedor de un tipo virtual usado para la plantilla de la tabla omm (method_template).
*   El tipo T debe verificar is_virtual_valid.
*/
template<typename T>
struct virtual_type{
    using type = T;
};

/**
*   Comprueba si un tipo es un virtual_type. Esto es, que sea una clase virtual_type con un tipo que cumpla is_virtual_valid.
*/
//template<typename T>
//struct is_virtual_type{
//    static constexpr bool value = false;
//};
//
//template<typename T>
//struct is_virtual_type<virtual_type<T>>{
//    static constexpr bool value = is_virtual_valid_v<T>;
//};
//
//template<typename T>
//static constexpr bool is_virtual_type_v = is_virtual_type<T>::value;

////---------------------------------------------------------------------------------

/**
*   Interfaz para el usuario.
*   El tipo usado debe ser un tipo virtual, es decir, un puntero o una referencia a un tipo polimorfico (que tenga al menos un metodo virtual).
*/
template<typename T>
using Virtual = std::enable_if_t<is_virtual_valid_v<T>,virtual_type<T>>;


/**
*   Cambia el tipo principal de un tipo virtual
*/
template<typename T, typename D>
struct slice_virtual_type;

template<typename T, typename D>
struct slice_virtual_type<virtual_type<T>,D>{
    using type = virtual_type<slice_type_t<T,D>>;
};


////---------------------------------------------------------------------------------
////--------------------------------- List type -------------------------------------
////---------------------------------------------------------------------------------

/**
*   Lista de tipos
*/
template<typename... TS>
struct flist;

/**
*   Lista vacia
*/
using nil = flist<>;

/**
*   Comprueba si un tipo es una flist
*/
template<typename L>
struct is_flist{
    static constexpr bool value = false;
};

template<typename... TS>
struct is_flist<flist<TS...>>{
    static constexpr bool value = true;
};

template<typename T>
static constexpr bool is_flist_v = is_flist<T>::value;

////---------------------------------------------------------------------------------

/**
*   Comprueba si una type_list esta vacia
*/
template<typename T>
struct null{
    static constexpr bool value = true;
};

template<typename T, typename... TS>
struct null<flist<T,TS...>>{
    static constexpr bool value = false;
};

template<T>
static constexpr bool null_v = null<T>::value;


/**
*   Inserta un elemento al inicio de una lista
*/
template<typename T, typename L>
struct cons;

template<typename T, typename... TS>
struct cons<T,flist<TS...>>{
    using type = flist<T,TS...>;
};

template<typename T, typename L>
using cons_t = cons<T,L>::type;


/**
*   Devuelve el primer elemento de una lista
*/
template<typename L>
struct car;

template<typename T, typename... TS>
struct car<flist<T,TS...>>{
    using type = T;
};

template<typename L>
using car_t = car<L>::type;


/**
*   Elimina el primer elemento de una lista
*/
template<typename L>
struct cdr{
    using type = nil;
};

template<typename T, typename... TS>
struct cdr<flist<T,TS...>>{
    using type = flist<TS...>;
};

template<typename L>
using cdr_t = cdr<L>::type;


/**
*   Devuelve el numero de elementos de una lista
*/
template<typename L>
struct length{
    static constexpr size_t value = 0;
};

template<typename T, typename... TS>
struct length<flist<T,TS...>>{
    static constexpr size_t value = 1+length<flist<TS...>>::value;
};

template<typename L>
static constexpr size_t length_v = length<L>::value;


/**
*   Retorna true si encuentra un elemento presente en una lista
*/
template<typename T, typename L>
struct member{
    static constexpr bool value = false;
};

template<typename T, typename R, typename... RS>
struct member<T,flist<R,RS...>>{
    static constexpr bool value = member<T,type_list<RS...>>::value;
};

template<typename T, typename... RS>
struct member<T,flist<T,RS...>>{
    static constexpr bool value = true;
};

template<typename T, typename L>
static constexpr bool member_v = member<T,L>::value;


/**
*   Retorna el indice de un tipo en una type_list
*/
template<Type_list L, typename T, int k>
struct member_at_type_aux{
    static constexpr int value = member_at_type_aux<typename cdr_type<L>::type,T,k+1>::value;
};

template<typename... LS, typename T, int k>
struct member_at_type_aux<type_list<T,LS...>,T,k>{
    static constexpr int value = k;
};

template<Type_list L, typename T>
requires has_type<L,T>::value
struct position{
    static constexpr int value = member_at_type_aux<L,T,0>::value;
};

//
///**
//*   Retorna el tipo que se encuentra en la posicion k-esima
//*/
//template<Type_list L, int k>
//requires (k<length_type<L>::value)
//struct ref_type{
//    using type = typename ref_type<typename cdr_type<L>::type,k-1>::type;
//};
//
//template<Type_list L>
//requires (!is_null_type<L>::value)
//struct ref_type<L,0>{
//    using type = typename car_type<L>::type;
//};
//
//
//template<template<typename> typename P, Type_list L>
//struct map_type;
//
//template<template<typename> typename P, typename... TS>
//struct map_type<P,type_list<TS...>>{
//    using type = type_list<typename P<TS>::type...>;
//};
//
//
///**
//*   Retorna false si el predicado que se pasa retorna false para algun elemento de la lista
//*/
//template<template<typename> typename P, Type_list L>
//struct and_map_type{
//    static constexpr bool value = true;
//};
//
//template<template<typename> typename P, typename T, typename... TS>
//struct and_map_type<P,type_list<T,TS...>>{
//    static constexpr bool value = P<T>::value && and_map_type<P,type_list<TS...>>::value;
//};
//
//
///**
//*   Retorna true si el predicado que se pasa retorna true para algun elemento de la lista
//*/
//template<template<typename> typename P, Type_list L>
//struct or_map_type{
//    static constexpr bool value = false;
//};
//
//template<template<typename> typename P, typename T, typename... TS>
//struct or_map_type<P,type_list<T,TS...>>{
//    static constexpr bool value = P<T>::value || or_map_type<P,type_list<TS...>>::value;
//};
//
////---------------------------------------------------------------------------------
//
///**
//*   Crea una lista con los tipos que son derivados de un tipo base
//*/
//template<Type_list L, typename B, Type_list D>
//struct make_derived_list_aux;
//
//template<Type_list L, typename B>
//requires is_null_type<L>::value
//struct make_derived_list_aux<L,B,type_list<>>{
//    using type = L;
//};
//
//template<Type_list L, typename B, Type_list D>
//requires std::is_base_of<B,typename car_type<D>::type>::value
//struct make_derived_list_aux<L,B,D>{
//    using type = typename cons_type<typename car_type<D>::type,typename make_derived_list_aux<L,B,typename cdr_type<D>::type>::type>::type;
//};
//
//template<Type_list L, typename B, Type_list D>
//requires (!std::is_base_of<B,typename car_type<D>::type>::value)
//struct make_derived_list_aux<L,B,D>{
//    using type = typename make_derived_list_aux<L,B,typename cdr_type<D>::type>::type;
//};
//
//template<typename B, Type_list D>
//struct make_derived_list{
//    using type = typename make_derived_list_aux<type_list<>,B,D>::type;
//};
//
////---------------------------------------------------------------------------------
//
///**
//*   Retorna una lista con los tipos virtuales extraidos en su forma core
//*/
//template<Type_list N, Type_list L>
//struct get_virtual_core_types_aux{
//    using type = N;
//};
//
//template<Type_list N, Virtual_type T, typename... TS>
//struct get_virtual_core_types_aux<N,type_list<T,TS...>>{
//    using type = typename cons_type<typename core_type<typename T::type>::type,typename get_virtual_core_types_aux<N,type_list<TS...>>::type>::type;
//};
//
//template<Type_list N, typename T, typename... TS>
//struct get_virtual_core_types_aux<N,type_list<T,TS...>>{
//    using type = typename get_virtual_core_types_aux<N,type_list<TS...>>::type;
//};
//
//template<Type_list L>
//struct get_virtual_core_types{
//    using type = typename get_virtual_core_types_aux<type_list<>,L>::type;
//};
//
////---------------------------------------------------------------------------------
//
///**
//*   Cambia el contenido de los tipos virtuales por los tipos derivados
//*/
//template<Type_list L, Type_list D>
//struct slice_virtual_types{
//    using type = typename create_type_list<>::type;
//};
//
//template<typename L, typename... LS, Type_list D>
//struct slice_virtual_types<type_list<L,LS...>,D>{
//    using type = typename cons_type<L,typename slice_virtual_types<type_list<LS...>,D>::type>::type;
//};
//
//template<Virtual_type L, typename... LS, Type_list D>
//struct slice_virtual_types<type_list<L,LS...>,D>{
//    using type = typename cons_type<typename slice_virtual_type<L,typename car_type<D>::type>::type,typename slice_virtual_types<type_list<LS...>, typename cdr_type<D>::type>::type>::type;
//};
//
//
///**
//*   Extrae el contenido de los tipos virtuales
//*/
//template<Type_list L>
//struct extract_virtual_types{
//    using type = typename create_type_list<>::type;
//};
//
//template<typename L, typename... LS>
//struct extract_virtual_types<type_list<L,LS...>>{
//    using type = typename cons_type<L,typename extract_virtual_types<type_list<LS...>>::type>::type;
//};
//
//template<Virtual_type L, typename... LS>
//struct extract_virtual_types<type_list<L,LS...>>{
//    using type = typename cons_type<typename L::type,typename extract_virtual_types<type_list<LS...>>::type>::type;
//};
//
//
///**
//*   Cambia el contenido de los tipos virtuales por los tipos derivados y los extrae de sus contenedores virtuales
//*/
//template<Type_list L, Type_list D>
//struct extract_slice_virtual_types{
//    using type = typename create_type_list<>::type;
//};
//
//template<typename L, typename... LS, Type_list D>
//struct extract_slice_virtual_types<type_list<L,LS...>,D>{
//    using type = typename cons_type<L,typename extract_slice_virtual_types<type_list<LS...>,D>::type>::type;
//};
//
//template<Virtual_type L, typename... LS, Type_list D>
//struct extract_slice_virtual_types<type_list<L,LS...>,D>{
//    using type = typename cons_type<typename slice_type<typename L::type,typename car_type<D>::type>::type,typename extract_slice_virtual_types<type_list<LS...>, typename cdr_type<D>::type>::type>::type;
//};
//
////---------------------------------------------------------------------------------
//
///**
//*   Retorna el tipo de una funcion a partir del tipo de retorno y el tipo de los argumentos
//*/
//template<typename R, Type_list L>
//struct type_list_to_function_type;
//
//template<typename R, typename... LS>
//struct type_list_to_function_type<R,type_list<LS...>>{
//    using type = R(LS...);
//};
//
////---------------------------------------------------------------------------------
////--------------------------------- Base of any -----------------------------------
////---------------------------------------------------------------------------------
//
///**
//*   Tipo de dato que guarda un tipo base y una lista de tipos derivados
//*/
//template<typename B, Type_list D>
//struct base_of_any{
//    using type_base = B;
//    using type_derived = D;
//};
//
//
//template<typename B, Type_list D>
//struct create_base_of_any{
//    using type = base_of_any<B,typename make_derived_list<B,D>::type>;
//};
//
//
//template<typename T>
//struct is_base_of_any{
//    static constexpr bool value = false;
//};
//
//template<typename B>
//struct is_base_of_B{
//    template<typename D>
//    struct type{
//        static constexpr bool value = std::is_base_of<B,D>::value;
//    };
//};
//
//template<typename B, Type_list D>
//requires and_map_type<is_base_of_B<B>::template type,D>::value
//struct is_base_of_any<base_of_any<B,D>>{
//    static constexpr bool value = true;
//};
//
//template<typename B>
//concept Base_of_any = is_base_of_any<B>::value;
//
//
///**
//*   Retorna la cantidad de tipos derivados de un base_of_any
//*/
//template<Base_of_any B>
//struct length_of_base;
//
//template<typename B, Type_list D>
//struct length_of_base<base_of_any<B,D>>{
//    static constexpr int value = length_type<D>::value;
//};
//
////---------------------------------------------------------------------------------
//
///**
//*   Busca el tipo que se corresponde con un type_info y devuelve su posicion
//*/
//template<Base_of_any B>
//struct search_index_base{
//    static int call(const std::type_info& info){
//        return 0;
//    }
//};
//
//
//template<typename B, Type_list D>
//requires (!is_null_type<D>::value)
//struct search_index_base<base_of_any<B,D>>{
//    static int call(const std::type_info& info){
//        if (info == typeid(typename car_type<D>::type))
//            return 0;
//        else
//            return 1+search_index_base<base_of_any<B,typename cdr_type<D>::type>>::call(info);
//    }
//};
//
////---------------------------------------------------------------------------------
////--------------------------- Virtual method template -----------------------------
////---------------------------------------------------------------------------------
//
///**
//*   Extrae el tipo de los argumentos de un tipo de funcion
//*/
//template<typename T>
//requires std::is_function<T>::value
//struct extract_arg_types;
//
//template<typename R, typename... AS>
//struct extract_arg_types<R(AS...)>{
//    using type = typename create_type_list<AS...>::type;
//};
//
//
///**
//*   Extrae el tipo de retorno de un tipo de funcion
//*/
//template<typename T>
//requires std::is_function<T>::value
//struct extract_ret_type;
//
//template<typename R, typename... AS>
//struct extract_ret_type<R(AS...)>{
//    using type = R;
//};
//
//
///**
//*   Objeto que representa una plantilla con tipos virtuales
//*/
//template<typename R, Type_list A>
//struct virtual_method_template{
//    using type_ret = R;
//    using type_args = A;
//};
//
//
///**
//*   Constructor de un method template
//*/
//template<typename T>
//struct create_virtual_method_template{
//    using type = virtual_method_template<typename extract_ret_type<T>::type,typename extract_arg_types<T>::type>;
//};
//
//
///**
//*   Comprueba si una lista de tipos es una plantilla valida
//*/
//template<typename M>
//struct is_virtual_method_template;
//
//template<typename R, Type_list A>
//struct is_virtual_method_template<virtual_method_template<R,A>>{
//    static constexpr bool value = or_map_type<is_virtual_type,A>::value;
//};
//
//
///**
//*   Tipo que representa la plantilla de un metodo
//*/
//template<typename T>
//concept Virtual_method_template = is_virtual_method_template<T>::value;
//
//
///**
//*   Extrae los tipos virtuales de un virtual_method_template, por lo que devuelve un method_template
//*/
//template<typename R, Type_list A>
//struct method_template;
//
//template<Virtual_method_template M>
//struct extract_method_virtual_types{
//    using type = method_template<typename M::type_ret,typename extract_virtual_types<typename M::type_args>::type>;
//};
//
//
///**
//*   Cambia los tipos virtuales de un virtual_method_template por los tipos derivados de D
//*/
//template<Virtual_method_template M, Type_list D>
//struct slice_method_virtual_types{
//    using type = virtual_method_template<typename M::type_ret,typename slice_virtual_types<typename M::type_args,D>::type>;
//};
//
//
///**
//*   Cambia los tipos virtuales de un virtual_method_template por los tipos derivados de D y los extrae
//*/
//template<Virtual_method_template M, Type_list D>
//struct extract_slice_method_virtual_types{
//    using type = method_template<typename M::type_ret,typename extract_slice_virtual_types<typename M::type_args,D>::type>;
//};
//
////---------------------------------------------------------------------------------
////------------------------------- Method template ---------------------------------
////---------------------------------------------------------------------------------
//
//
///**
//*   Representa una plantilla sin tipos virtuales
//*/
//template<typename R, Type_list A>
//struct method_template{
//    using type_ret = R;
//    using type_args = A;
//};
//
///**
//*   Constructor de la plantilla sin tipos virtuales
//*/
//template<Virtual_method_template M>
//struct create_method_template{
//    using type = typename extract_method_virtual_types<M>::type;
//};
//
//
///**
//*   Comprueba si un tipo es un method template
//*/
//template<typename T>
//struct is_method_template{
//    static constexpr bool value = false;
//};
//
//template<typename R, Type_list A>
//struct is_method_template<method_template<R,A>>{
//    static constexpr bool value = true;
//};
//
//template<typename T>
//concept Method_template = is_method_template<T>::value;
//
//
///**
//*   Convierte un method template en el tipo de una funcion
//*/
//template<Method_template M>
//struct method_template_to_function_type{
//    using type = typename type_list_to_function_type<typename M::type_ret,typename M::type_args>::type;
//};
//
//
////---------------------------------------------------------------------------------
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
////---------------------------------------------------------------------------------
////---------------------------------- Int list -------------------------------------
////---------------------------------------------------------------------------------
//
///**
//*   Una lista de enteros
//*/
//template<int... ks>
//struct int_list;
//
//
//template<int... ks>
//struct create_int_list{
//    using type = int_list<ks...>;
//};
//
//
///**
//*   Comprueba si un tipo es una int_list
//*/
//template<typename L>
//struct is_int_list{
//    static constexpr bool value = false;
//};
//
//template<int... ks>
//struct is_int_list<int_list<ks...>>{
//    static constexpr bool value = true;
//};
//
//template<typename L>
//concept Int_list = is_int_list<L>::value;
//
//
///**
//*   Comprueba que dos listas de enteros son iguales
//*/
//template<Int_list L1, Int_list L2>
//struct equal_int{
//    static constexpr bool value = true;
//};
//
//template<int k, int... ks, int r, int... rs>
//struct equal_int<int_list<k,ks...>,int_list<r,rs...>>{
//    static constexpr bool value = false;
//};
//
//template<int k, int... ks, int... rs>
//struct equal_int<int_list<k,ks...>,int_list<k,rs...>>{
//    static constexpr bool value = equal_int<int_list<ks...>,int_list<rs...>>::value;
//};
//
//
///**
//*   Comprueba si una lista de enteros esta vacia
//*/
//template<Int_list L>
//struct is_null_int{
//    static constexpr bool value = true;
//};
//
//template<int k, int... ks>
//struct is_null_int<int_list<k,ks...>>{
//    static constexpr bool value = false;
//};
//
//
///**
//*   Inserta un entero al inicio
//*/
//template<int k, Int_list L>
//struct cons_int;
//
//template<int k, int... ks>
//struct cons_int<k,int_list<ks...>>{
//    using type = int_list<k,ks...>;
//};
//
//
///**
//*   Retorna el primer entero de la lista
//*/
//template<Int_list L>
//struct car_int;
//
//template<int k, int... ks>
//struct car_int<int_list<k,ks...>>{
//    static constexpr int value = k;
//};
//
//
///**
//*   Elimina el primer entero de la lista
//*/
//template<Int_list L>
//struct cdr_int;
//
//template<int k, int... ks>
//struct cdr_int<int_list<k,ks...>>{
//    using type = int_list<ks...>;
//};
//
//
///**
//*   Retorna el numero de enteros de una int_list
//*/
//template<Int_list L>
//struct length_int{
//    static constexpr int value = 0;
//};
//
//template<int k, int... ks>
//struct length_int<int_list<k,ks...>>{
//    static constexpr int value = 1+length_int<int_list<ks...>>::value;
//};
//
//
///**
//*   Ejecuta una fucnion sobre cada elemento y devuelve una int_list con los resultados
//*/
//template<template<int> typename F, Int_list L>
//struct map_int;
//
//template<template<int> typename F, int... ks>
//struct map_int<F,int_list<ks...>>{
//    using type = int_list<F<ks>::value...>;
//};
//
//
///**
//*   Concatena dos listas de enteros
//*/
//template<Int_list L1, Int_list L2>
//struct append_int;
//
//template<int... ks, int... rs>
//struct append_int<int_list<ks...>,int_list<rs...>>{
//    using type = int_list<ks...,rs...>;
//};
//
////---------------------------------------------------------------------------------
//
///**
//*   Crea una lista de ceros
//*/
//template<int k>
//struct make_zeros{
//    using type = typename cons_int<0,typename make_zeros<k-1>::type>::type;
//};
//
//template<>
//struct make_zeros<0>{
//    using type = typename create_int_list<>::type;
//};
//
////---------------------------------------------------------------------------------
//
///**
//*   Decrementa el ultimo digito que no sea cero, sustituyendo los ceros del final por los digitos de M.
//*/
//template<Int_list L, Int_list M>
//struct decrease_int_list;
//
//template<int k, Int_list M>
//struct decrease_int_list<int_list<k>,M>{
//    using type = int_list<k-1>;
//};
//
//template<int k, int j, int... ks, int r, int s, int... rs>
//requires (!equal_int<int_list<j,ks...>,typename make_zeros<length_int<int_list<j,ks...>>::value>::type>::value)
//struct decrease_int_list<int_list<k,j,ks...>,int_list<r,s,rs...>>{
//    using type = typename cons_int<k,typename decrease_int_list<int_list<j,ks...>,int_list<s,rs...>>::type>::type;
//};
//
//template<int k, int j, int... ks, int r, int s, int... rs>
//requires equal_int<int_list<j,ks...>,typename make_zeros<length_int<int_list<j,ks...>>::value>::type>::value
//struct decrease_int_list<int_list<k,j,ks...>,int_list<r,s,rs...>>{
//    using type = int_list<k-1,s,rs...>;
//};
//
////---------------------------------------------------------------------------------
//
///**
//*   Crea una lista de todas las combinaciones de numeros entre 0 y el numero indicado
//*   en la int_list pasada como parametro
//*/
//template<Type_list L, Int_list M>
//struct make_combinations_aux{
//    using type = L;
//};
//
//template<typename T, typename... TS, Int_list M>
//requires (!equal_int<T,typename make_zeros<length_int<T>::value>::type>::value)
//struct make_combinations_aux<type_list<T,TS...>,M>{
//    using type = typename make_combinations_aux<type_list<typename decrease_int_list<T,M>::type,T,TS...>,M>::type;
//};
//
//template<Int_list M>
//struct make_combinations{
//    using type = typename make_combinations_aux<type_list<M>,M>::type;
//};
//
////---------------------------------------------------------------------------------
////------------------------------- Type id converter -------------------------------
////---------------------------------------------------------------------------------
//
///**
//*   Tipo que representa un conversor entre un tipo y un identificador (o indice para la tabla)
//*/
//template<Base_of_any... BS>
//struct type_id;
//
///**
//*   Constructor de un type id
//*/
//template<Type_list B, Type_list D>
//struct create_type_id;
//
//template<typename... BS, Type_list D>
//struct create_type_id<type_list<BS...>,D>{
//    using type = type_id<typename create_base_of_any<BS,D>::type...>;
//};
//
//
///**
//*   Comprueba si un tipo es type id
//*/
//template<typename T>
//struct is_type_id{
//    static constexpr bool value = false;
//};
//
//template<Base_of_any... BS>
//struct is_type_id<type_id<BS...>>{
//    static constexpr bool value = true;
//};
//
//template<typename T>
//concept Type_id = is_type_id<T>::value;
//
//
///**
//*   Retorna el indice de un tipo D sabiendo que su tipo base es B
//*/
//template<Type_id C, typename B, typename D>
//struct type_to_index;
//
//template<Base_of_any B, Base_of_any... BS, typename Ba, typename D>
//requires std::same_as<typename B::type_base,Ba>
//struct type_to_index<type_id<B,BS...>,Ba,D>{
//    static constexpr int value = member_at_type<typename B::type_derived,D>::value;
//};
//
//template<Base_of_any B, Base_of_any... BS, typename Ba, typename D>
//requires (!std::same_as<typename B::type_base,Ba>)
//struct type_to_index<type_id<B,BS...>,Ba,D>{
//    static constexpr int value = type_to_index<type_id<BS...>,Ba,D>::value;
//};
//
//
///**
//*   Retorna el tipo que ocupa la posicion k sabiendo que su tipo base es B
//*/
//template<Type_id C, typename B, int k>
//struct index_to_type;
//
//template<Base_of_any B, Base_of_any... BS, typename Ba, int k>
//requires std::same_as<typename B::type_base,B>
//struct index_to_type<type_id<B,BS...>,Ba,k>{
//    using type = typename ref_type<typename B::type_derived,k>::type;
//};
//
//template<Base_of_any B, Base_of_any... BS, typename Ba, int k>
//requires (!std::same_as<typename B::type_base,B>)
//struct index_to_type<type_id<B,BS...>,Ba,k>{
//    using type = typename ref_type<typename B::type_derived,k>::type;
//};
//
//
///**
//*   Retorna una lista de tipos cuyos indices en la tabla son los que se pasan como parametro
//*/
//template<Type_id C, Int_list L>
//struct indices_to_types{
//    using type = type_list<>;
//};
//
//template<Base_of_any B, Base_of_any... BS, int k, int... ks>
//struct indices_to_types<type_id<B,BS...>,int_list<k,ks...>>{
//    using type = typename cons_type<typename index_to_type<type_id<B,BS...>,typename B::type_base,k>::type,typename indices_to_types<type_id<BS...>,int_list<ks...>>::type>::type;
//};
//
////---------------------------------------------------------------------------------
//
///**
//*   Retorna una lista con el numero de tipos derivados que contiene cada base
//*/
//template<Type_id C>
//struct length_of_each_base;
//
//template<Base_of_any... BS>
//struct length_of_each_base<type_id<BS...>>{
//    using type = int_list<length_of_base<BS>::value...>;
//};
//
////---------------------------------------------------------------------------------
//
///**
//*   Devuelve una lista con todas las posibles combinaciones de tipos derivados
//*/
//template<Type_id C, Type_list N>
//struct make_derived_combinations_aux;
//
//template<Type_id C, Int_list... NS>
//struct make_derived_combinations_aux<C,type_list<NS...>>{
//    using type = typename create_type_list<typename indices_to_types<C,NS>::type...>::type;
//};
//
//template<int k>
//struct sub1{
//    static constexpr int value = k-1;
//};
//
//template<Type_id C>
//struct make_derived_combinations{
//    using type = typename make_derived_combinations_aux<C,typename make_combinations<typename map_int<sub1,typename length_of_each_base<C>::type>::type>::type>::type;
//};
//
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
