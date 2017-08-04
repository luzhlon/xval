#ifndef __XVAL_H__
#define __XVAL_H__

#include <stdint.h>

#ifdef _MSC_VER
    #ifdef DLL_EXPORT
        #define EXPORT __declspec(dllexport)
    #else
        #define EXPORT __declspec(dllimport)
    #endif
#else
    #define EXPORT
#endif

namespace xval {

    class Object;
    class String;
    class List;
    class Tuple;
    class Dict;
    class Value;

    enum {
    	TV_NIL = 0,
    	TV_BOOL,
    	TV_INT,
    	TV_FLOAT,
    	TV_POINTER,
    	// These below is GC-Object
    	TV_STRING,
    	TV_TUPLE,
    	TV_LIST,
        TV_DICT
    };

    typedef uint32_t hash_t;
    typedef uint16_t type_t;
}

#endif /* __XVAL_H__ */
