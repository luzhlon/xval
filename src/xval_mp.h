#ifndef __XVAL_MP_H__
#define __XVAL_MP_H__

#include <iostream>
#include <string>
#include "xval_val.h"

namespace xval {
    using namespace std;
    // pack string
    bool EXPORT mp_pack(const char *str, size_t len, bool bin, ostream& o);
    bool EXPORT mp_pack(const char *str, ostream& o);
    // pack boolean
    bool EXPORT mp_pack(bool b, ostream& o);
    // pack array
    bool EXPORT mp_pack(const Value *p, size_t len, ostream& o);

    // pack any type
    bool EXPORT mp_pack(const Value& v, ostream& o);
    bool EXPORT mp_pack(const Value& v, string& s);
    // unpack any type
    bool EXPORT mp_unpack(Value &v, istream& i);
    bool EXPORT mp_unpack(Value &v, const string& s);
}

#endif /* __XVAL_MP_H__ */
