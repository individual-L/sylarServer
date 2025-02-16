#include"stream.hpp"


namespace gaiya{

  //固定读取length大小的数据，不读到不返回
int Stream::readFixSize(void* buffer, size_t length){
  size_t npos = 0;
  size_t cap = length;
  int64_t rt = 0;
  while(cap > 0){
    rt = read((char*)buffer + npos,cap);
    if(rt <= 0){
      return npos + rt;
    }
    cap -= rt;
    npos += rt;
  }
  return length;
}

int Stream::readFixSize(ByteArray::ptr ba, size_t length){
  size_t cap = length;
  int64_t rt = 0;
  while(cap > 0){
    rt = read(ba,cap);
    if(rt <= 0){
      return rt;
    }
    cap -= rt;
  }
  return length;
}

int Stream::writeFixSize(const void* buffer, size_t length){
  size_t npos = 0;
  size_t cap = length;
  int64_t rt = 0;
  while(cap > 0){
    rt = write((const char*)buffer + npos,cap);
    if(rt <= 0){
      return npos + rt;
    }
    cap -= rt;
    npos += rt;
  }
  return length;
}

int Stream::writeFixSize(ByteArray::ptr ba, size_t length){
  size_t cap = length;
  int64_t rt = 0;
  while(cap > 0){
    rt = write(ba,cap);
    if(rt <= 0){
      return rt;
    }
    cap -= rt;
  }
  return length;
}
}
