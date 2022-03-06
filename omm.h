#ifndef OMM_H_INCLUDED
#define OMM_H_INCLUDED

#include <typeinfo>
#include <type_traits>
#include <algorithm>

/**
 This library allows the programmer to use open multi-methods.
 I was inspired by Jean-Louis Leroy's library named yomm2.

 In the first lines are defined some metatypes like int_constant, collection and tlist.
 I use them to make easier the metaprogramming. After all, using templates is functional
 programming, but tedious and annoying.

 The goal in this library is to create the omm table, an array containing function pointers
 that points to the method implementations. In order to achieve this, several metafunctions
 which operates with differents metatypes are needed. The diagram below shows the way the
 omm table is created.

    How to create the omm table:



            vbsign_to_bsign  -------------------------------> BS = tlist<R,BArgs> ---------------------
                            |                                                                          |
                            |                                                      *create_table_omm*  |
                            |                                                      make_function_cell  |
           ftype_to_sign    |                                                                          v
    ftype ---------------> VBS  *---------------------------------->* DSCOMB  *----------------->* omm_table  <----- F
                            |                                           *
                            |            *vbsign_to_dsign_combinations* ^
       get_base_core_types  |                          vbsign_to_dsign  |
                            |                                           |
                            v                                           *
                           BCL                                 -----> DCOMB
                            |                                 /         *
                            |    *make_derived_combinations* /          ^
                            |        -----------------------/           |
            create_type_id  |       /  indices_to_types                 |
                            v      /                                    *
               DCL ------> TID  ---------------------------------->  Indices
                                           *make_indices*



    Where:
        - ftype: It is the function type template. In it, the virtual types are specified.
                 * Example: void(Virtual<Base1*>,int,SomeClass,Virtual<const Base2&>)
                 + Actually, Virtual is turned into virtual_type.

        - DCL (Derived Core List): It is a list containing the classes which participate in the multiple dispatch.
                 * Example: tlist<Derived1,Derived2,Derived3>

        - F: It is an struct containing all the multi-method implementations. They have to be static function
             named implementation.

        - VBS (Virtual Base Signature): It is a tlist containing the type from ftype.
                 * Example: tlist<void,virtual_type<Base1*>,int,SomeClass,virtual_type<const Base2&>>

        - BCL (Base Core List): It is a list containing all the virtual classes from VBS, in their
                                core form, i.e. without cv qualifiers, references or pointers.
                 * Example: tlist<Base1,Base2>

        - BS (Base Signature): It is a list containing the types from VBS without the virtual_type wrappers.
                 * Example: tlist<void,Base1*,int,SomeClass,const Base2&>

        - TID: It is a list of lists. Each list has as first element a base core type, and the rest of the elements
               are their derived classes that appears in DCL.
                 * Example: tlist<tlist<Base1,Derived1>,tlist<Base2,Derived2,Derived3>>

        - Indices: It is a list containing all the possible tuples of indices of the omm table. Each tuple has the same length
               as TID, and contains numbers from 0 to the amount, minus 1, of derived types are in each list from TID.
                 * Example: tlist<tlist<0,0>,tlist<0,1>>
                 + Actually, each number is a int_contant type.

        - DCOMB: It is a list containing all the combinations of derived types that can be formed according
               to their position in TID and to the list of Indices.
                 * Example: tlist<tlist<Derived1,Derived2>,tlist<Derived1,Derived3>>

        - DSCOMB (Derived Signature Combinations): It is a list containing all the possible signatures that can be formed accordingly
               to the Combination Derived Type list. Actually, the VBS is used to turn the virtual_types into a derived type.
                 * Example: tlist<tlist<void,Derived1*,int,SomeClass,const Derived2&>,tlist<void,Derived1*,int,SomeClass,const Derived3&>>

        - table_omm: It is an array of pointers to the implementations.
*/

//---------------------------------------------------------------------------------

