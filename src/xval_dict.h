#ifndef __XVAL_DICT_H__
#define __XVAL_DICT_H__

#include "xval_obj.h"
#include "xval_val.h"

namespace xval {
    class EXPORT Dict: public Object {
    public:
        static Value New(size_t size = 0);

        class Node : public Value {
        public:
            Node() { _next() = 0; }

            // Get key's reference
            inline Value& key() { return *this; }
            // Get value's reference
            inline Value& value() { return _val; }
            // Get next node
            inline Node *next() { return this + _next(); }
            // If this node is empty
            inline bool empty() { return key().isnil(); }
            // If this node is valid
            inline bool valid() {
                return this != nullptr && !empty();
            }

        private:
            friend struct Dict;

            inline void init(const Value& k = Value::Nil,
                const Value& v = Value::Nil) { set(k, v), _next() = 0; }
            // Set key and value
            inline void set(const Value& k, const Value& v) {
                key() = k, value() = v;
            }
            // Move this node to n
            void moveto(Node *n) {
                n->key() = std::move(key());
                n->value() = std::move(value());
                if (_next())     // next is valid
                    n->setnext(next()),
                    previous()->setnext(n),
                    _next() = 0;
            }
            // Remove self node, return next node
            inline Node *rmself() {
                if (!_next())
                    return nullptr;
                auto nx = next();
                key() = std::move(nx->key());
                value() = std::move(nx->value());
                setnext(nx->next());
                return nx;
            }
            // Remove next node and return it
            inline Node *rmnext() {
                auto nx = next();
                if (nx == this)
                    return nullptr;
                setnext(nx->next());
                return nx;
            }
            // Link node n into this linklist
            inline void linkin(Node *n) {
                n->setnext(next());
                setnext(n);
            }
            // Find k in this linklist
            Node *find(const Value& k) {
                auto p = this;
                do if (p->key() == k)
                    return p;
                while (p = p->next(), p != this);
                return nullptr;
            }
            // Get previous node
            Node *previous() {
                for (auto p = this; ; p = p->next())
                    if (p->next() == this) return p;
            }
            // Set next node
            inline void  setnext(Node *n) { _next() = n - this; }
            inline int32_t& _next() { return _ext; }

        private:
            Value _val;     // value
        };

        struct Iterator {
            Iterator(Node *P, Node *E) :p(P), e(E) { increase(); }

            void  operator++() { ++p; increase(); }
            bool  operator!=(Iterator& it) { return p != it.e; }
            Node& operator*() { return *p; }

            void increase() { while (p < e && p->empty()) p++; }
        private:
            Node *p;
            Node *e;
        };

        Iterator begin() const {
            Iterator it(_items, _items + _capacity);
            return it;
        }
        Iterator end() const {
            Iterator it(_items, _items + _capacity);
            return it;
        }

        size_t capacity() { return _capacity; }
        size_t size() const { return _size; }

        String *get(const char *str, size_t len, hash_t hash);
        Value get(const Value& k);
        void  set(const Value& k, const Value& v);
        bool  remove(const Value& k);
        void  clear();

        bool destruct() override { delete this; return true; }
        operator bool() const override { return size() > 0; }
        bool operator==(const Value& v) const override {
            return v.isdict() && this == &v.dict();
        }

    private:
        // Allocate array
        void alloc(size_t size) {
            _capacity = size;
            _items = _capacity ? new Node[_capacity]: nullptr;
            _idle = _capacity - 1;
            _size = 0;
        }
        // Increase size
        void incsize() { ++_size; }
        // Decrease size
        void decsize() { --_size; }
        // Expand the capacity
        void expand();
        // Transfer the nodes [begin, end) to new
        void transfer(Node *begin, Node *end);
        // Get a idle node, it's needed extend if return nullptr
        Node *popidle() {
            while (_idle >= 0
                && !_items[_idle].empty())
                --_idle;
            return _idle < 0 ? nullptr : &_items[_idle--];
        }
        // Add a node to idle
        void pushidle(Node *n) {
            auto off = n - _items;
            if (off > _idle)
                _idle = off;
            n->init();
        }
        // Get node by hash
        Node *getnode(hash_t hash) {
            return capacity() ? &_items[hash % capacity()] : nullptr;
        }
        // Get node by key
        Node *getnode(const Value& key) {
            auto node = getnode(key.hash());
            return node->valid() && really(node) ?
                node->find(key) : nullptr;
        }
        // Check a node if should be in it's position
        inline bool really(Node *n) {
            return getnode(n->key().hash()) == n;
        }

    private:
        Dict(size_t size = 0)
            : Object(TV_DICT) { alloc(size); }
        ~Dict() override { delete[] _items; }

        Node    *_items;
        int32_t _idle;
        uint32_t _size;
        uint32_t _capacity;
    };

    inline Value _DICT(Dict& d) { return &d; }

    template <typename T, typename T2, typename... Args>
    inline Value _DICT(Dict& d, T k, T2 v, Args... rest) {
        d.set(k, v);
        return _DICT(d, rest...);
    }

    template <typename... Args>
    Value DICT(Args... all) {
        return _DICT(Dict::New((sizeof...(all)) / 2).dict(), all...);
    }
}

#endif /* __XVAL_DICT_H__ */
