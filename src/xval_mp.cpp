
#include "xval_mp.h"
#include "xval_list.h"
#include "xval_dict.h"
#include "xval_str.h"

#if (defined _WIN32) || (defined _WIN64)
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <endian.h>
#include <arpa/inet.h>
#define htonll(I) htobe64((I))
#define ntohll(I) be64toh((I))
#endif

using namespace xval;

inline uint8_t  hton(uint8_t b) { return b; }
inline uint16_t hton(uint16_t s) { return htons(s); }
inline uint32_t hton(uint32_t l) { return htonl(l); }
inline uint64_t hton(uint64_t ll) { return htonll(ll); }

inline uint8_t  ntoh(uint8_t b) { return b; }
inline uint16_t ntoh(uint16_t s) { return ntohs(s); }
inline uint32_t ntoh(uint32_t l) { return ntohl(l); }
inline uint64_t ntoh(uint64_t ll) { return ntohll(ll); }

enum {
    PRE_FIXMAP = 0x80,
    PRE_FIXARR = 0x90,

    PRE_NEGINT = 0xE0,

    V_NIL = 0xC0,
    V_FALSE = 0xC2, V_TRUE = 0xC3,
    V_BIN8 = 0xC4, V_BIN16 = 0xC5, V_BIN32 = 0xC6,
    V_EXT8 = 0xC7, V_EXT16 = 0xC8, V_EXT32 = 0xC9,
    V_FLOAT32 = 0xCA, V_FLOAT64 = 0xCB,
    V_UINT8 = 0xCC, V_UINT16 = 0xCD, V_UINT32 = 0xCE, V_UINT64 = 0xCF,
    V_INT8 = 0xD0, V_INT16 = 0xD1, V_INT32 = 0xD2, V_INT64 = 0xD3,
    V_FIXEXT1 = 0xD4,
    V_FIXEXT2 = 0xD5,
    V_FIXEXT4 = 0xD6,
    V_FIXEXT8 = 0xD7,
    V_FIXEXT16 = 0xD8,
    V_STR8 = 0xD9, V_STR16 = 0xDA, V_STR32 = 0xDB,
    V_ARR16 = 0xDC, V_ARR32 = 0xDD,
    V_MAP16 = 0xDE, V_MAP32 = 0xDF,
};

struct Buf {
    Buf(ostream& o) {
        p = &buf[1]; os = &o;
    }

    operator bool() {
        return success;
    }

    template<typename T>
    inline void write(uint8_t pre, T len) {
        buf[0] = pre;
        *(T *)&buf[1] = hton(len);
        success = os->write(buf, sizeof(T) + 1) ?
                    true : false;
    }

    inline void write_b(uint8_t pre) {
        buf[0] = pre;
        success = os->write(buf, 1) ?
                    true : false;
    }
    inline void write8(uint8_t pre, uint8_t len) {
        write<uint8_t>(pre, len);
    }
    inline void write16(uint8_t pre, uint16_t len) {
        write<uint16_t>(pre, len);
    }
    inline void write32(uint8_t pre, uint32_t len) {
        write<uint32_t>(pre, len);
    }
    inline void write64(uint8_t pre, uint64_t len) {
        write<uint64_t>(pre, len);
    }

    union {
        char *p;
        uint8_t *pb;
        uint16_t *ps;
        uint32_t *pl;
        uint64_t *pll;
    };
    ostream *os;
    char buf[10];
    bool success = false;
};

static bool pack(List& l, ostream& o) {
    return mp_pack(l.begin(), l.size(), o);
}

static bool pack(Tuple& t, ostream& o) {
    return mp_pack(t.begin(), t.size(), o);
}

static bool pack(Dict& d, ostream& o) {
    Buf buf(o);

    if (d.size() < 0x10)
        buf.write_b(PRE_FIXMAP | d.size());
    else if (d.size() < 0x10000)
        buf.write16(V_MAP16, d.size());
    else if (d.size() < 0x100000000)
        buf.write16(V_MAP32, d.size());

    if (!buf)
        return false;
    for (auto i : d)
        if (!(mp_pack(i.key(), o) && mp_pack(i.value(), o)))
            return false;
    return true;
}

