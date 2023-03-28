#pragma once


/// GENERAL WARNINGS :
/// std=c++20 REQUIRED
/// expect frequent random crashes
/// Do not forget Wno-return-type & Wno-non-template-friend


#define USEFUL_CEXPR_AUXS
#define CEXPR_IF
#define CEXPR_RECURSION



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
    template <class T, class U> 
    constexpr bool is_same_v = is_same<T, U>::value;

    struct monost {};

    template <class ... Args>
    struct Argpass {
        template <class ... Ts>
        static auto Merge(Argpass<Ts...>) {
            return Argpass<Ts ..., Args ...>{};
        }
    };
    template <>
    struct Argpass<monost> {
        template <class ... Ts> 
        static auto Merge(Argpass<Ts ... >) {
            return Argpass<Ts ...>{};
        }
    };
    
    template <class T, class U>
    const T& cast(const U& ref) { return (const T&)ref; }

    template<typename ...Args>
    inline void pass(Args&&...) {}


    template<typename... Args>
    struct GetFirstArg {
        template<typename T, typename ... Ts>
        struct Chipper {
            using type = T;
        };

        using T = typename Chipper<Args...>::type;
    };

    template <const char v[]>
    struct storage {
        static constexpr auto value = v;
    };
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
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr int fadd_reader(int) {
            return fadd_reader<T, N + 1>(int{});
        }

        template <class, int N = 0>
        constexpr int load_reader(float) {
            return N;
        }

        template<class T, int N = 0,
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr int load_reader(int) {
            return load_reader<T, N + 1>(int{});
        }

        template <class T>
        struct name_guard {};
    };

    template<class T, int R = detail::fadd_reader<detail::name_guard<T>, 0>(int{})>
    constexpr int fetch_add() {
        return R;
    }

    template <class T, int R = detail::load_reader<detail::name_guard<T>, 0>(int{})>
    constexpr int load() {
        return R;
    }
};



///
/// This is an implementation of a compile-time list of variables.
///
///
/// Each list has a **Name**: an user-given type T
/// Denote its values as $T
/// The counter is zero-initialized
///
/// Usage:
///
/// Store<T, val>  === $T.append(val)
/// value<T>       === $T[-1]
/// value_at<T, M> === $T[M]
/// len<T>         === len($T)
/// pack_of<T>     === return $T as parameter pack of Argpass< Vals ... >;
///
/// Can be used as a variable
///

namespace type_var {
    namespace detail {
        template <typename T>
        struct ConjureInstanceOf {T& operator*();};

        template <typename T>
        struct Wrap {
            using Res = T;
        };

        template <class T, class U>
        struct MonadicWrap {
            using Res = U;
        };

        template <class T> 
        struct MonadicWrap<T, replace_std::monost>{
            using Res = T;
        };
    

        template<class, int>                                         
        struct Flag {
            friend constexpr auto flag(Flag);
        };

        template<class T, int N, class Val>
        struct Writer {
            friend constexpr auto flag(Flag<T,N>) { 
                return *ConjureInstanceOf<Val>{}; 
            }
            static constexpr int value = N;
        };

        template<class T, class Val, int N = 0>
        constexpr int fadd_reader(float) {
            return Writer<T, N, Val>::value;
        }

        template<class T, class Val, int N = 0,
                class = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr int fadd_reader(int) {
            return fadd_reader<T, Val, N + 1>(int{});
        }

        template <class, int N = 0>
        constexpr auto load_reader(float) {
            return Wrap<replace_std::monost>{};
        }

        template<class T, int N = 0,
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr auto load_reader(int) {
            return MonadicWrap<U, typename decltype(load_reader<T, N + 1>(int{}))::Res>{};
        }

        template <class, int N = 0>
        constexpr auto at_reader(float) {
            return Wrap<replace_std::monost>{};
        }

        template<class T, int M, int N = 0,
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr auto at_reader(int) {
            if constexpr (N == M) {
                return Wrap<U>{};
            } else {
                return at_reader<T, M, N+1>(int{});
            }
        }


        template <class, int N = 0>
        constexpr auto len_reader(float) {
            return N;
        }

        template<class T, int N = 0,
            class U = decltype(flag(Flag<T, N>{})),
            auto = [](){}>
        constexpr int len_reader(int) {
            return len_reader<T, N + 1>(int{});
        }

