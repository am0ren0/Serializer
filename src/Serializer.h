#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cinttypes>

namespace bdf {

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
template<typename Stream, int Endian=BDF_SYSTEM_ENDIAN>
struct Serializer {

    typedef Serializer<Stream,Endian> Self;
    Serializer(Stream & ss_) : ss(ss_) {
        static_assert(Endian==BDF_LITTLE_ENDIAN || Endian==BDF_BIG_ENDIAN, "Invalid Endian");
    }

    // write
    template<typename T, typename std::enable_if< std::is_integral<T>{} || std::is_floating_point<T>{}, int>::type = 0>
    Self & operator << (T v) {
        Swapper<Endian!=BDF_SYSTEM_ENDIAN>::swap(v);
        ss.write(reinterpret_cast<const char *>(&v), sizeof(T));
        return *this;
    }

    // read
    template<typename T, typename std::enable_if< std::is_integral<T>{} || std::is_floating_point<T>{}, int>::type = 0>
    Self & operator >> (T & v) {
        ss.read(reinterpret_cast<char *>(&v), sizeof(T));
        Swapper<Endian!=BDF_SYSTEM_ENDIAN>::swap(v);
        return *this;
    }

    Stream & ss;
};

// constructor
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream>
Serializer<Stream,Endian> serializer(Stream & ss) { return Serializer<Stream,Endian>(ss); }

// std::string
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::string & str) {
    s << str.size();
    s.ss.write(&str[0],str.size());
    return s;
}
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::string & str) {
    size_t n;
    s >> n;
    str.resize(n);
    s.ss.read(&str[0],n);
    return s;
}

// std::string
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream, typename T1, typename T2>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::pair<T1,T2> & p) { return (s << p.first << p.second); }
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream, typename T1, typename T2>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::pair<T1,T2> & p) { return (s >> p.first >> p.second); }

// std::vector
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream, typename T>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::vector<T> & vv) { s << vv.size(); for(auto it=vv.begin(); it!=vv.end(); ++it) s << *it; return s; }
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream, typename T>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::vector<T> & vv) {
    size_t n;
    s >> n;
    vv.clear();
    vv.resize(n);
    for(size_t i=0; i<n; ++i)
        s >> vv[i];
    return s;
}

// std::map
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream, typename T1, typename T2>
Serializer<Stream,Endian> & operator << (Serializer<Stream,Endian> & s, const std::map<T1,T2> & mm) { s << mm.size(); for(auto it=mm.begin(); it!=mm.end(); ++it) s << *it; return s; }
template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream, typename T1, typename T2>
Serializer<Stream,Endian> & operator >> (Serializer<Stream,Endian> & s, std::map<T1,T2> & mm) {
    size_t n;
    s >> n;
    mm.clear();
    std::pair<T1,T2> p;
    for(size_t i=0; i<n; ++i) {
        s >> p;
        mm[p.first] = p.second;
    }
    return s;
}

}   // namespace bdf