/**
*   Some helper aliases
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
//-------------------------------- INT_CONSTANT -----------------------------------
//---------------------------------------------------------------------------------

/**
*   Represents an integer.
*/
template<int k>
using int_constant = std::integral_constant<int,k>;

//---------------------------------------------------------------------------------

/**
*   The numbers 0 and 1.
*/
using zero = int_constant<0>;
using one = int_constant<1>;

//---------------------------------------------------------------------------------

/**
*   Performs the sum of several numbers.
*    - TS... : Each number is an int_constant type.
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
*   Add 1 to an int_constant.
*    - T : The int_constant to add 1.
*/
template<typename T>
using add1 = add<one,T>;

template<typename T>
using add1_t = typename add1<T>::type;

template<typename T>
static constexpr int add1_v = add1<T>::value;

//---------------------------------------------------------------------------------

/**
*   Subtract 1 to an int_constant.
*    - T : The int_constant to subtract 1.
*/
template<typename T>
using sub1 = add<int_constant<-1>,T>;

template<typename T>
using sub1_t = typename sub1<T>::type;

template<typename T>
static constexpr int sub1_v = sub1<T>::value;


//---------------------------------------------------------------------------------
//--------------------------------- COLLECTION ------------------------------------
//---------------------------------------------------------------------------------

/**
*   Represents a collection of types. It is used to perform variadic template unpacking.
*/
template<typename... S>
struct collection{
    using type = collection<S...>;
};

//---------------------------------------------------------------------------------

/**
*   Applies a metafunction receiving all the types contained in a collection
*    - P : The metafunction to be applied.
*    - C : The collection containint the arguments of P.
*/
template<template<typename...> typename P, typename C>
struct apply_collection{};

template<template<typename...> typename P, typename... S>
struct apply_collection<P,collection<S...>> : P<S...>{};

template<template<typename...> typename P, typename C>
using apply_collection_t = typename apply_collection<P,C>::type;


//---------------------------------------------------------------------------------
//------------------------------------ TLIST --------------------------------------
//---------------------------------------------------------------------------------

/**
*   Represents an empty list.
*/
struct nil{
    using type = nil;
};

//---------------------------------------------------------------------------------

/**
*   Represents a cons cell. A tlist is defined as a cons cell that its rigth cell
*   is another list (empty or not).
*    - T : The left cell.
*    - S : The right cell.
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
*   Helper function to create tlist.
*    - TS... : The types to be in the created list.
*/
template<typename... TS>
struct tlist : nil{};

template<typename T, typename... TS>
struct tlist<T,TS...> : cons<T,typename tlist<TS...>::type>{};

template<typename... TS>
using tlist_t = typename tlist<TS...>::type;

//---------------------------------------------------------------------------------

/**
*   Transforms a tlist into a collection.
*    - T : The list to turn into a collection.
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
*   Returns the left cell of a cons type.
*    - L : The cons cell to retrive its left cell.
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
*   Returns the right cell of a cons cell.
*    - L : The cons cell to retrieve its right cell.
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
*   Concatenates several lists.
*    - S... : The lists to be concatenated.
*/
template<typename... S>
struct append : nil{};

template<typename... S>
struct append<nil,S...> : append<S...>{};

template<typename T, typename R, typename... S>
struct append<cons<T,R>,S...> : cons<T,typename append<R,S...>::type>{};

template<typename... S>
using append_t = typename append<S...>::type;

//---------------------------------------------------------------------------------

/**
*   Returns the length of a list, i.e. the number of its elements.
*    - L : The list to retrieve its length.
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
*   Returns the nth-element of a list.
*    - L : The list to retrieve the element at the nth position.
*    - K : The position where the element to retrieve is.
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
*   Applies a metafunction over all the elements of a list and retrurns the list with the results.
*    - P : The metafunction to be applied to every element.
*    - L : The list whose elements are passed P.
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
*   Performs a fold action from the right to the left of a list.
*    - P : The metafunction used in the fold.
*    - I : The initial value passed to P.
*    - L : The list to be folded.
*/
template<template<typename,typename> typename P, typename I, typename L>
struct reduce_from_end : P<typename reduce_from_end<P,I,cdr_t<L>>::type,car_t<L>>{};

