#include <typeinfo>
#include <iostream>
#include <type_traits>

using namespace std;

template<int I, typename T>
typename enable_if<I == 8>::type
test(int i, T t){
    cout << "out of range" << endl;
}

template<int I = 0, typename T>
typename enable_if<I < 8>::type
test(int i, T t){
    if(i != I){
        cout << I << endl;
        test<I + 1>(i, t);
    }
}

struct A{
};

int main(){
    test(3, 4);
    using t = typename result_of<decltype(&test<1, int>)(int, int)>::type;

    return 0;
}
