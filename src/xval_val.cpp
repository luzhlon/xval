
#include "xval_val.h"
#include "xval_list.h"
#include "xval_dict.h"

namespace xval {

    bool Value::operator==(const Value& v) const {
        return type() != v.type() ? false :
                isobj() ? obj() == v : v._int == _int;
    }

    Value Value::get(const Value& v) {
        if (islist() && v.isint())
            return list().get(v.Int());
        else if (istuple() && v.isint())
            return tuple().get(v.Int());
        else if (isdict())
            return dict().get(v);
        else
            return Value::Nil;
    }

    Value& Value::set(const Value& k, const Value& v) {
        if (islist() && v.isint())
            list().set(v.Int(), v);
        if (istuple() && v.isint())
            tuple().set(v.Int(), v);
        else if (isdict())
            dict().set(k, v);
        return *this;
    }

    Value& Value::operator=(const Value &v) {
        return copy(v).incref();
    }

    Value& Value::operator=(Value &&v) {
        decref().copy(v);
        v.init();
        return *this;
    }

}