template<template<typename,typename> typename P, typename I, typename T>
struct reduce_from_end<P,I,cons<T,nil>> : P<I,T>{};

template<template<typename,typename> typename P, typename I, typename L>
using reduce_from_end_t = typename reduce_from_end<P,I,L>::type;

//---------------------------------------------------------------------------------

/**
*   Performs a cons creation if B is true_type.
*    - B : A bool_constant that indicates whether to perform the cons or not.
*    - T : The type to be left consed with L.
*    - L : The type to be right consed with T.
*/
template<typename B, typename T, typename L>
struct cons_if : cons<T,L>{};

template<typename T, typename L>
struct cons_if<std::false_type,T,L> : L{};

template<typename B, typename T, typename L>
using cons_if_t = typename cons_if<B,T,L>::type;

//---------------------------------------------------------------------------------

/**
*   Removes the elements of a list that not verifies a condition.
*    - P : P is evaluated once per element. If the result is false_type, the element is removed from the list.
*    - L : The list whose elements could be removed.
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
*   Performs a metafunction using as arguments the types that are passed to apply, including all
*   the elements contained in the list that should be in last place.
*    - P : The metafunction to be performed.
*    - S... : The arguments of P. The last arguments to be passed to P should be at a list being last argument of apply.
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
*   Returns true_type if some element is true_type. In other case, it returns false_type.
*    - TS... : A bunch of types being true_type or false_type.
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
*   Returns a list where each element is a list that contains two elements from the initial lists
*   as long as both elements has the same index.
*    - L : A tlist to be zipped.
*    - S : A tlist to be zipped.
*/
template<typename L, typename S>
struct zip : nil{};

template<typename P, typename Q, typename R, typename S>
struct zip<cons<P,Q>,cons<R,S>> : cons<tlist_t<P,R>,typename zip<Q,S>::type>{};

template<typename L, typename S>
using zip_t = typename zip<L,S>::type;

//---------------------------------------------------------------------------------

/**
*   Reverses the order of the arguments a metafunction receives.
*    - P : The metafunction whose arguments are going to be flipped.
*/
template<template<typename,typename> typename P>
struct flip{
    template<typename T, typename S>
    using type = typename P<S,T>::type;
};


//---------------------------------------------------------------------------------
//-------------------------------- virtual_type -----------------------------------
//---------------------------------------------------------------------------------

/**
*   Wraps a type. It is used to know which type participate in the multiple dispatch.
*    - T : The type that will be wrapped.
*/
template<typename T>
struct virtual_type{
    using type = virtual_type<T>;
};

//---------------------------------------------------------------------------------

/**
*   Checks whether a type is a pointer or a reference to a polymorphic type.
*    - T : The type to check.
*/
template<typename T>
struct is_polymorphic_pr : or_type<std::is_polymorphic_t<std::remove_pointer_t<T>>,std::is_polymorphic_t<std::remove_reference_t<T>>>{};

template<typename T>
using is_polymorphic_pr_t = typename is_polymorphic_pr<T>::type;

template<typename T>
static constexpr bool is_polymorphic_pr_v = is_polymorphic_pr<T>::value;


//---------------------------------------------------------------------------------
//-------------------------- Type related metafunctions ---------------------------
//---------------------------------------------------------------------------------

/**
*   Removes references, pointers and cv qualifiers from a type.
*    - T : The type to be turned into a core form.
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
*   The reference, pointers and cv qualifiers are traslated from a type to another one.
*    - N : The type with reference, pointers and cv qualifiers that will be traslated to S.
*    - S : The type that will receive the reference, pointers and cv qualifiers from T.
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
//------------------------- VBS (Virtual Base Signature) --------------------------
//---------------------------------------------------------------------------------

/**
*   Creates the VBS from the ftype.
*/
template<typename T>
struct ftype_to_sign{};

