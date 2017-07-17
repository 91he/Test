#include <tuple>
#include <array>
#include <string>
#include <iostream>

template<size_t N>
std::array<std::string, N> split(const std::string& str, const char delimiter){
    size_t pos;
    size_t begin = 0;
    size_t index = 0;
    std::array<std::string, N> ret;

    do{
        pos = str.find_first_of(delimiter, begin);

        ret[index++] = std::move(str.substr(begin, pos -  begin));

        begin = pos + 2;
    }while(pos != std::string::npos);

    return ret;
}

template<size_t N, typename T1>
constexpr static inline auto make(const std::array<std::string, N>& strs, size_t index, const T1& tp)
{
    return tp;
}

template<size_t N, typename T1, typename T2, typename... Args>
constexpr static inline auto make(const std::array<std::string, N>& strs, size_t index, const T1& tp, T2& first, Args&... args)
{
    return make(strs, index + 1, std::tuple_cat(tp, std::make_tuple(std::pair<const std::string, T2&>(strs[index], first))), args...);
    //return make(strs, index + 1, std::tuple_cat(tp, std::make_tuple(std::make_pair(strs[index], first))), args...);
}

#define VA_ARGS_NUM(...) std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value
#define META(...) auto meta(){ \
        auto strs = split<VA_ARGS_NUM(__VA_ARGS__)>(#__VA_ARGS__, ','); \
        return make(strs, 0, std::tuple<>(), __VA_ARGS__); \
    }

struct person
{
    std::string name;
    int age;
    double sorlary;
    META(name, age, sorlary) //定义一个支持变参的meta函数
};

int main()
{
    person p = {"tom", 20, 1000.0};
    auto tp = p.meta();
    std::cout << std::get<0>(tp).first << ": " << std::get<0>(tp).second << std::endl;
    std::cout << std::get<1>(tp).first << ": " << std::get<1>(tp).second << std::endl;
    std::cout << std::get<2>(tp).first << ": " << std::get<2>(tp).second << std::endl;

    //static_assert(std::is_same(std::tuple<std::pair<std::string, int>>, decltype(tp), “not same”);
    //在内存中是这样的 {{“name”:”tom”}, {“age”:20}};
    return 0;
}
