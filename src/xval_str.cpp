#include <ctype.h>
#include <istream>
#include <stddef.h>

#include "xval_str.h"
#include "xval_dict.h"
#include "xval_val.h"
#include "xval_conf.h"

static auto _strdict = xval::Dict::New();
static auto& strdict = _strdict.dict();

static uint32_t _seed = 1314;

namespace xval {
    hash_t _gethash(const char *data, size_t len) {
        hash_t hash = _seed;
        auto p = data; auto e = p + len;
        while (p < e)
            (hash *= _seed) += *p++;
        return hash ? hash : 1;
    }
    bool _isshort(const char *data, size_t len) {
        if (len > XVAL_SHORTSTR_LEN) return false;
        for (auto p = data, e = data + len; p < e; ++p)
            if (isalnum((uint8_t)*p) || *p == '_')
                continue;
            else
                return false;
        return true;
    }
    void String::_addinternal(String *str) {
        strdict.set(str, str);
    }
    String *String::_getinternal(const char *data, size_t len) {
        String *p = nullptr;
        if (_isshort(data, len))
            p = strdict.get(data, len, _gethash(data, len));
        return p;
    }

    Value::Value(const char *str, size_t len)
        : Value(String::New(str, len)) {}

    Value::operator const char *() const {
        return isstr() ? str().c_str() : nullptr;
    }

    String* String::allocate(size_t size) {
        return (String *)(new char[offsetof(String, _data) + size + 1]);
    }

    Value String::New(const char *data, size_t len, bool bin) {
        String *str = nullptr;
        // Allocate new String
        uint16_t flags = 0;
        hash_t hash = 0;
        if (_isshort(data, len)) {
            flags |= IS_SHORT;
            hash = _gethash(data, len);
            str = strdict.get(data, len, hash);
        }
        if (!str) {
            str = allocate(len);
            str = new(str) String(flags, data, len, hash);
        }
        str->isbin(bin);
        return str;
    }

    Value String::TRef(const char *data, size_t len) {
        return new String(IS_REF, data, len);
    }

    Value String::TRef(const char *data) {
        return TRef(data, strlen(data));
    }

    Value String::CRef(const char *data, size_t len) {
        String *str = nullptr;
        uint16_t flags = IS_REF;
        hash_t hash = 0;
        if (_isshort(data, len)) {
            flags |= IS_SHORT;
            hash = _gethash(data, len);
            str = strdict.get(data, len, hash);
        }
        return str ? str : new String(flags, data, len, hash);
    }
    Value String::CRef(const char *data) {
        return String::CRef(data, strlen(data));
    }

    Value operator""_x(const char *data, size_t len) {
        return String::CRef(data, len);
    }

    using namespace std;

    Value String::New(istream& in, size_t n, bool bin) {
        if (n < XVAL_SHORTSTR_LEN) {
            char buf[XVAL_SHORTSTR_LEN];
            return in.read(buf, n).good() ?
                New(buf, n) : Value::Nil;
        }
        auto str = allocate(n);
        // Read string from stream failure
        if (!in.read(str->_data, n)) {
            delete str;
            return Value::Nil;
        }
        str->_data[n] = 0;
        str->_size = n;
        str->_ext = 0;
        str->isbin(bin);
        return new(str) String;
    }

    bool String::operator==(const Value &v) const {
        // assert v.isstr()
        auto& str = v.str();
        return &str == this ? true : (
            hash() == str.hash() &&
            size() == str.size() &&
            !memcmp(c_str(), str.c_str(), size()));
    }
}
