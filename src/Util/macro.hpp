#ifndef __MACRO_H__
#define __MACRO_H__


#include<string>
#include<assert.h>
#include"log.hpp"
#include"util.hpp"

namespace gaiya{


#define GAIYA_ASSERT(x) \
  if(!(x)){ \
    LOG_ERROR(LOG_ROOT()) << "ASSERTION: " << #x \
    << "\nbacktrace:\n" << gaiya::backTraceToString(100); \
    assert(x); \
  }


#define GAIYA_ASSERT2(x,m) \
  if(!(x)){ \
    LOG_ERROR(LOG_ROOT()) << "ASSERTION: " << #x \
    <<std::endl << m \
    << "\nbacktrace:\n" << gaiya::backTraceToString(100); \
    assert(x); \
  }

}
#endif
