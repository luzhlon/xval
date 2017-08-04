#ifndef __XVAL_VAL_H__
#define __XVAL_VAL_H__

#include <cstring>
#include <iostream>

#include "xval.h"
#include "xval_obj.h"

namespace xval {
    using namespace std;

    class EXPORT Value {
    public:
        static const Value Nil;

    public:
        Value() { init(); }
    	Value(bool b) { _type = TV_BOOL; _int = static_cast<int64_t>(b); }
    	Value(int64_t i) { _type = TV_INT; _int = i; }
    	Value(uint64_t i) : Value((int64_t)i) {}
#ifndef _WIN64
        Value(size_t i) : Value((int64_t)i) {}
#endif // !_WIN64
    	Value(double n) { _type = TV_FLOAT; _float = n; }
    	Value(float n) : Value((double)n) {}
    	Value(const char *str) : Value(str, strlen(str)) {}
        Value(const char *str, size_t len);
        Value(const string& str) : Value(str.c_str(), str.size()) {}
    	Value(void *p) { _type = TV_POINTER; _pointer = p; }
        Value(Object& obj) : Value(&obj) {}
        Value(Object *obj) {
            _type = obj->type();
            _obj = obj;
            obj->incref();
        }
        // Copy and move constructor
        Value(const Value& v) { copy(v).incref(); }
        Value(Value&& v) { copy(v), v.init(); }
        ~Value() { decref(); }
        // reset this value
        inline Value& reset() {
            return decref().init();
        }
        // initialize this value
        inline Value& init() {
            _type = TV_NIL, _int = 0;
            return *this;
        }
    	// Get the value's hashvalue
        hash_t hash() const {
            return isobj() ? obj().hash() : static_cast<hash_t>(_int);
        }
    	// Type judge
    	inline type_t type() const  { return _type; }
    	inline bool isobj() const   { return type() >= TV_STRING; }
    	inline bool isnil() const   { return type() == TV_NIL; }
    	inline bool isbool() const  { return type() == TV_BOOL; }
        inline bool istrue() const  { return isbool() && _int; }
        inline bool isfalse() const { return isbool() && !_int; }
    	inline bool isstr() const   { return type() == TV_STRING; }
    	inline bool islist() const  { return type() == TV_LIST; }
    	inline bool istuple() const { return type() == TV_TUPLE; }
    	inline bool isdict() const  { return type() == TV_DICT; }
    	inline bool isint() const   { return type() == TV_INT; }
    	inline bool isfloat() const { return type() == TV_FLOAT; }
    	inline bool ispointer() const { return type() == TV_POINTER; }
    	// Concrete object reference
    	inline Object&  obj() const { return *_obj; }
    	inline List&    list() const { return *(List *)_obj; }
    	inline String&  str() const { return *(String *)_obj; }
    	inline Dict&    dict() const { return *(Dict *)_obj; }
    	inline Tuple&   tuple() const { return *(Tuple *)_obj; }
    	inline int64_t  Int() const { return _int; }
        inline int64_t  Int(int64_t opt) { return isint() ? _int : opt; }
    	inline double   Float() const { return _float; }
    	inline double   Float(double opt) const { return isfloat() ? _float : opt; }
    	inline bool     Bool() const { return static_cast<bool>(_int); }
    	inline bool     Bool(bool opt) const { return isbool() ? Bool() : opt; }
    	inline void    *pointer() const { return _pointer; }

        inline operator Object *() const { return &obj(); }
        inline operator Dict *() const { return &dict(); }
        inline operator List *() const { return &list(); }
        inline operator String *() const { return &str(); }

        operator const char *() const;
        operator bool() const {                      // Boolean evaluate
            return isobj() ? (bool)obj() : static_cast<bool>(_int);
        }
        inline operator uint64_t() const { return _int; }
        inline operator int64_t() const { return _int; }
        inline operator float() const { return _float; }
        inline operator double() const { return _float; }
    	// Operators
    	Value& operator=(const Value &v);
    	Value& operator=(Value &&v);
        bool operator==(const Value& v) const;
        // Get and Set: for List and Dict
        Value get(const Value& v);
        Value operator[](const Value& v) { return get(v); }
        Value& set(const Value& k, const Value& v);

    private:
        // increase the references
        inline Value& incref() {
            if (isobj()) obj().incref();
            return *this;
        }
        // decrease the references
        inline Value& decref() {
            if (isobj()) obj().decref();
            return *this;
        }
        // copy the data from v to this
        inline Value& copy(const Value& v) {
            _type = v._type, _int = v._int;
            return *this;
        }

    	union {
    		Object *_obj;
    		void   *_pointer;
    		int64_t _int;
    		double  _float;
    	};
    	type_t _type;

    protected:
    	int32_t _ext;
    };
}

#endif /* __XVAL_VAL_H__ */
