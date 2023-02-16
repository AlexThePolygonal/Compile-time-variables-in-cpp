#include "lib.hpp"

using namespace type_var;



struct test {};
static_assert(cexpr_counter::load<test>() == 0);

struct a {};
struct b {};




STORE<a, a>;
STORE<a, int>;
STORE<b, value<a>>;
STORE<a, bool>;

static_assert(replace_std::is_same_v<value<b>, int>);
static_assert(replace_std::is_same_v<value<a>, bool>);



// template <bool, class T, class F, class G, auto v = [](){}> 
// struct cond_subst;


// template <bool, class T, class F, class G, auto v> 
// struct cond_subst : T:: template call<G> {
// };

// template <class T, class F, class G, auto v>
// struct cond_subst<true, T, F, G, v> : T:: template call<F>{
// };


struct tag {};


CEXPR_DO type_var::Store<tag, int>;

// template <class name>
// struct setter {
//     template <class T, auto=[](){}>
//     struct call : type_var::Store<name, T> {};
// };

CEXPR_DO cond_subst<true, setter<tag>, float, double>;

static_assert(replace_std::is_same_v<float, value<tag>>);


template <class name, unsigned N>
struct SmolRecurse : type_var::Store<name, next<value<name>>>, SmolRecurse<name, N-1> {};

template <class name>
struct SmolRecurse<name, 0> {};

struct tag2 {};
STORE<tag2, int>;

template struct SmolRecurse<tag2, 3>;

static_assert(replace_std::is_same_v<value<tag2>, next<next<next<int>>>>);




// template <class func_wrap, unsigned N, auto v = [](){}>
// struct Recurse :func_wrap:: template call<[](){}>,  Recurse<func_wrap, N-1, v> {};

// template <class func_wrap, auto v> 
// struct Recurse<func_wrap, 0, v> {};


template <class name>
struct MakeList {
    template <auto v>
    struct call : type_var::Store<name, next<value<name>>, v> {};
};

struct tag3 {};
STORE<tag3, long>;

CEXPR_DO Recurse<MakeList<tag3>, 2>;
CEXPR_DO Recurse<MakeList<tag3>, 1>;

static_assert(replace_std::is_same_v<value<tag3>, next<next<next<long>>>>);





template <class name, class stopvar>
struct MakeList4 {
    template <auto v>
    struct call : 
        type_var::Store<name, next<value<name>>, v>, 
        cond_subst<
            replace_std::is_same_v<
                value<name>, 
                next<next<next<next<double>>>> 
            >,
            setter<stopvar>,
            False,
            True
        > 
    {};
};

struct tag4{};
struct stopvar4{};

STORE<tag4, double>;
STORE<stopvar4, True>;

CEXPR_DO While<MakeList4<tag4, stopvar4>, stopvar4, 0, true>;

static_assert(replace_std::is_same_v<value<stopvar4>, False>);
static_assert(replace_std::is_same_v<value<tag4>, next<next<next<next<double>>>>>);


int main() {
}