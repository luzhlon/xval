#ifndef __XVAL_LIST_H__
#define __XVAL_LIST_H__

#include <initializer_list>
#include "xval_obj.h"
#include "xval_val.h"

namespace xval {
    using namespace std;

    class EXPORT List: public Object {
    public:
        static Value New(size_t size = 0);
        static Value New(initializer_list<Value> l);

        void append(const Value& val) {
            set(size(), val);
        }
        void append(Value *v, size_t n);
        void push_back(const Value& val) { append(val); }
        Value pop_back();

        inline size_t size() const { return _size; }
        inline bool empty() const { return size() == 0; }
        size_t capacity() const { return _capacity; }

        void resize(size_t n);
        void reserve(size_t n) { if (capacity() < n) expand(n); }

        inline Value get(size_t i) const {
            return i < size() ?  _array[i] : Value::Nil;
        }
        inline Value operator[](size_t i) const {
            return get(i);
        }
        void set(size_t i, const Value& v) {
            if (i >= size())
                resize(i + 1);
            _array[i] = v;
        }

        List& operator+=(const Value& v) {
            if (v.islist())
                append(v.list()._array, v.list().size());
            else
                append(v);
            return *this;
        }

        inline Value *begin() const { return _array; }
        inline Value *end() const { return _array + size(); }

        operator bool() const override { return size() > 0; }

    private:
        List(size_t cap)
            : Object(TV_LIST) { reserve(cap); }
        ~List() override { delete[] _array; }
        // expand the capacity
        void expand(size_t min);

        uint32_t _size = 0;
        uint32_t _capacity = 0;
        Value   *_array = nullptr;
    };

    class EXPORT Tuple : public Object {
    public:
        static Value New(size_t n);
        static Value New(const Value *p, size_t n);
        static Value New(initializer_list<Value> l);

        static void Init(Value *l, size_t len);
        static void Init(Value *l, const Value *s, size_t len);
        static void UnRef(Value *l, size_t len);

        inline size_t size() const { return _size; }
        inline bool empty() const { return size() == 0; }

        inline Value get(size_t i) const {
            return i < size() ? _data[i] : Value::Nil;
        }
        inline Value operator[](size_t i) const { return get(i); }
        inline void set(size_t i, const Value& v) {
            if (i < size()) _data[i] = v;
        }

        inline Value *begin() { return _data; }
        inline Value *end() { return _data + size(); }

        operator bool() const override { return size() > 0; }

    private:
        Tuple(size_t n)
            : Object(TV_TUPLE), _size(n) { Init(_data, size()); }
        Tuple(const Value *p, size_t n)
            : Object(TV_TUPLE), _size(n) { Init(_data, p, size()); }
        ~Tuple() override { UnRef(_data, size()); }

        static Tuple *alloc(size_t n);

        size_t _size;
        Value _data[0];
    };
}

#endif /* __XVAL_LIST_H__ */
