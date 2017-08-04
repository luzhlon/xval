#ifndef __XVAL_STR_H__
#define __XVAL_STR_H__

#include <cstring>
#include "xval.h"
#include "xval_obj.h"

namespace xval {
    hash_t _gethash(const char *data, size_t len);
    bool _isshort(const char *data, size_t len);

    class EXPORT String: public Object {
    public:
        enum {
            IS_SHORT = 1,
            IS_BINARY = 2,
            IS_REF = 4
        };

        static Value New(const char *data, size_t len, bool bin = false);
        static Value New(std::istream& in, size_t len, bool bin = false);
        // Reference a temporary pointer
        static Value TRef(const char *data, size_t len);
        static Value TRef(const char *data);
        // Reference a pointer from C string
        static Value CRef(const char *data, size_t len);
        static Value CRef(const char *data);

        static void    _addinternal(String *str);
        static String *_getinternal(const char *data, size_t len);

        size_t size() const { return _size; }
        bool equals(const char *str, size_t len) {
            return len == size() ?
                !memcmp(c_str(), str, len) : false;
        }
        operator char*() { return (char*)c_str(); }
        operator const char*() { return c_str(); }

        bool operator==(const Value &v) const override;
        operator bool() const override
            { return size() > 0; }
        hash_t hash() const override {
            return _hash ? _hash :
                (_hash = _gethash(c_str(), size()));
        }

        void isbin(bool b)
            { if (b) _ext |= IS_BINARY; else _ext &= ~IS_BINARY; }
        bool isbin() const { return _ext & IS_BINARY; }
        bool isshort() const { return _ext & IS_SHORT; }
        bool isref() const { return _ext & IS_REF; }
        const char *c_str() const { return isref() ? _ref : _data; }

    protected:
        String() : Object(TV_STRING) {}
        String(uint16_t flags, const char *data, size_t len, hash_t hash = 0)
            : String() {
            _ext = flags;
            _size = len;
            _hash = hash;
            if (isref())
                _ref = data;
            else
                copy(data, len);
            if (isshort())
                _addinternal(this);
        }
        static String* allocate(size_t size);

        inline void isshort(bool b)
            { if (b) _ext |= IS_SHORT; else _ext &= ~IS_SHORT; }
        inline void isref(bool b)
            { if (b) _ext |= IS_REF; else _ext &= ~IS_REF; }
        inline void copy(const char *data, size_t n) {
            memcpy(_data, data, n); _data[n] = 0;
        }

        mutable uint32_t _size;
        mutable hash_t   _hash;
        union {
            const char *_ref;
            char  _data[1];
        };
    };

    Value EXPORT operator""_x(const char *data, size_t len);
}

#endif /* __XVAL_STR_H__ */
