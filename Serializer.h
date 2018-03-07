#pragma once

#include <cassert>
#include <utility>
#include <cstring>
#include <string>

namespace femto {

enum {
    BDF_LITTLE_ENDIAN,
    BDF_BIG_ENDIAN,
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    BDF_SYSTEM_ENDIAN = BDF_LITTLE_ENDIAN
#else
    BDF_SYSTEM_ENDIAN = BDF_BIG_ENDIAN
#endif
};

template<int nBytes>
void swapBytes(char *src);
template<typename T>
inline void swapBytes(T & src) { swapBytes<sizeof(T)>( reinterpret_cast<char *>(&src) ); }

template<> inline void swapBytes<1>(char */*src*/) {}
template<> inline void swapBytes<2>(char *src) { std::swap(src[0], src[1]); }
template<> inline void swapBytes<4>(char *src) { std::swap(src[0], src[3]); std::swap(src[1], src[2]); }
template<> inline void swapBytes<8>(char *src) { std::swap(src[0], src[7]); std::swap(src[1], src[6]); std::swap(src[2], src[5]); std::swap(src[3], src[4]); }

template<bool Enabled> struct Swapper;
template<> struct Swapper<false> { template<typename T> static void swap(T &) {} };
template<> struct Swapper<true > { template<typename T> static void swap(T & v) { swapBytes(v); } };

/** Serializer class
 *
 */
template<typename Stream, int Endian>
struct Serializer {

    typedef Serializer<Stream,Endian> Self;
    Serializer(Stream & ss_) : ss(ss_) {
        static_assert(Endian==BDF_LITTLE_ENDIAN || Endian==BDF_BIG_ENDIAN, "Invalid Endian");
    }

    // write
    Self & write(const char * str, size_t n) { ss.write(str,n); return *this;}

    template<typename T0, typename ...T>
    Self & write(const T0 & a0, const T & ...aa) {
        *this << a0;
        return write(aa...);
    }
    template<typename T>
    Self & write(const T & a) {
        return (*this << a);
    }

    template<typename T, typename std::enable_if< std::is_integral<T>{} || std::is_floating_point<T>{}, int>::type = 0>
    Self & operator << (T v) {
        Swapper<Endian!=BDF_SYSTEM_ENDIAN>::swap(v);    // swapping the local copy of v
        write(reinterpret_cast<const char *>(&v), sizeof(T));
        return *this;
    }
    // literal strings. use write for buffers not ending with the null char
    Self & operator << (const char * str) {
        size_t n = std::strlen(str);    // assume null ended char string
        *this << n;
        write(&str[0],n);
        return *this;
    }

    // read
    size_t read(char * str) {
        size_t n;
        *this >> n;
        ss.read(str,n);
        return n;
    }
    void read(char * str, size_t n) {
        assert(n>0);
        ss.read(str,n);
    }

    template<typename T0, typename ...T>
    void read(T0 & a0, T & ...aa) {
        *this >> a0;
        read(aa...);
    }
    template<typename T>
    void read(T & a) {
        *this >> a;
    }

    template<typename T, typename std::enable_if< std::is_integral<T>{} || std::is_floating_point<T>{}, int>::type = 0>
    Self & operator >> (T & v) {
        read(reinterpret_cast<char *>(&v), sizeof(T));
        Swapper<Endian!=BDF_SYSTEM_ENDIAN>::swap(v);
        return *this;
    }
    // literal strings. use read for buffers to avoid adding null char at the end
    Self & operator >> (char * str) {
        size_t n = read(str);
        str[n] = '\0';  // set null char at the end of the string
        return *this;
    }

    template<typename T>
    T get() {
        T v;
        *this >> v;
        return v;
    }

private:
    Stream & ss;
};

// utility constructor
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream>
Serializer<Stream,Endian> serializer(Stream & ss) { return Serializer<Stream,Endian>(ss); }

#define SERIALIZE_MEMBERS(_class_, ...)\
    template<int Endian, typename Stream>\
    void write(Serializer<Stream,Endian> & s) const { s.write(__VA_ARGS__); }\
    template<int Endian, typename Stream>\
    friend Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const _class_ & obj) { obj.write(s); return s; }\
    template<int Endian, typename Stream, typename ...T>\
    void read(Serializer<Stream,Endian> & s) { s.read(__VA_ARGS__); }\
    template<int Endian, typename Stream>\
    friend Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, _class_ & obj) { obj.read(s); return s; }

}   // namespace bdf