template<typename R, typename... AS>
struct ftype_to_sign<R(AS...)> : tlist<R,AS...>{};

template<typename T>
using ftype_to_sign_t = typename ftype_to_sign<T>::type;


//---------------------------------------------------------------------------------
//----------------------------- BCL (Base Core List) ------------------------------
//---------------------------------------------------------------------------------

/**
*   Retrieves from the VBS a list with the virtual base types in their core form.
*    - VBS : The VBS type.
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
//----------------------------- BS (Base Signature) -------------------------------
//---------------------------------------------------------------------------------

/**
*   Creates a BS from a VBS.
*    - VBS : The VBS type.
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
//------------------------------------- TID ---------------------------------------
//---------------------------------------------------------------------------------

/**
*   Creates a list whose first element is a base type and the rest of elements are derived types from the DCL.
*    - B : The base type that will be the first element of the list.
*    - DCL : The DCL type.
*/
template<typename B, typename DCL>
struct create_base_of_many : cons<B,remove_if_not_t<std::is_base_of_c<B>::template type,DCL>>{};

template<typename B, typename DCL>
using create_base_of_many_t = typename create_base_of_many<B,DCL>::type;

template<typename DCL>
struct create_base_of_many_c_inv{
    template<typename B>
    using type = create_base_of_many_t<B,DCL>;
};

//---------------------------------------------------------------------------------

/**
*   Create a TID from the BCL and the DCL.
*    - BCL : The BCL type.
*    - DCL : The DCL type.
*/
template<typename BCL, typename DCL>
struct create_type_id : mapcar<create_base_of_many_c_inv<DCL>::template type,BCL>{};

template<typename BCL, typename DCL>
using create_type_id_t = typename create_type_id<BCL,DCL>::type;


//---------------------------------------------------------------------------------
//----------------------------------- Indices -------------------------------------
//---------------------------------------------------------------------------------

/**
*   Inserts an element in every list contained in the LL list.
*    - T : The element that will be inserted in every list contained in the LL list.
*    - LL : A list that contains a bunch of lists where the T element will be inserted in.
*/
template<typename T, typename LL>
using cons_all = mapcar<cons_c<T>::template type,LL>;

template<typename T, typename LL>
using cons_all_t = typename cons_all<T,LL>::type;

template<typename LL>
struct cons_all_c_inv{
    template<typename T>
    using type = cons_all_t<T,LL>;
};

//---------------------------------------------------------------------------------

/**
*   Returns the cartesian product of several lists.
*    - LS... : The lists used for creating the cartesian product.
*/
template<typename L, typename M>
struct cartesian_product_aux : apply<append,mapcar_t<cons_all_c_inv<M>::template type,L>>{};

template<typename... LS>
struct cartesian_product : reduce_from_end<flip<cartesian_product_aux>::template type,tlist_t<nil>,tlist_t<LS...>>{};

template<typename... LS>
using cartesian_product_t = typename cartesian_product<LS...>::type;

//---------------------------------------------------------------------------------

/**
*   Returns a list with the numbers from zero until a given one.
*    - M : Indicates that the maximum number to generate will be M.
*/
template<typename K, typename M>
struct less_numbers_aux : cons<K,typename less_numbers_aux<add1_t<K>,M>::type>{};

template<typename M>
struct less_numbers_aux<M,M> : nil{};

template<typename M>
using less_numbers = less_numbers_aux<zero,M>;

template<typename M>
using less_numbers_t = typename less_numbers<M>::type;

//---------------------------------------------------------------------------------

/**
*   Creates a list containing every possible combinantion of indices in the omm table.
*    - TID : The TID type.
*/
template<typename TID>
struct make_indices : apply<cartesian_product,mapcar_t<less_numbers,mapcar_t<length,TID>>>{};

template<typename TID>
using make_indices_t = typename make_indices<TID>::type;


//---------------------------------------------------------------------------------
//------------------------------------- DCOMB -------------------------------------
//---------------------------------------------------------------------------------

