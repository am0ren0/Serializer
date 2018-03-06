#include <Serializer.h>
using namespace bdf;

#include <sstream>
#include <iterator> // for ostream_iterator
#include <vector>
#include <map>
using namespace std;

struct A {
    int a, b;
};

template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const A & a) {
    return (s << a.a << a.b);
}
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, A & a) {
    return (s >> a.a >> a.b);
}

int main() {
    {
        stringstream ss;
        auto s = serializer(ss);

        vector<A> aa = {{1,2},{3,4},{5,6}};

        s << aa;

        vector<A> bb;
        s >> bb;
        ;

    }

    if(0){
        std::vector<int> vv = {1,2,3,4,5};
        std::map<int,int> mm = {{1,11},{2,22},{3,33}};
        string str = "hola";
        int i = 666;

        stringstream ss;
        auto s = serializer(ss);

        s << vv << i << str << mm;

        for (const auto & i: vv) std::cout << i << ' ';
        cout << endl;
        for (const auto & i: mm) std::cout << i.first << "=" << i.second << ' ';
        cout << endl;
        cout << i << endl;
        cout << str << endl;

        vv.clear();
        mm.clear();
        str.clear();
        i = 0;

        s >> vv >> i >> str >> mm;

        for (const auto & i: vv) std::cout << i << ' ';
        cout << endl;
        for (const auto & i: mm) std::cout << i.first << "=" << i.second << ' ';
        cout << endl;
        cout << i << endl;
        cout << str << endl;
    }

    {
        stringstream ss;
        auto s = serializer<BDF_LITTLE_ENDIAN>(ss);

        int a = 1;
        s << a;
        int b=0;
        s >> b;
        cout << b << endl;
    }
    {
        stringstream ss;
        auto s = serializer<BDF_BIG_ENDIAN>(ss);

        int a = 1;
        s << a;
        int b=0;
        s >> b;
        cout << b << endl;
    }

    return 0;
}
