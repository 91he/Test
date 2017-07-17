#include <iostream>

template<class T>
struct RangeImpl{
    struct RangeImplIterator{
        RangeImplIterator& operator++(){
            n += step;
            return *this;
        }

        T operator*(){
            return n;
        }

        bool operator!=(const RangeImplIterator& rhs)const {
            return n < rhs.n;
        }

        T n;
        T step;
    };

    RangeImplIterator begin(){
        return RangeImplIterator{be, step};
    }

    RangeImplIterator end(){
        return RangeImplIterator{ed, step};
    }

    T be;
    T ed;
    T step;
};

template<class T>
auto Range(T n){
    return RangeImpl<T>{T(0), n, T(1)};
}

template<class T>
auto Range(T b, T e, T s = T(1)){
    return RangeImpl<T>{b, e, s};
}

int main(){
    for(auto iter : Range(5))
        std::cout << iter << std::endl;
    return 0;
}
