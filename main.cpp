#include "cexpr_lib.hpp"

/*
*>>===========================================<<*
*>>=      THIS IS THE AD-HOC TEST FILE       =<<*
*>>= TESTING IS DONE IN THE GLOBAL NAMESPACE =<<*
*>>=    RESULTS FOR GCC AND CLANG WILL VARY  =<<*
*>>=       LACK OF STDLIB IS INTENTIONAL     =<<*
*>>===========================================<<*
*/

/// Preliminaries
/// here all auxiliary things go

using namespace type_var;
using namespace cexpr_control;
using namespace cexpr_lib_aux;
using namespace replace_std;

#define VAR(name) struct name {}
#define STORE template struct type_var::Store
#define CEXPR_DO template struct cexpr_control::

// Another wrapper for types
template <class T> struct next { using prev = T; };


///
/// Test basics
///

static_assert(!is_same_v< uniq<>, uniq<> >, "the type of each [](){} is not unique. Everything is broken");

///
/// Test counters
///


VAR(test);
static_assert(cexpr_counter::load<test>() == 0, "Initial value is malformed");

static_assert(cexpr_counter::fetch_add<test>() == 0);
static_assert(cexpr_counter::fetch_add<test>() == 1);
static_assert(cexpr_counter::fetch_add<test>() == 2);
static_assert(cexpr_counter::load<test>() == 3);
static_assert(cexpr_counter::fetch_add<test>() == 3);    
static_assert(cexpr_counter::load<test>() == 4); 

// check that there are no warnings here
// otherwise something is not right ...



///
/// Test basic variable functionality
///

// Block 1
    VAR(a);
    VAR(b);
    STORE<a, a>;
    STORE<a, int>;
    STORE<b, value<a>>;
    STORE<a, bool>;

    static_assert(is_same_v<value<b>, int>);
    static_assert(is_same_v<value<a>, bool>);

// Block 2
    VAR(c);
    STORE<c, c>;

    STORE<c, next<value<c>>>;
    STORE<c, next<value<c>>>;
    STORE<c, next<value<c>>>;

    static_assert(is_same_v<value<c>, next<next<next<c>>>>, "Multiple Sets to value do not work. Maybe 'value' is being cached?");

// Block 3
    VAR(d);
    static_assert(is_same_v<value<d>, None>, "Default value is wrong");

// Block 4
    VAR(e);
    static_assert(len<e> == 0);
    STORE<e, int>;
    STORE<e, int>;
    STORE<e, int>;
    static_assert(len<e> == 3, "Pack does not contain all the operations on 'e'. Maybe Store is idempotent?");

// Block 5
    VAR(f);
    static_assert(len<f> == 0);
    static_assert(is_same_v<pack_of<f>, Argpass<>>, "Default pack of values is wrong");
    STORE<f, int>;
    STORE<f, long>;
    STORE<f, int>;
    STORE<f, int>;
    static_assert(len<f> == 4);
    static_assert(is_same_v<pack_of<f>, Argpass<int, long, int, int>>, "Pack does not contain the history of the variable. Maybe Store is being cached?");

///
/// Test conditional operations
///

// Block 1
    VAR(g);
    STORE<g, long>;
    CEXPR_DO IfSubst<is_same_v<value<g>, int>, Setter<g>::Set, float>;

    static_assert(is_same_v<value<g>, long>, "IfSubst instantiates 'Setter::Set'");

    CEXPR_DO IfSubst<is_same_v<value<g>, long>, Setter<g>::Set, float>;

    static_assert(is_same_v<value<g>, float>, "IfSubst does not instantiate 'Setter::Set'");


// Block 2
    VAR(h);
    STORE<h, long>;
    CEXPR_DO IfSubst<
        is_same_v<value<h>, long>, 
        Setter<h>::Set, float
    >;
    STORE<h, long>;
    CEXPR_DO IfSubst<
        is_same_v<value<h>, long>, 
        Setter<h>::Set, float
    >;
    STORE<h, int>;
    CEXPR_DO IfSubst<
        is_same_v<value<h>, int>, 
        Setter<h>::Set, float
    >;

    static_assert(len<h> == 6);
    static_assert(is_same_v<value<h>, float>, "The 'Setter<name>::Set' is being cached. Maybe you forgot to use '[](){}' or 'replace_std::uniq<>'?");