// forward definitions of std classes
namespace std {
template<class Value, size_t N>                                         struct array;
template<class Value, class Alloc>                                      class vector;
template<class Key, class Comp, class Alloc>                            class set;
template<class Key, class Hash, class Pred, class Alloc>                class unordered_set;
template<class Key, class Value, class Comp, class Alloc>               class map;
template<class Key, class Value, class Hash, class Pred, class Alloc>   class unordered_map;
}

namespace femto {
// std::string
template<int Endian, typename Stream>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::string & str) {
    s << str.size();
    s.write(&str[0],str.size());
    return s;
}
template<int Endian, typename Stream>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::string & str) {
    size_t n = s.template get<size_t>();
    str.resize(n);
    s.read(&str[0],n);
    return s;
}

// std::pair
template<int Endian, typename Stream, typename T1, typename T2>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::pair<T1,T2> & p) { return (s << p.first << p.second); }
template<int Endian, typename Stream, typename T1, typename T2>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::pair<T1,T2> & p) { return (s >> p.first >> p.second); }

// std::array
template<int Endian, typename Stream, typename T, size_t N>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::array<T,N> & aa) {
    for(int i=0; i<N; ++i)
        s << aa[i];
    return s;
}
template<int Endian, typename Stream, typename T, size_t N>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::array<T,N> & aa) {
    for(int i=0; i<N; ++i)
        s >> aa[i];
    return s;
}

// std::vector
template<int Endian, typename Stream, typename T, typename Alloc>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::vector<T, Alloc> & vv) { s << vv.size(); for(auto it=vv.begin(); it!=vv.end(); ++it) s << *it; return s; }
template<int Endian, typename Stream, typename T, typename Alloc>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::vector<T, Alloc> & vv) {
    size_t n = s.template get<size_t>();
    vv.clear();
    vv.resize(n);
    for(size_t i=0; i<n; ++i)
        s >> vv[i];
    return s;
}

// std::set
template<int Endian, typename Stream, typename T, typename Comp, typename Alloc>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::set<T, Comp, Alloc> & ss) { s << ss.size(); for(auto it=ss.begin(); it!=ss.end(); ++it) s << *it; return s; }
template<int Endian, typename Stream, typename T, typename Comp, typename Alloc>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::set<T, Comp, Alloc> & ss) {
    size_t n = s.template get<size_t>();
    ss.clear();
    auto prev = ss.cbegin();
    for(size_t i=0; i<n; ++i) {
        prev = ss.emplace_hint(prev,s.template get<T>());
    }
    return s;
}

// std::unordered_set
template<int Endian, typename Stream, typename T, class Hash, class Pred, class Alloc>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::unordered_set<T, Hash,Pred,Alloc> & ss) { s << ss.size(); for(auto it=ss.begin(); it!=ss.end(); ++it) s << *it; return s; }
template<int Endian, typename Stream, typename T, class Hash, class Pred, class Alloc>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::unordered_set<T, Hash,Pred,Alloc> & ss) {
    size_t n = s.template get<size_t>();
    ss.clear();
    ss.reserve(n);
    auto prev = ss.cbegin();
    for(size_t i=0; i<n; ++i) {
        prev = ss.emplace_hint(prev, s.template get<T>());
    }
    return s;
}

// std::map
template<int Endian, typename Stream, typename T1, typename T2, typename Comp, typename Alloc>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::map<T1,T2, Comp, Alloc> & mm) { s << mm.size(); for(auto it=mm.begin(); it!=mm.end(); ++it) s << *it; return s; }
template<int Endian, typename Stream, typename T1, typename T2, typename Comp, typename Alloc>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::map<T1,T2, Comp, Alloc> & mm) {
    size_t n = s.template get<size_t>();
    mm.clear();
    typedef std::pair<T1,T2> Pair;
    auto prev = mm.cbegin();
    for(size_t i=0; i<n; ++i) {
        prev = mm.emplace_hint(prev, s.template get<Pair>());
    }
    return s;
}

// std::unordered_map
template<int Endian, typename Stream, typename T1, typename T2, class Hash, class Pred, class Alloc>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::unordered_map<T1,T2, Hash,Pred,Alloc> & mm) { s << mm.size(); for(auto it=mm.begin(); it!=mm.end(); ++it) s << *it; return s; }
template<int Endian, typename Stream, typename T1, typename T2, class Hash, class Pred, class Alloc>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::unordered_map<T1,T2, Hash,Pred,Alloc> & mm) {
    size_t n = s.template get<size_t>();
    mm.clear();
    mm.reserve(n);
    typedef std::pair<T1,T2> Pair;
    auto prev = mm.cbegin();
    for(size_t i=0; i<n; ++i) {
        prev = mm.emplace_hint(prev, s.template get<Pair>());
    }
    return s;
}

}   // namespace bdf