static bool pack(int64_t i, ostream& o) {
    Buf buf(o);

    if (i < 0x10000) {              // int16 && negative
        if (i < 0) {                // negative int
            if (i > -33)                    // negative fixint
                buf.write_b(i & 0xFF);
            else if (i >= (int64_t)(int8_t)0x80)
                buf.write8(V_INT8, i & 0xFF);
            else if (i >= (int64_t)(int16_t)0x8000)
                buf.write16(V_INT16, i);    // int 16
            else if (i >= (int64_t)(int32_t)0x80000000)
                buf.write32(V_INT32, i);    // int 32
            else
                buf.write64(V_INT64, i);    // int 64
        } else if (i < 0x80)                // fixint
            buf.write_b(i & 0xFF);
        else if (i < 0x100)                 // uint 8
            buf.write8(V_UINT8, i & 0xFF);
        else                                // uint 16
            buf.write16(V_UINT16, i);
    } else if (i < 0x100000000)             // uint 32
        buf.write32(V_UINT32, i);
    else                                    // uint 64
        buf.write64(V_UINT64, i);

    return buf;
}

static bool pack(double f, ostream& o) {
    Buf buf(o);
    buf.write64(V_FLOAT64, *(uint64_t *)(&f));
    return buf;
}

static bool pack(String &s, ostream& o) {
    return mp_pack(s.c_str(), s.size(), s.isbin(), o);
}

static Value readstr(istream& i, size_t len) {
    auto str = String::New(i, len);
    if (str.isnil())
        return Value::Nil;
    str.str().isbin(false);
    return str;
}

static Value readbin(istream& i, size_t len) {
    auto str = String::New(i, len);
    if (str.isnil())
        return Value::Nil;
    str.str().isbin(true);
    return str;
}

static bool readarray(istream& i, size_t len, Value& o) {
    if (o.islist())
        o.list().resize(0);
    else
        o = List::New(len);
    auto& list = o.list();
    for (Value v; len--; list.append(v))
        if (!mp_unpack(v, i))
            return false;
    return true;
}

static bool readmap(istream& i, size_t len, Value& o) {
    if (o.isdict())
        o.dict().clear();
    else
        o = Dict::New(len);
    Value k, v;
    auto& dict = o.dict();
    while (len--) {
        if (!mp_unpack(k, i))
            return false;
        if (!mp_unpack(v, i))
            return false;
        dict.set(k, v);
    }
    return true;
}

template <typename T>
static bool unpack(istream& i, T& n) {
    return i.read((char *)&n, sizeof(n)) ?
        (n = ntoh(n), true) : false;
}
static bool unpack(istream& i, float &f) {
    return unpack(i, (uint32_t &)f);
}
static bool unpack(istream& i, double &f) {
    return unpack(i, (uint64_t &)f);
}

static bool unpackvar(istream& i, unsigned char n, Value& v) {
    union {
        float f; double d;
        int8_t b; uint8_t ub;
        int16_t s; uint16_t us;
        int32_t l; uint32_t ul;
        int64_t ll; uint64_t ull;
    } val;
    switch (n) {
        case V_BIN8:
            return unpack(i, val.ub) ?
                (v = readbin(i, val.ub), (bool)v) : false;
        case V_BIN16:
            return unpack(i, val.us) ?
                (v = readbin(i, val.us), (bool)v) : false;
        case V_BIN32:
            return unpack(i, val.ul) ?
                (v = readbin(i, val.ul), (bool)v) : false;
        case V_FLOAT32:
            return unpack(i, val.f) ?
                (v = Value(val.f), true) : false;
        case V_FLOAT64:
            return unpack(i, val.d) ?
                (v = Value(val.d), true) : false;
        case V_UINT8:
            return unpack(i, val.ub) ?
                (v = Value((int64_t)val.ub), true) : false;
        case V_UINT16:
            return unpack(i, val.us) ?
                (v = Value((int64_t)val.us), true) : false;
        case V_UINT32:
            return unpack(i, val.ul) ?
                (v = Value((int64_t)val.ul), true) : false;
        case V_UINT64:
            return unpack(i, val.ull) ?
                (v = Value((int64_t)val.ull), true) : false;
        case V_INT8:
            return unpack(i, val.ub) ?
                (v = Value((int64_t)val.b), true) : false;
        case V_INT16:
            return unpack(i, val.us) ?
                (v = Value((int64_t)val.s), true) : false;
        case V_INT32:
            return unpack(i, val.ul) ?
                (v = Value((int64_t)val.l), true) : false;
        case V_INT64:
            return unpack(i, val.ull) ?
                (v = Value(val.ll), true) : false;
        case V_STR8:
            return unpack(i, val.ub) ?
                (v = readstr(i, val.ub), (bool)v) : false;
        case V_STR16:
            return unpack(i, val.us) ?
                (v = readstr(i, val.us), (bool)v) : false;
        case V_STR32:
            return unpack(i, val.ul) ?
                (v = readstr(i, val.ul), (bool)v) : false;
        case V_ARR16:
            return unpack(i, val.us) ? readarray(i, val.us, v) : false;
        case V_ARR32:
            return unpack(i, val.ul) ? readarray(i, val.ul, v) : false;
        case V_MAP16:
            return unpack(i, val.us) ? readmap(i, val.ul, v) : false;
        case V_MAP32:
            return unpack(i, val.ul) ? readmap(i, val.ul, v) : false;
        default: /*
            case V_EXT8: case V_EXT16: case V_EXT32:
            case V_FIXEXT1: case V_FIXEXT2:
            case V_FIXEXT4: case V_FIXEXT8:
            case V_FIXEXT16: // */
            return false;
    }
    return false;
}
static bool unpackfix(istream& i, unsigned char n, Value& v) {
    size_t len = n & 0x0F;
    if (n < 0xC0) switch (n & 0xF0) {
        case PRE_FIXMAP:
            return readmap(i, len, v);
        case PRE_FIXARR:
            return readarray(i, len, v);
        default:            // fixstr (0xA0 - 0xBF, 101xxxxx)
            v = readstr(i, n & 0x1F); break;
    } else switch (n) {
        case V_NIL:
            return (v = Value::Nil, true);
        case V_FALSE:
            return (v = false, true);
        case V_TRUE:
            return (v = true, true);
        default:
            return (v = Value::Nil, false);
    }
    return !v.isnil();
}

