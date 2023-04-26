#pragma once


/// GENERAL WARNINGS :
/// std=c++20 REQUIRED
static_assert(__cplusplus >= 202002L);
/// expect frequent random crashes
/// Do not forget Wno-return-type & Wno-non-template-friend


#define USEFUL_CEXPR_AUXS
#define CEXPR_IF
#define CEXPR_RECURSION



///
/// Helper: using std may result in excessive crashing
///
namespace replace_std {
    // Replacement for std::is_same_v
    template <typename, typename>
    inline constexpr bool is_same_v = false;
    template <typename T>
    inline constexpr bool is_same_v<T, T> = true;

    // Pythonesque None
    // The result for ill-formed operations
    struct None {};

    // Pass parameter packs as results of type-levvel functions
    template <class ... Args>
    struct Argpass {
        template <class ... Ts>
        static auto Merge(Argpass<Ts...>) {
            return Argpass<Ts ..., Args ...>{};
        }
    };
    template <>
    struct Argpass<None> {
        template <class ... Ts> 
        static auto Merge(Argpass<Ts ... >) {
            return Argpass<Ts ...>{};
        }
    };

    // Get first element of parameter pack
    template<typename... Args>
    struct GetFirstArg {
        template<typename T, typename ... Ts>
        struct Chipper {
            using type = T;
        };

        using T = typename Chipper<Args...>::type;
    };

    // Each instantiation of this type is unique
    template <auto v = [](){}>
    using uniq = decltype(v);
};



#ifdef USEFUL_CEXPR_AUXS

/// Useful types and macros :
/// 'Int' is an integer wrapper
namespace cexpr_lib_aux {


// #define STORE template struct type_var::Store
// #define CEXPR_DO template struct
    
    // Type-level wrapper for constexpr int
    template <int val_> struct Int { constexpr static int val = val_; };
    
    // Condition control
    struct Bool {};
    struct True : Bool {};
    struct False : Bool {};
};
#endif




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
    // Fetch the value and increment the variable. 
    // Equvalent to ($T)++
    template<class T, class v = replace_std::uniq<[](){}>, int R = detail::fadd_reader<detail::name_guard<T>, 0>(int{})>
    constexpr int fetch_add() {
        return R;
    }

    // Fetch the value
    // Equvalent to $T
    template <class T, class v = replace_std::uniq<[](){}>, int R = detail::load_reader<detail::name_guard<T>, 0>(int{})>
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
        struct MonadicWrap<T, replace_std::None>{
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
            return Wrap<replace_std::None>{};
        }

        template<class T, int N = 0,
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr auto load_reader(int) {
            return MonadicWrap<U, typename decltype(load_reader<T, N + 1>(int{}))::Res>{};
        }

        template <class, int N = 0>
        constexpr auto at_reader(float) {
            return Wrap<replace_std::None>{};
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

    // Store Val in T
    // Equivalent to $T = Val
    template<class T, class Val, class v = replace_std::uniq<[](){}>, int R = detail::fadd_reader<detail::name_guard<T>, Val, 0>(int{})>
    struct Store {
        // Index of Val in the variable T, impl #1
        using CurrentIdx = cexpr_lib_aux::Int<R>;

        // Index of Val in the variable T, impl #2
        template <class U> struct ComputeIdx { static constexpr int idx = R - 1;}; 
    };

    // Get the value stored in T
    // Equvalent to $T
    template <class T, class v = replace_std::uniq<[](){}>, class R = typename decltype(detail::load_reader<detail::name_guard<T>, 0>(int{}))::Res>
    using value = R;

    // Get the number of times a variable was assigned to T
    template <class T, class v = replace_std::uniq<[](){}>, int N = detail::len_reader<detail::name_guard<T>, 0>(int{})>
    constexpr int len = N;

    // Get the value assigned to T in Store #M
    template <class T, unsigned M, class v = replace_std::uniq<[](){}>, class U = typename decltype(detail::at_reader<detail::name_guard<T>, M, 0>(int{}))::Res>
    using value_at = U;

    // Get all the values ever stored in T in chronological order
    template <class T, class v = replace_std::uniq<[](){}>, class U = decltype(detail::pack_reader<detail::name_guard<T>, 0>(int{}))>
    using pack_of = U;
};

///
/// Control Patterns
///

#ifdef CEXPR_IF

/// Stateful branching and Recursion
/// 
/// Results and temporary values may be stored in external compile-time type variables
///
/// IfSubst<cond, func, arg>            ===  execute function with given argument if true
/// IfElseSubst<cond, func, arg1, arg2> ===  execute function with first argument if true and with second if false
/// Recurse<func, N>                    ===  execute a function N times
/// DoWhile<func, stopcond>             ===  execute func while the type variable stopcond does not contain True
///
namespace cexpr_control {

/// Essentially, a constexpr if-else
///
/// if 'cond', 
/// forms 'func<F>', instantiating its side-effects
/// else       
/// forms 'func<G>', instantiating its side-effects
///
/// Second argument for func should be ignored
///
template <bool cond, template <class, class> class func, class arg_if_true, class arg_if_false, class v = replace_std::uniq<[](){}>> 
struct IfElseSubst;

template <bool, template <class, class> class func, class arg_if_true, class arg_if_false, class v> 
struct IfElseSubst : 
    func<arg_if_false, replace_std::uniq<[](){}>> 
{};

template <template <class, class> class func, class arg_if_true, class arg_if_false, class v>
struct IfElseSubst<true, func, arg_if_true, arg_if_false, v> : 
    func<arg_if_true, replace_std::uniq<[](){}>>
{};


/// Essentially, a constexpr if
///
/// if 'cond', 
/// forms 'func<F>', instantiating its side-effects
///
/// Second argument for func should be ignored
///

template <bool cond, template <class, class> class func, class arg, class v = replace_std::uniq<[](){}>>
struct IfSubst;

template <bool cond, template <class, class> class func, class arg, class v>
struct IfSubst {};

template <template <class, class> class func, class arg, class v>
struct IfSubst<true, func, arg, v> : 
    func<arg, replace_std::uniq<[](){}>>
{};

///
/// Curried Store<name, T>
/// Use in IfElseSubst
///
template <class name>
struct Setter {
    template <class T, class>
    struct Set : type_var::Store<name, T> {};
};



#ifdef CEXPR_RECURSION


///
/// Primitive recursion :
/// for (unsigned N; N--; N > 0) {
///     func<>;
/// }
///
template <template<class> class func, unsigned N, class v = replace_std::uniq<[](){}>>
struct Recurse :
    func<replace_std::uniq<[](){}>>,  
    Recurse<func, N-1, replace_std::uniq<[](){}>> 
{};

template <template<class> class func, class v> 
struct Recurse<func, 0, v> 
{};



///
/// True recursion :
///  
/// do 
///     func<>;
/// while($stopcond == True)
///
template <template<class> class func, class stopcond, unsigned N = 0, bool sfinae_cond = true, class v = replace_std::uniq<[](){}>>
struct DoWhile;

template <template<class> class func, class stopcond, unsigned N, bool sfinae_cond, class v>
struct DoWhile :
    func<replace_std::uniq<[](){}>>,
    DoWhile<
        func, 
        stopcond, 
        N+1, 
        replace_std::is_same_v<type_var::value<stopcond>, cexpr_lib_aux::True>, 
        v
    > 
{};

template <template<class> class func, class stopcond, unsigned N, class v>
struct DoWhile<func, stopcond, N, false, v> {};

#endif
#endif 

};