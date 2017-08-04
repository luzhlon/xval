#ifndef __XVAL_LIST_CPP__
#define __XVAL_LIST_CPP__

#include <stddef.h>
#include "xval_list.h"

namespace xval {

    Value List::New(size_t size) {
        return new List(size);
    }

    Value List::New(initializer_list<Value> l) {
        auto list = List::New(l.size());
        for (size_t i = 0; i < l.size(); i++)
            list.list().append(l.begin()[i]);
        return list;
    }

    Value List::pop_back() {
        if (size())
            return std::move(_array[_size--]);
        else
            return Value::Nil;
    }

    void List::resize(size_t n) {
        if (n < size())
            for (auto i = n; i < size(); i++)
                (&_array[i])->Value::~Value();
        else
            reserve(n);
        _size = n;
    }

    void List::expand(size_t min) {
        auto ncap = capacity() ? capacity() * 2 : 1;
        while (ncap < min) ncap *= 2;
        auto narr = new Value[ncap];
        for (int i = 0; i < size(); i++)
            narr[i] = std::move(_array[i]);
        // Origianl values were moved, so only delete the memory
        delete[] _array;
        _capacity = ncap;
        _array = narr;
    }

    void List::append(Value *v, size_t n) {
        reserve(size() + n);
        for (size_t i = 0; i < n; i++)
            _array[i + _size] = v[i];
        _size += n;
    }

    Tuple *Tuple::alloc(size_t n) {
        auto size = sizeof(Value) * n + offsetof(Tuple, _data);
        return (Tuple *)(new char[size]);
    }

    Value Tuple::New(size_t n) {
        auto t = alloc(n);
        return new(t) Tuple(n);
    }

    Value Tuple::New(const Value *p, size_t n) {
        auto t = alloc(n);
        return new(t) Tuple(p, n);
    }

    Value Tuple::New(initializer_list<Value> l) {
        return New(l.begin(), l.size());
    }

    void Tuple::Init(Value *l, const Value *s, size_t len) {
        for (auto p = l, e = l + len; p < e; p++)
            new(p) Value(*s++);
    }

    void Tuple::Init(Value *l, size_t len) {
        for (auto p = l, e = l + len; p < e; p++)
            new(p) Value();
    }

    void Tuple::UnRef(Value *l, size_t len) {
        for (auto p = l, e = l + len; p < e; p++)
            p->~Value();
    }
}

#endif /* __XVAL_LIST_CPP__ */
