#include "xval_val.h"
#include "xval_obj.h"

namespace xval {
    const Value Value::Nil;

    bool Object::operator==(const Value& v) const {
        return this == &v.obj();
    }
}