/**
*   Returns a list of derived types associated with the indices in KS. For each number in
*   the list KS, a type is retrieved from TID whose position is that number.
*    - TID : The TID type.
*    - KS : A list containing the indices.
*/
template<typename TID, typename KS>
struct indices_to_types : mapcar<apply_c<nth>::template type,zip_t<TID,KS>>{};

template<typename TID, typename KS>
using indices_to_types_t = typename indices_to_types<TID,KS>::type;

template<typename TID>
struct indices_to_types_c{
    template<typename KS>
    using type = indices_to_types_t<TID,KS>;
};

//---------------------------------------------------------------------------------

/**
*   Returns a list containing all the combinations of derived types that the signatures of the
*   implementations can have.
*    - TID : The TID type.
*    - IND : The list containing all the possible indices of the omm table.
*/
template<typename TID, typename IND>
struct make_derived_combinations : mapcar<indices_to_types_c<TID>::template type,IND>{};

template<typename TID, typename IND>
using make_derived_combinations_t = typename make_derived_combinations<TID,IND>::type;


//---------------------------------------------------------------------------------
//------------------------------------ DSCOMB -------------------------------------
//---------------------------------------------------------------------------------

/**
*   Turns the VBS into a DS, i.e. a signature where the virtual types have been replaced
*   by a derived type from the DL list.
*    - VBS : The VBS type.
*    - DL : A list containing derived types that will replace the virtual types from the VBS.
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
*   Returns a list containing every possible derived signature from the derived types in DCOMB
*   that will replace the virtual base types from the VBS.
*    - VBS : The VBS type.
*    - DCOMB : The DCOMB type.
*/
template<typename VBS, typename DCOMB>
struct vbsign_to_dsign_combinations : mapcar<vbsign_to_dsign_c<VBS>::template type,DCOMB>{};

template<typename VBS, typename DCOMB>
using vbsign_to_dsign_combinations_t = typename vbsign_to_dsign_combinations<VBS,DCOMB>::type;


//---------------------------------------------------------------------------------
//---------------------------------- Omm table ------------------------------------
//---------------------------------------------------------------------------------

/**
*   Returns a function type from a collection of types.
*    - C : The collection with the types that will be used to form the function type.
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
*   Returns a function type from a signature, i.e. a list of types.
*    - L : The list with the types that will be used to form the function type.
*/
template<typename L>
struct signature_to_function_type : collection_to_function_type<tlist_to_collection_t<L>>{};

template<typename L>
using signature_to_function_type_t = typename signature_to_function_type<L>::type;

//---------------------------------------------------------------------------------

/**
*   Taken from https://stackoverflow.com/questions/28309164/checking-for-existence-of-an-overloaded-member-function
*   Checks whether an implementation exists inside the struct with all the implementations.
*    - F: The struct containing all the implementations.
*    - CD: A collection representing the signature of the specific implementation method.
*/
template <typename T, typename... Dargs>
class has_implementation_aux{

    template <typename C, typename = decltype(C::implementation(std::declval<Dargs>()...))>
    static std::true_type test(int);

    template <typename C>
    static std::false_type test(...);

public:

    using type = decltype(test<T>(0));
    static constexpr bool value = decltype(test<T>(0))::value;

};

template<typename F, typename CD>
struct has_implementation{};

template<typename F, typename R, typename... Dargs>
struct has_implementation<F,collection<R,Dargs...>> : has_implementation_aux<F,Dargs...>{};

template<typename F, typename CD>
using has_implementation_t = typename has_implementation<F,CD>::type;

template<typename F, typename CD>
static constexpr bool has_implementation_v = has_implementation<F,CD>::value;

//---------------------------------------------------------------------------------