namespace xval {

    bool mp_pack(const char *str, size_t len, bool bin, ostream& o) {
        Buf buf(o);
        char b8 = V_STR8, b16 = V_STR16, b32 = V_STR32;
        if (bin)
            b8 = V_BIN8, b16 = V_BIN16, b32 = V_BIN32;

        if (len < 0x20 && !bin)
            buf.write_b((0xA0 | len) & 0xFF);
        else if (len < 0x100)
            buf.write8(b8, len);
        else if (len < 0x10000)
            buf.write16(b16, len);
        else if (len < 0x100000000)
            buf.write32(b32, len);

        return buf ?
            (bool)o.write(str, len) : false;
    }

    bool mp_pack(const char *str, ostream& o) {
        return mp_pack(str, strlen(str), false, o);
    }

    bool mp_pack(bool b, ostream& o) {
        char c = b ? V_TRUE : V_FALSE;
        return o.write(&c, 1) ? true : false;
    }

    bool mp_pack(const Value& v, ostream& o) {
        char c;
        switch (v.type()) {
            case TV_NIL:
                c = V_NIL;
                return o.write(&c, 1) ? true : false;
            case TV_BOOL:
                return mp_pack(v.Bool(), o);
            case TV_INT: case TV_POINTER:
                return ::pack(v.Int(), o);
            case TV_FLOAT:
                return ::pack(v.Float(), o);
            case TV_STRING:
                return ::pack(v.str(), o);
            case TV_TUPLE:
                return ::pack(v.tuple(), o);
            case TV_LIST:
                return ::pack(v.list(), o);
            case TV_DICT:
                return ::pack(v.dict(), o);
            default:
                return false;
        }
    }

    bool EXPORT mp_pack(const Value *p, size_t len, ostream& o) {
        Buf buf(o);
        if (len < 0x10)                 // fixarray
            buf.write_b(PRE_FIXARR | len);
        else if (len < 0x10000)         // array 16
            buf.write16(V_ARR16, len);
        else if (len < 0x100000000)     // array 32
            buf.write32(V_ARR32, len);
        if (!buf) return false;
        for (size_t i = 0; i < len; i++)
            if (!mp_pack(p[i], o))
                return false;
        return true;
    }

    bool mp_unpack(Value& v, istream& i) {
        union { char c; unsigned char n; };
        if (!i.read(&c, 1)) return false;
        if (n < 0x80)                       // Positive fixint
            v = Value((int64_t)n);
        else if ((n & 0xE0) == 0xE0)        // Negative fixint
            v = Value((int64_t)c);
        else if (n < 0xC4)
            return unpackfix(i, n, v);
        else
            return unpackvar(i, n, v);
        return true;
    }

#include <streambuf>
    class membuf : public streambuf {
        public:
            membuf(const string& str) {
                auto p = (char *)str.c_str();
                setg(p, p, p + str.size());
            }
            membuf(string& str) : _str(&str) {}
        protected:
            int uflow() override { return EOF; }
            int overflow(int n) override {
                char c = n;
                _str->append(&c, 1);
                return 1;
            }
            streamsize xsputn(const char *s, streamsize size) override {
                _str->append(s, size);
                return size;
            }
        private:
            string *_str;
    };

    bool mp_pack(const Value& v, string& s) {
        membuf buf(s);
        ostream os(&buf);
        return mp_pack(v, os);
    }

    bool mp_unpack(Value &v, const string& s) {
        membuf buf(s);
        istream is(&buf);
        return mp_unpack(v, is);
    }

}
