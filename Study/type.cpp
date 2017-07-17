#include <tuple>
#include <iostream>
#include <type_traits>

using namespace std;

template<typename T>
void print(T t)
{
    cout << t << endl;
}

template<typename T, typename... Args>
void print(T t, Args... args)
{
    print(t);
    print(args...);
}

namespace MyDelegate
{
    template<typename T, typename R, typename... Args>
    class _MyDelegate
    {
    public:
        _MyDelegate(T *t, R (T::*f)(Args...)):m_t(t), m_f(f){}

        R operator()(Args&&... args)
        {
            return (m_t->*m_f)(std::forward<Args>(args)...);
        }
    private:
        T *m_t;
        R (T::*m_f)(Args...);
    };

    template<typename T, typename R, typename... Args>
    _MyDelegate<T, R, Args...> CreateDelegate(R (T::*f)(Args...), T *t)
    {
        return _MyDelegate<T, R, Args...>(t, f);
    }
};


struct Test{
    void Fun(int a){cout << a << endl;}
};

template<template<typename...> class T, typename N>
struct is_type_of: public std::false_type{};

template<template<typename...> class T, typename... Args>
struct is_type_of<T, T<Args...>>: public std::true_type{};

template<template<typename...> class T, typename N>
bool isTypeOf(N){
    return is_type_of<T, typename std::decay<N>::type>::value;
}

namespace detail{
    template<template<typename...> class T, typename N>
    std::false_type isTypeOf(N);

    template<template<typename...> class T, typename... Args>
    std::true_type isTypeOf(T<Args...>);

    template<template<typename...> class T, typename N>
    using is_type_of = decltype(isTypeOf<T>(declval<typename std::decay<N>::type>()));
};

int main(){
    //auto tp = std::make_tuple(1, 2.1, "hello");
    //cout << std::get<1>(tp) << endl;

    Test t;
    auto func = MyDelegate::CreateDelegate(&Test::Fun, &t);
    func(3);

    std::cout << isTypeOf<std::tuple>(std::make_tuple(1, "hello")) << std::endl;
    std::cout << isTypeOf<std::tuple>(1) << std::endl;

    std::cout << detail::is_type_of<std::tuple, std::tuple<int>>::value << std::endl;
    std::cout << detail::is_type_of<std::tuple, int>::value << std::endl;
    
    return 0;
}

