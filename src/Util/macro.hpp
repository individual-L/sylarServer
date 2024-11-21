#ifndef __MACRO_H__
#define __MACRO_H__


#include<string>
#include<assert.h>
#include"log.hpp"
#include"util.hpp"

//代码是否由GCC过着Clang编译，因为她两提供__builtin_except(!!(x),1)函数
#if defined __GNUC__ || __llvm__
//告诉编译器!!(x)这个表达式大概率可能等于常量1
# define GAIYA_LIKELY(x) __builtin_expect(!!(x),1)
//告诉编译器!!(x)这个表达式大概率可能等于常量1
# define GAIYA_UNLIKELY(x) __builtin_expect(!!(x),0)

#else

# define GAIYA_LIKELY(x) (x)
# define GAIYA_UNLIKELY(x) (x)

#endif


#define GAIYA_ASSERT(x) \
  if(GAIYA_UNLIKELY(!(x))){ \
    LOG_ERROR(LOG_ROOT()) << "ASSERTION: " << #x \
    << "\nbacktrace:\n" << gaiya::backTraceToString(100); \
    assert(x); \
  }


#define GAIYA_ASSERT2(x,m) \
  if(GAIYA_UNLIKELY(!(x))){ \
    LOG_ERROR(LOG_ROOT()) << "ASSERTION: " << #x \
    <<std::endl << m \
    << "\nbacktrace:\n" << gaiya::backTraceToString(100); \
    assert(x); \
  }

#endif
