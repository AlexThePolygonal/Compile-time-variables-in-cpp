# Compile-time type variables

This mini-library implements *stateful metaprogramming*, that is, mutable constexpr "variables". Sounds insane, right?

## A first taste of power
This a useless toy example, don't take it seriously.

```cpp

struct Foo1 : // this is a "variable"
    type_var::Store<Foo1, int> // we store a value in it
    {
};

struct Foo2 : // this is another "variable"
    type_var::Store<Foo2, long> // we store a value in it
    {
};

template <class Var>
struct Bar {
    using TypeValue = value<Var>; // here we extract the value of the variable Var
};

template struct Bar<Foo1>; // instantiate it for Foo1;

// But a mistake happened. We need to change the value of Foo2;
// Fret not!

struct change_value : 
    type_var::Store<Foo2, int> // change the value of Foo2
    {
};

static_assert(std::is_same_v<typename Bar<Foo2>::TypeValue, int>); // passes
```
The real use for this is more convenient metaprogramming and code generation.
## Requirements
Tested with clang15 and gcc12.
- Clang crashes frequently, but if it compiles, it works perfectly
- GCC always compiles, but is unable to stomach heavy recursion. Use ```DoWhile``` with caution

On GCC, always use flags ```-Wno-non-template-friend -Wno-return-type```

This code is only possible thanks to the amazing innovations ```c++20``` standard. Always use it!

## How to use

Look at the code, there isn't much there.

For examples, look at the tests in main.cpp.

We have:
- A monotonic constexpr counter, implemented in namespace ```cexpr_counter```
- The type variable, implemented in namespace ```type_variable```. It can also be described as a mutable list of immutable types.
    -  Instantiating the ```Store<Name, Value>``` structure will append ```Value``` to the list of the variable```Name```
    -  The template ```value<Name>```  gives the the last element of the list of the variable ```Name```
    -  The template ```len<Name>``` gives the length of the list
    -  The template  ```value_at<Name,N>``` gives the *N*-th value in the list  
    -  The template ```pack_of<Name>``` returns the whole list, wrapped in ```replace_std::Argspass<...>```
- Flow control operations:
    - Instantiating ```IfSubst<cond, func, arg>```, if ```cond``` is true,will instantiate ```func<arg>```, triggering its side-effects.
    - Instantiating ```IfElseSubst<cond, func, arg_if_true, arg_if_false>```, if ```cond``` is true,will instantiate ```func<arg_if_true>```, triggering its side-effects. Otherwise, ```func<arg_if_false>``` will be instantiated.
    - Instantiating ```Recurse<func,N>``` will instantiate ```func<>``` with all the side-effects exactly ```N``` times.
    - Instantiating ```DoWhile<func, stop_condition>``` will call ```func<>``` while the variable ```stop_condition``` does not contain ```replace_std::True```.

This is enough for simple Python-like coding.

## Pitfalls

Compilers cache templates by default, so the following simple example is not going to work:
```cpp
struct Var {};
template struct type_var::Store<Var, bool>; // Var now contains bool

struct CachedFoo {
    using Baz = value<Var>;
};

template struct type_var::Store<Var, int>; // Var now contains int

// Not going to work!
static_assert(is_same_v<typename CachedFoo::Baz, bool>); // evaluates to false
```
To evade that, make the compiler refresh the values in Foo like this:
```cpp

template <class = replace_std::uniq<>>
struct CorrectFoo {};
```
In the 2020 standard, the expression ```[](){}``` will always have a unique type, and ```CorrectFoo``` will be a new type each time, recomputing the state-dependent parameters each time.