// Block 3
    VAR(i);
    STORE<i, int>;
    static_assert(is_same_v<value<i>, int>);

    CEXPR_DO IfElseSubst<
        is_same_v<value<i>, int>, 
        Setter<i>::Set, next<value<i>>, value<i>
    >;
    static_assert(is_same_v<value<i>, next<int>>);

    CEXPR_DO IfElseSubst<
        is_same_v<value<i>, int>, 
        Setter<i>::Set, value<i>, next<value<i>>
    >;
    static_assert(is_same_v<value<i>, next<next<int>>>);

// Block 4
    VAR(j);
    STORE<j, long>;
    CEXPR_DO IfElseSubst<
        is_same_v<value<j>, long>, 
        Setter<j>::Set, float, bool
    >;
    STORE<j, long>;
    CEXPR_DO IfElseSubst<
        is_same_v<value<j>, long>, 
        Setter<j>::Set, float, bool
    >;
    STORE<j, int>;
    CEXPR_DO IfElseSubst<
        is_same_v<value<j>, int>, 
        Setter<j>::Set, float, bool
    >;
    static_assert(len<j> == 6);
    static_assert(is_same_v<value<j>, float>, "The 'Setter<name>::Set' is being cached. Maybe you forgot to use '[](){}' or 'replace_std::uniq<>'?");

// Block 5
    VAR(k);
    STORE<k, int>;
    template <class name>
    struct ListMaker {
        template <class>
        struct Make : type_var::Store<name, next<value<name>>> {};
    };
    CEXPR_DO Recurse<ListMaker<k>::Make, 2>;
    static_assert(is_same_v<value<k>, next<next<int>>>);
    CEXPR_DO Recurse<ListMaker<k>::Make, 2>;
    static_assert(is_same_v<value<k>, next<next<next<next<int>>>>>);

// Block 6
    VAR(l);
    STORE<l, int>;

    template <class name>
    struct NestedListMaker {
        template <class>
        struct temp {};

        template <class T>
        struct Make : 
            Store<temp<T>,int>,
            Recurse<
                ListMaker<temp<T>>::template Make, 
                len<name>
            >,
            Store<name, value<temp<T>>>
        {};
    };

    static_assert(len<l> == 1);
    CEXPR_DO Recurse<NestedListMaker<l>::Make, 2>;
    static_assert(is_same_v<value<l>, next<next<int>>>, "GCC, sorry");


// template <class name>
// struct MakeList {
//     template <auto v = [](){}>
//     struct Set : type_var::Store<name, next<value<name>>> {};
// };

// struct tag3 {};
// STORE<tag3, long>;

// CEXPR_DO Recurse<MakeList<tag3>, 2>;
// CEXPR_DO Recurse<MakeList<tag3>, 1>;

// static_assert(replace_std::is_same_v<value<tag3>, next<next<next<long>>>>);





// template <class name, class stopvar>
// struct MakeList4 {
//     template <auto v = [](){}>
//     struct Set : 
//         Store<name, next<value<name>>>, 
//         IfSubst<
//             replace_std::is_same_v<
//                 value<name>, 
//                 next<next<next<double>>>
//             >,
//             Setter<stopvar>,
//             False
//         > 
//     {};
// };

// VAR(tag4);
// VAR(stopvar4);

// STORE<tag4, double>;
// STORE<stopvar4, True>;

// CEXPR_DO DoWhile<MakeList4<tag4, stopvar4>, stopvar4>;

// static_assert(replace_std::is_same_v<value<tag4>, next<next<next<double>>>>);
// static_assert(replace_std::is_same_v<value<stopvar4>, False>);



// namespace Peano {
//     struct IsPeanoInteger {};
//     struct Zero : IsPeanoInteger {};

//     template <class T> struct Succ : IsPeanoInteger {
//         // static_assert(std::is_same_v<T, None>);
//     };
//     template <class V> struct Succ<Succ<V>> : IsPeanoInteger {
//         using Prev = Succ<V>;
//     };
//     template <> struct Succ<Zero> : IsPeanoInteger {
//         using Prev = Zero;
//     };