        template <class, int N = 0>
        constexpr auto pack_reader(float) {
            return replace_std::Argpass<>{};
        }

        template<class T, int N = 0,
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr auto pack_reader(int) {
            return decltype(pack_reader<T, N + 1>(int{}))::Merge(replace_std::Argpass<U>{});
        }

        template <class T>
        struct name_guard {};

    };

    template<class T, class Val, auto v = [](){}, int R = detail::fadd_reader<detail::name_guard<T>, Val, 0>(int{})>
    struct Store { template <class U> struct ComputeIdx { static constexpr int idx = R - 1;}; };

    template <class T, auto v = [](){}, class R = typename decltype(detail::load_reader<detail::name_guard<T>, 0>(int{}))::Res>
    using value = R;

    template <class T, auto v = [](){}, int N = detail::len_reader<detail::name_guard<T>, 0>(int{})>
    constexpr int len = N;

    template <class T, unsigned M, auto v = [](){}, class U = typename decltype(detail::at_reader<detail::name_guard<T>, M, 0>(int{}))::Res>
    using value_at = U;

    template <class T, auto v = [](){}, class U = decltype(detail::pack_reader<detail::name_guard<T>, 0>(int{}))>
    using pack_of = U;
};

/// Useful types and macros :

#ifdef USEFUL_CEXPR_AUXS

namespace cexpr_lib_aux {


// #define STORE template struct type_var::Store
// #define CEXPR_DO template struct

    template <unsigned val_> struct Int { constexpr static unsigned val = val_; };
    template <class T> struct next { using prev = T;};

    struct Bool {};
    struct True : Bool {};
    struct False : Bool {};

    struct None {};

};
#endif

///
/// Control Patterns
///

#ifdef CEXPR_IF

namespace cexpr_control {

/// Essentially, a constexpr if-else
///
/// if `cond`, 
/// forms func::call<F>, instantiating its side-effects
/// else       
/// forms func::call<G>, instantiating its side-effects
///
/// To use, implement your side-effects in func::call, leaving func a wrapper
///
template <bool cond, class func, class arg_if_true, class arg_if_false, auto v = [](){}> 
struct if_else_subst;

template <bool, class func, class arg_if_true, class arg_if_false, auto v> 
struct if_else_subst : 
    func:: template call<arg_if_false> 
{};

template <class func, class arg_if_true, class arg_if_false, auto v>
struct if_else_subst<true, func, arg_if_true, arg_if_false, v> : 
    func:: template call<arg_if_true>
{};


/// Essentially, a constexpr if
///
/// if `cond`, 
/// forms func::call<F>, instantiating its side-effects
///
/// To use, implement your side-effects in func::call, leaving func a wrapper
///

template <bool cond, class func, class arg, auto v = [](){}>
struct if_subst;

template <bool cond, class func, class arg, auto v>
struct if_subst {};

template <class func, class arg, auto v>
struct if_subst<true, func, arg, v> : 
    func:: template call<arg> 
{};

///
/// Curried Store<name, T>
/// Use in if_else_subst
///
template <class name>
struct setter {
    template <class T, auto = [](){}>
    struct call : type_var::Store<name, T> {};
};


#ifdef CEXPR_RECURSION


///
/// Primitive recursion :
/// for (unsigned N; N--; N > 0) {
///     func::call
/// }
///
template <class func, unsigned N, auto v = [](){}>

struct Recurse :
    func:: template call<[](){}>,  
    Recurse<func, N-1, v> 
{};

template <class func, auto v> 

struct Recurse<func, 0, v> 
{};

///
/// True recursion :
/// DoWhile 
///     ($stopcond == True) 
/// do 
///     func::call
// ///
template <class func, class stopcond, unsigned N = 0, bool sfinae_cond = true, auto v = [](){}>
struct DoWhile;

template <class func, class stopcond, unsigned N, bool sfinae_cond, auto v>
struct DoWhile :
    func:: template call<[](){}>,  
    DoWhile<
        func, 
        stopcond, 
        N+1, 
        replace_std::is_same_v<type_var::value<stopcond>, cexpr_lib_aux::True>, 
        v
    > 
{};

template <class func, class stopcond, unsigned N, auto v>
struct DoWhile<func, stopcond, N, false, v> {};

#endif
#endif
};