/**
*   Generates a function pointer that calls a specific implementation method after doing a cast.
*    - F: The struct containing all the implementations.
*    - BS: A tlist containing the signature of the returned function pointer.
*    - DS: A tlist containing the signature of the specific implementation method.
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
struct make_function_cell : make_function_cell_aux<has_implementation_t<F,tlist_to_collection_t<DS>>,
                                                   F,tlist_to_collection_t<BS>,tlist_to_collection_t<DS>>{};

//---------------------------------------------------------------------------------

/**
*   Creates the omm table.
*    - F : The struct containing all the implementations.
*    - BS : The BS type.
*    - DSCOMB : The DSCOMB type.
*/
template<typename F, typename BS, typename DSCOMB>
struct create_omm_table_aux{};

template<typename F, typename BS, typename... DS>
struct create_omm_table_aux<F,BS,collection<DS...>>{
    static constexpr std::add_pointer_t<signature_to_function_type_t<BS>> value[] = {make_function_cell<F,BS,DS>::value...};
};

template<typename F, typename BS, typename DSCOMB>
struct create_omm_table : create_omm_table_aux<F,BS,tlist_to_collection_t<DSCOMB>>{};

template<typename F, typename BS, typename DSCOMB>
static constexpr auto create_omm_table_v = create_omm_table<F,BS,DSCOMB>::value;


//---------------------------------------------------------------------------------
//---------------------------------- Get index ------------------------------------
//---------------------------------------------------------------------------------

/**
*   Returns an index from the type_info of an object.
*    - BM : A list whose first element is a base type and the rest are derived types.
*    - info : The type_info of an object.
*/
template<typename BM>
struct position_derived_runtime_aux{
    static int call(const std::type_info& info){
        return 0;
    }
};

template<typename D, typename S>
struct position_derived_runtime_aux<cons<D,S>>{
    static int call(const std::type_info& info){
        if (info == typeid(D))
            return 0;
        else
            return 1+position_derived_runtime_aux<S>::call(info);

    }
};

template<typename BM>
struct position_derived_runtime{
    static int call(const std::type_info& info){
        return position_derived_runtime_aux<BM>::call(info);
    }
};

//---------------------------------------------------------------------------------

/**
*   Returns the type_info of the most derived type of an object.
*    - A : The type of the object the type_info will be created from.
*    - a : The object to retrieve its type_info.
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
*   Returns the index where the implementation that must be called is.
*    - TID : The TID type.
*    - VBS : The VBS type.
*    - AS... : The types of the arguments that will be passed to the implementation.
*    - as... : The objects that will be passed to the implementation.
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
    static constexpr int current_multiplier = get_index_aux<TS,BS,AS...>::current_multiplier*length_v<T>;
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
//-------------------------- Putting it all together  -----------------------------
//---------------------------------------------------------------------------------

template<typename T>
struct Debug{};

/**
*   Uses all the metafunctions defined above to create the omm table and allows to the
*   user use open multi-methods.
*    - F : The struct where the desired implementations are.
*    - ftype : The function type indicating which are the virtual base types.
*    - DCL : The derived types that participate in the multiple dispatch.
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
    static constexpr auto table = create_omm_table_v<F,BS,DSCOMB>;

    template<typename... AS>
    static auto call(AS&&... as){
        return table[get_index<TID,VBS,AS...>::call(std::forward<AS>(as)...)](std::forward<AS>(as)...);
    }
};


//---------------------------------------------------------------------------------
//------------------------------- User interface  ---------------------------------
//---------------------------------------------------------------------------------

/**
*   Used to indicate the struct that contains all the implementations.
*/
template<typename F>
using WithImplementations = F;

/**
*   Used to indicate the template signature of the implementations.
*/
template<typename F>
using WithSignature = F;

/**
*   Used to indicate the types that participate in the multiple dispatch.
*/
template<typename T>
using Virtual = std::enable_if_t<is_polymorphic_pr_v<T>,virtual_type<T>>;

/**
*   Helper to create a list with the derived types that will participate on the
*   creation of the omm table.
*/
template<typename... DS>
using WithDerivedTypes = tlist_t<DS...>;




#endif // OMM_H_INCLUDED
