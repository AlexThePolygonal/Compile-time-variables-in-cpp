#pragma once


/// GENERAL WARNINGS :
/// compile with clang, gcc goes insane.
/// std=c++20 REQUIRED
/// tested with clang++-15
/// expect frequent random frontend crashes


///
/// Helper: using std may result in excessive crashing
///
namespace replace_std {
    template <class T, class U>
    struct is_same {
        constexpr static bool value = false;
    };
    template <class T>
    struct is_same<T, T> {
        constexpr static bool value = true;
    };
    template <class T, class U> constexpr bool is_same_v = is_same<T, U>::value;
};





///
/// This is an implementation of a compile-time counter.
///
/// Each counter has a **Name**: an user-given type T
/// The value of a counter is an integer of type size_t
/// Denote its value as $T
/// The counter is zero-initialized
///
/// Usage:
///
/// fetch_add<T>() === ($T)++;
/// load<T>()      === ($T);
///
///
namespace cexpr_counter {
    namespace detail {
        template<class, int>                                         
        struct Flag {
            friend constexpr auto flag(Flag);
        };

        template<class T, int N>
        struct Writer {
            friend constexpr auto flag(Flag<T,N>) { 
                return true; 
            }
            static constexpr int value = N;
        };

        template<class T, int N = 0>
        constexpr int fadd_reader(float) {
            return Writer<T, N>::value;
        }

        template<class T, int N = 0,
                class = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr int fadd_reader(int) {
            return fadd_reader<T, N + 1>(int{});
        }

        template <class, int N = 0>
        constexpr int load_reader(float) {
            return N;
        }

        template<class T, int N = 0,
                bool = flag(Flag<T, N>{}),
                auto = [](){}>
        constexpr int load_reader(int) {
            return load_reader<T, N + 1>(int{});
        }
    };

    template<class T, int R = detail::fadd_reader<T, 0>(int{})>
    constexpr int fetch_add() {
        return R;
    }

    template <class T, int R = detail::load_reader<T, 0>(int{})>
    constexpr int load() {
        return R;
    }
};

///
/// This is an implementation of a compile-time class variable.
///
/// Each variable has a **Name**: an user-given type T
/// The value of a variable may be an arbitary type except void
/// Denote its value as $T
/// The initial value of a variable is undefined
///
/// Usage:
///
/// value<T>     === $T;
/// Store<T, U>  === $T = U;
///
///

namespace type_var {
    // details
    template <class Name, int N> 
    struct flag {
        friend auto injected(flag<Name, N>);
    };

    // details
    template <typename T>
    struct ConjureInstanceOf {T& operator*();};

    template <class Name, class Value, auto = [](){}, int N = cexpr_counter::fetch_add<Name>()>
    struct Store {
        friend auto injected(flag<Name, N>) { return *ConjureInstanceOf<Value>{}; }
    };

    template <class Name, auto=[](){}>
    using value = decltype(injected(flag<Name, cexpr_counter::load<Name>() - 1>()));

};

///
/// This is an implementation of a compile-time class append-only list.
/// The list consists of class variables and is readonly
///
/// Usage:
///
/// value<T, i>   === T[i]
/// Append<T, U>  === T[len++] = U
///
///

namespace type_list {
    // details
    template <class Name, int N> 
    struct flag {
        friend auto injected(flag<Name, N>);
    };

    // details
    template <typename T>
    struct ConjureInstanceOf {T& operator*();};

    template <class Name, class Value, auto = [](){}, int N = cexpr_counter::fetch_add<Name>()>
    struct Append {
        friend auto injected(flag<Name, N>) { return *ConjureInstanceOf<Value>{}; }
    };

    template <class Name, auto=[](){}>
    using last_value = decltype(injected(flag<Name, cexpr_counter::load<Name>() - 1>()));

    template <class Name,unsigned N, auto=[](){}>
    using value = decltype(injected(flag<Name, N>()));

    template <class Name, auto=[](){}>
    constexpr unsigned len = cexpr_counter::load<Name>() - 1;
};

//


/// Useful types and macros :

#define STORE template struct type_var::Store
#define CEXPR_DO template struct

template <unsigned val_> struct Int { constexpr static unsigned val = val_; };
template <class T> struct next {};

struct Bool {};
struct True : Bool {};
struct False : Bool {};


///
/// Control Patterns
///


/// Essentially, a constexpr if
///
/// if `cond`, 
/// forms func_wrap::call<F>, instantiating its side-effects
/// else       
/// forms func_wrap::call<G>, instantiating its side-effects
///
/// To use, implement your side-effects in func_wrap::call, leaving func_wrap a wrapper
///
template <bool cond, class func_wrap, class F, class G, auto v = [](){}> 

struct cond_subst;

template <bool, class func_wrap, class F, class G, auto v> 

struct cond_subst : 
func_wrap:: template call<G> 
{};

template <class func_wrap, class F, class G, auto v>

struct cond_subst<true, func_wrap, F, G, v> : 
func_wrap:: template call<F>
{};

///
/// Curried Store<name, T>
/// Use in cond_subst
///
template <class name>
struct setter {

    template <class T, auto = [](){}>

    struct call : type_var::Store<name, T> {};
};


///
/// Primitive recursion :
/// for (unsigned N; N--; N > 0) {
///     func_wrap::call
/// }
///
template <class func_wrap, unsigned N, auto v = [](){}>

struct Recurse :
func_wrap:: template call< [](){} >,  Recurse<func_wrap, N-1, v> 
{};

template <class func_wrap, auto v> 

struct Recurse<func_wrap, 0, v> 
{};

///
/// True recursion :
/// While 
///     ($stopcond == True) 
/// do 
///     func_wrap::call
///
template <class func_wrap, class stopcond, unsigned N = 0, bool sfinae_cond = true, auto v = [](){}>

struct While;

template <class func_wrap, class stopcond, unsigned N, bool sfinae_cond, auto v>

struct While :
func_wrap:: template call<[](){}>,  
While<func_wrap, stopcond, N+1, replace_std::is_same_v<type_var::value<stopcond>, True>, v> 
{};

template <class func_wrap, class stopcond, unsigned N, auto v>

struct While<func_wrap, stopcond, N, false, v> 
{};