//     using One   = Succ<Zero>;
//     using Two   = Succ<One>;
//     using Three = Succ<Two>;
//     using Four  = Succ<Three>;
//     using Five  = Succ<Four>;

//     template <class U, class V>
//     struct Add {
//         using result = typename Add<typename U::Prev, Succ<V>>::result;
//     };

//     template <class V>
//     struct Add<Zero,V> {
//         using result = V;
//     };

//     template <class U, class V>
//     struct Mul {
//         using result = typename Add<typename Mul<typename U::Prev, V>::result, V>::result;
//     };

//     template <class V>
//     struct Mul<Zero, V> {
//         using result = Zero;
//     };

    
//     template <class T>
//     constexpr unsigned cast = cast<typename T::Prev> + 1;

//     template<>
//     constexpr unsigned cast<Zero> = 0;

//     template <class U>
//     struct IsDivBy2 {
//         using U1 = typename U::Prev;
//         using U2 = typename U1::Prev;
//         using result = typename IsDivBy2<U2>::result;
//         using Result = Succ<typename IsDivBy2<U2>::Result>;
//     };
//     template <>
//     struct IsDivBy2<Zero> { 
//         using result = True; 
//         using Result = Zero;
//     };


//     template <>
//     struct IsDivBy2<One> { 
//         using result = False;
//         using Result = None; 
//     };

//     static_assert(cast<One>   == 1);
//     static_assert(cast<Two>   == 2);
//     static_assert(cast<Three> == 3);
//     static_assert(cast<Four>  == 4);
//     static_assert(cast<Five>  == 5);

//     static_assert(cast<Add<Two, Three>::result> == 5);
//     static_assert(cast<Mul<Two, Two>::result> == 4);
//     static_assert(cast<Mul<Two, Three>::result> == 6);
//     static_assert(replace_std::is_same_v<
//                     IsDivBy2<
//                         Mul<Two, Three>::result
//                     >::result,
//                     True
//                 >);
//     static_assert(replace_std::is_same_v<
//                     IsDivBy2<
//                         Mul<Two, Three>::result
//                     >::Result,
//                     Three
//                 >);

//     static_assert(replace_std::is_same_v<
//                     IsDivBy2<
//                         Mul<Three, Three>::result
//                     >::result,
//                     False
//                 >);
// };


// using namespace Peano;
// // 7 -> 22 -> 11 -> 34 -> 17 -> 52 -> 26 -> 13 -> 40 -> 20 -> 10 -> 5 -> 16 -> 8 -> 4 -> 2 -> 1

// template <class counter, class var, class stopvar>
// struct PeanoThingie {
//     template <auto v = [](){}>
//     struct Set :
//         if_else_subst<
//             replace_std::is_same_v<typename IsDivBy2<value<var>>::result, True>,
//             Setter<var>,
//             typename IsDivBy2<value<var>>::Result,
//             Succ<typename Mul<Three, value<var>>::result>
//         >,
//         Store<counter, Succ<value<counter>>>,
//         IfSubst<
//             replace_std::is_same_v<
//                 value<var>, 
//                 One
//             >,
//             Setter<stopvar>,
//             False
//         >
//     {};
// };


// struct counter5 {};
// struct stopvar5 {};
// struct var5 {};
// STORE<counter5, Zero>;
// STORE<stopvar5, True>;
// STORE<var5, Two>;

// CEXPR_DO DoWhile<PeanoThingie<counter5,var5, stopvar5>, stopvar5>;
// // Clang does right when not ill
// static_assert(Peano::cast<value<var5>> == 1);
// static_assert(Peano::cast<value<counter5>> == 1);

// template <class Arg, class ... Args>
// void print(replace_std::Argpass<Arg, Args...>) {
//     std::cout << typeid(Arg).name() <<'\n';
//     if constexpr (sizeof...(Args)) {
//         print(replace_std::Argpass<Args...>{});
//     }
// }


int main() {
    // print(pack_of<counter5>{});
}