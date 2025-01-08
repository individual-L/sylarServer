#ifndef __TypeOrder_H__
#define __TypeOrder_H__

#include <type_traits>
#include <byteswap.h>
#include<stdint.h>

#define SYS_LITTLE_ENDIAN 1

#define SYS_BIG_ENDIAN 0

namespace gaiya{

//判断是否为小端系统
inline bool isLittleSys(){
  uint16_t n = 0x1;
  //[0]是访问指针的低地址位，小端是从低地址位开始
  if(((uint8_t*)&n)[0] == 1){
    return SYS_LITTLE_ENDIAN;
  }else{
    return SYS_BIG_ENDIAN;
  }
}

#define SYS_ENDIAN isLittleSys()

//64字节网络字节序转换
template<typename T>
typename std::enable_if_t<sizeof(T) == sizeof(uint64_t),T> byteSwap(T val){
  return bswap_64(val);
}

//32字节网络字节序转换
template<typename T>
typename std::enable_if_t<sizeof(T) == sizeof(uint32_t),T> byteSwap(T val){
  return bswap_32(val);
}

//16字节网络字节序转换
template<typename T>
typename std::enable_if_t<sizeof(T) == sizeof(uint16_t),T> byteSwap(T val){
  return bswap_16(val);
}
template<typename T>
T byteSwapHN(T val){
  if(isLittleSys()){
    return byteSwap(val);
  }
  return val;
}


}


#endif
