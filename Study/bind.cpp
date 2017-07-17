#include <iostream>
#include <functional>
#include <type_traits>

template<int... Indexs>
struct IndexSeq{};

template<int N, int... Rest>
struct MakeIndexs : MakeIndexs<N - 1, N - 1, Rest...>{};

template<int... Rest>
struct MakeIndexs<0, Rest...>
{
    typedef IndexSeq<Rest...> type;
};

template<int I>
struct Placeholder{};

template<typename T, typename Tuple>
auto SelectArg(T&& t, Tuple& tp)->T&&
{
    return std::forward<T>(t);
}

template<int I, typename Tuple>
auto SelectArg(Placeholder<I>&, Tuple& tp)->decltype(std::get<I - 1>(tp))
{
    return std::get<I - 1>(tp);
}

template<typename F, typename... Args>
class Bind_t{
    Bind_t(F f, std::tuple<Args...> t):m_f(f), m_t(t){}
    
    template<typename... CArgs>
    auto operator()()->typename std::result_of<F>::type{
    }

private:
    F m_f;
    std::tuple<Args...> m_t;
};

void fun(int a){}

int main(){
    //std::cout << typeid(MakeIndexs<3>::type).name() << std::endl;

    Placeholder<1> _1;
    Placeholder<2> _2;
    Placeholder<3> _3;

    auto t = std::make_tuple(1, "hello", 2.3);
    std::cout << SelectArg(_2, t) << std::endl; 

    std::cout << typeid(std::result_of<decltype(&fun)(int)>::type).name() << std::endl;

    return 0;
}
