#include "cexpr_lib.hpp"

using namespace type_var;
using namespace cexpr_control;
using namespace cexpr_lib_aux;

#define STORE template struct type_var::Store
#define CEXPR_DO template struct




struct test {};
static_assert(cexpr_counter::load<test>() == 0);

static_assert(cexpr_counter::fetch_add<test>() == 0);
static_assert(cexpr_counter::fetch_add<test>() == 1);
static_assert(cexpr_counter::fetch_add<test>() == 2);
static_assert(cexpr_counter::load<test>() == 3);


struct a {};
struct b {};




STORE<a, a>;
STORE<a, int>;
STORE<b, value<a>>;
STORE<a, bool>;
static_assert(replace_std::is_same_v<value<b>, int>);
static_assert(replace_std::is_same_v<value<a>, bool>);


struct tag {};


CEXPR_DO type_var::Store<tag, int>;

CEXPR_DO cexpr_control::if_else_subst<true, setter<tag>, float, double>;

static_assert(replace_std::is_same_v<float, value<tag>>);


template <class name, unsigned N>
struct SmolRecurse : type_var::Store<name, next<value<name>>>, SmolRecurse<name, N-1> {};

template <class name>
struct SmolRecurse<name, 0> {};

struct tag2 {};
STORE<tag2, int>;

template struct SmolRecurse<tag2, 3>;

static_assert(replace_std::is_same_v<value<tag2>, next<next<next<int>>>>);


template <class name>
struct MakeList {
    template <auto v = [](){}>
    struct call : type_var::Store<name, next<value<name>>> {};
};

struct tag3 {};
STORE<tag3, long>;

CEXPR_DO cexpr_control::Recurse<MakeList<tag3>, 2>;
CEXPR_DO cexpr_control::Recurse<MakeList<tag3>, 1>;

static_assert(replace_std::is_same_v<value<tag3>, next<next<next<long>>>>);





template <class name, class stopvar>
struct MakeList4 {
    template <auto v = [](){}>
    struct call : 
        Store<name, next<value<name>>>, 
        if_subst<
            replace_std::is_same_v<
                value<name>, 
                next<next<next<double>>>
            >,
            setter<stopvar>,
            False
        > 
    {};
};

struct tag4{};
struct stopvar4{};

STORE<tag4, double>;
STORE<stopvar4, True>;

CEXPR_DO cexpr_control::DoWhile<MakeList4<tag4, stopvar4>, stopvar4>;

static_assert(replace_std::is_same_v<value<tag4>, next<next<next<next<double>>>>>);
static_assert(replace_std::is_same_v<value<stopvar4>, False>);



namespace Peano {
    struct IsPeanoInteger {};
    struct Zero : IsPeanoInteger {};

    template <class T> struct Succ : IsPeanoInteger {
        // static_assert(std::is_same_v<T, None>);
    };
    template <class V> struct Succ<Succ<V>> : IsPeanoInteger {
        using Prev = Succ<V>;
    };
    template <> struct Succ<Zero> : IsPeanoInteger {
        using Prev = Zero;
    };

    using One   = Succ<Zero>;
    using Two   = Succ<One>;
    using Three = Succ<Two>;
    using Four  = Succ<Three>;
    using Five  = Succ<Four>;

    template <class U, class V>
    struct Add {
        using result = typename Add<typename U::Prev, Succ<V>>::result;
    };

    template <class V>
    struct Add<Zero,V> {
        using result = V;
    };

    template <class U, class V>
    struct Mul {
        using result = typename Add<typename Mul<typename U::Prev, V>::result, V>::result;
    };

    template <class V>
    struct Mul<Zero, V> {
        using result = Zero;
    };

    
    template <class T>
    constexpr unsigned cast = cast<typename T::Prev> + 1;

    template<>
    constexpr unsigned cast<Zero> = 0;

    template <class U>
    struct IsDivBy2 {
        using U1 = typename U::Prev;
        using U2 = typename U1::Prev;
        using result = typename IsDivBy2<U2>::result;
        using Result = Succ<typename IsDivBy2<U2>::Result>;
    };
    template <>
    struct IsDivBy2<Zero> { 
        using result = True; 
        using Result = Zero;
    };


    template <>
    struct IsDivBy2<One> { 
        using result = False;
        using Result = None; 
    };

    static_assert(cast<One>   == 1);
    static_assert(cast<Two>   == 2);
    static_assert(cast<Three> == 3);
    static_assert(cast<Four>  == 4);
    static_assert(cast<Five>  == 5);

    static_assert(cast<Add<Two, Three>::result> == 5);
    static_assert(cast<Mul<Two, Two>::result> == 4);
    static_assert(cast<Mul<Two, Three>::result> == 6);
    static_assert(replace_std::is_same_v<
                    IsDivBy2<
                        Mul<Two, Three>::result
                    >::result,
                    True
                >);
    static_assert(replace_std::is_same_v<
                    IsDivBy2<
                        Mul<Two, Three>::result
                    >::Result,
                    Three
                >);

    static_assert(replace_std::is_same_v<
                    IsDivBy2<
                        Mul<Three, Three>::result
                    >::result,
                    False
                >);
};


using namespace Peano;
// 7 -> 22 -> 11 -> 34 -> 17 -> 52 -> 26 -> 13 -> 40 -> 20 -> 10 -> 5 -> 16 -> 8 -> 4 -> 2 -> 1

template <class counter, class var, class stopvar>
struct PeanoThingie {
    template <auto v = [](){}>
    struct call : 
        Store<counter, Succ<value<counter>>>,
        if_subst<
            replace_std::is_same_v<
                value<var>, 
                One
            >,
            setter<stopvar>,
            False
        >,
        if_else_subst<
            replace_std::is_same_v<typename IsDivBy2<value<var>>::result, True>,
            setter<var>,
            typename IsDivBy2<value<var>>::Result,
            Succ<typename Mul<Three, value<var>>::result>
        >
    {};
};


struct counter5 {};
struct stopvar5 {};
struct var5 {};
STORE<counter5, Zero>;
STORE<stopvar5, True>;
STORE<var5, Succ<Succ<Five>>>;

CEXPR_DO cexpr_control::DoWhile<PeanoThingie<counter5,var5, stopvar5>, stopvar5>;
static_assert(cast<value<var5>> == 2);
static_assert(cast<value<counter5>> == 18);


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