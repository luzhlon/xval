#ifndef __XVAL_OBJ_H__
#define __XVAL_OBJ_H__

#include <iostream>

#include "xval.h"

namespace xval {
    class EXPORT Object {
    public:
        Object(type_t t) { _type = t; }
        virtual ~Object() {}

        type_t type() const { return _type; }

        Object& operator=(const Object& o) = delete;

        virtual operator bool() const { return this != nullptr; }
        virtual bool operator==(const Value& v) const;
        virtual hash_t hash() const { return (hash_t)(uint64_t)this; }

    protected:
        virtual bool destruct() { return false; }

        friend class Value;
        void incref() { ++_refs; }
        void decref() {
            if (!(--_refs))
                if (!destruct())
                    delete this;
        }

    private:

        uint32_t _refs = 0;
        type_t   _type;

    protected:
        uint16_t _ext;
    };
}

#endif /* __XVAL_OBJ_H__ */
