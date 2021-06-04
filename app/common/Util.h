#ifndef _UTIL_H_
#define _UTIL_H_

#include<utility>
#include<type_traits>
#include "Common.h"

#define HAS_MEMBER(Func) \
    template<typename T> \
    struct has_member_##Func \
    {\
    private:\
        template<typename U>\
            static auto Check(int) -> decltype( std::declval<U>().Func(), std::true_type() );\
        template<typename U>\
            static std::false_type Check(...);\
    public:\
        enum { value = std::is_same<decltype(Check<T>(0)),std::true_type>::value  };\
    };\


HAS_MEMBER(Init)
HAS_MEMBER(OnInit)

#endif