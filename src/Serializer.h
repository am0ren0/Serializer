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

    Self & operator << (const std::string & str) {
        *this << str.size();
        ss.write(&str[0],str.size());
        return *this;
    }

    template<typename T1, typename T2>
    Self & operator << (const std::pair<T1,T2> & p) { return (*this << p.first << p.second); }

    template<typename T>
    Self & operator << (const std::vector<T> & vv) { *this << vv.size(); for(auto it=vv.begin(); it!=vv.end(); ++it) *this << *it; return *this; }
    template<typename T1, typename T2>
    Self & operator << (const std::map<T1,T2> & mm) { *this << mm.size(); for(auto it=mm.begin(); it!=mm.end(); ++it) *this << *it; return *this; }

    // read
    template<typename T, typename std::enable_if< std::is_integral<T>{} || std::is_floating_point<T>{}, int>::type = 0>
    Self & operator >> (T & v) {
        ss.read(reinterpret_cast<char *>(&v), sizeof(T));
        Swapper<Endian!=BDF_SYSTEM_ENDIAN>::swap(v);
        return *this;
    }

    Self & operator >> (std::string & str) {
        size_t n;
        *this >> n;
        str.resize(n);
        ss.read(&str[0],n);
        return *this;
    }

    template<typename T1, typename T2>
    Self & operator >> (std::pair<T1,T2> & p) { return (*this >> p.first >> p.second); }

    template<typename T>
    Self & operator >> (std::vector<T> & vv) {
        size_t n;
        *this >> n;
        vv.clear();
        vv.resize(n);
        for(size_t i=0; i<n; ++i)
            *this >> vv[i];
        return *this;
    }
    template<typename T1, typename T2>
    Self & operator >> (std::map<T1,T2> & mm) {
        size_t n;
        *this >> n;
        mm.clear();
        std::pair<T1,T2> p;
        for(size_t i=0; i<n; ++i) {
            *this >> p;
            mm[p.first] = p.second;
        }
        return *this;
    }

private:
    Stream & ss;
};

template<int Endian=BDF_SYSTEM_ENDIAN, typename Stream>
Serializer<Stream,Endian> serializer(Stream & ss) { return Serializer<Stream,Endian>(ss); }

}   // namespace bdf
