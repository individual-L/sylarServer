#include"bytearray.hpp"
#include"endian.hpp"
#include"log.hpp"
#include<unistd.h>
#include<string.h>
#include<iomanip>
#include <fstream>
#include<macro.hpp>
#include<sstream>
#include<math.h>

static gaiya::Logger::ptr logger = LOG_M()->getLogger("master");

namespace gaiya{


ByteArray::ByteArray(size_t len)
:m_baseSize(len)
,m_position(0)
,m_capacity(len)
,m_size(0)
,m_isLittleSys(SYS_ENDIAN)
,m_root(new Node(len))
,m_cur(m_root){
}

ByteArray::~ByteArray()
{
  Node* tmp = m_root;
  while(tmp){
    m_cur = tmp;
    tmp = tmp->next;
    delete m_cur;
  }
}

ByteArray::Node::Node()
:ptr(nullptr)
,next(nullptr)
,size(0){

}
ByteArray::Node::Node(size_t len)
:ptr(new char[len])
,next(nullptr)
,size(len){

}
ByteArray::Node::~Node(){
  if(ptr){
    delete [] ptr;
  }
}
//数字压缩算法zigzag
static uint32_t EncodeZigzag32(const int32_t& v) {
  lOG_INFO_ROOT()<<v <<" encode==> " <<((uint32_t)(v << 1) ^ (v >> (sizeof(v) * 8 - 1)));
  return (v << 1) ^ (v >> 31);
}

// template<typename T>
// static uint32_t EncodeZigzagInt(const T& v) {
//   lOG_INFO_ROOT()<<v <<" encode==> " <<((v << 1) ^ (v >> (sizeof(v) * 8 - 1)));
//   return (v << 1) ^ (v >> (sizeof(v) * 8 - 1));
// }

static int32_t DecodeZigzag32(const uint32_t& v) {
  lOG_INFO_ROOT()<<v <<" decode==> " <<((v >> 1) ^ (-(v & 1)));
  return (v >> 1) ^ (-(v & 1));
}

static uint64_t EncodeZigzag64(const int64_t& v) {
  // lOG_INFO_ROOT()<<v <<" encode==> " <<((uint64_t)(v << 1) ^ (v >> (sizeof(v) * 8 - 1)));
  return (v << 1) ^ (v >> (sizeof(v) * 8 - 1));
}

static int64_t DecodeZigzag64(const uint64_t& v) {
  // lOG_INFO_ROOT()<<v <<" decode==> " <<((v >> 1) ^ (-(v & 1)));
  return (v >> 1) ^ (-(v & 1));
}


void ByteArray::write(const void *buf, size_t size){
  if(size == 0){
    return;
  }

  //扩容到能写入size大小
  addCapacity(size);
  // lOG_INFO_ROOT() <<"capacity: " <<m_capacity;

  //内存块起始地址
  size_t npos = m_position % m_baseSize;
  //当前内存块可操作的容量
  size_t ncap = m_cur->size - npos;
  //buf的数据操作起始位置
  size_t bpos = 0;

  while(size > 0){
    if(ncap >= size){
    //当前内存块大小足够写入
      memcpy(m_cur->ptr + npos,(const char*)buf + bpos,size);
      if(m_cur->size == size + npos){
        m_cur = m_cur->next;
      }
      m_position += size;
      bpos += size;
      size = 0;
    }else{
      memcpy(m_cur->ptr + npos,(const char*)buf + bpos,ncap);
      m_position += ncap;
      size -= ncap;
      bpos += ncap;
      m_cur = m_cur->next;
      ncap = m_cur->size;
      npos = 0;
    }
  }
  if(m_position > m_size){
    m_size = m_position;
  }
}

void ByteArray::read(void *buf, size_t size){
  if(size > getReadSize()){
    LOG_ERROR(logger) <<"size: "<<size <<" m_size: "<<m_size;
    throw std::out_of_range("not enough length");
  }

  size_t npos = m_position % m_baseSize;
  size_t ncap = m_cur->size - npos;
  size_t bpos = 0;

  while(size > 0){
    if(ncap >= size){
      memcpy((char*)buf + bpos,m_cur->ptr + npos,size);
      if((size + npos) == m_cur->size){
        m_cur = m_cur->next;
      }
      m_position += size;
      bpos += size;
      size = 0;
    }else{
      memcpy((char*)buf + bpos,m_cur->ptr + npos,ncap);
      bpos += ncap;
      npos = 0;
      m_position += ncap;
      size -= ncap;
      m_cur = m_cur->next;
      ncap = m_cur->size;
    }
  }
}

void ByteArray::read(void* buf, size_t size, size_t position) const{
  if(size > m_size - position){
    throw std::out_of_range("not enough length");
  }

  size_t npos = position % m_baseSize;
  size_t ncap = m_cur->size - npos;
  size_t bpos = 0;

  Node* node = m_cur;
  while(size > 0){
    if(ncap >= size){
      memcpy((char*)buf + bpos,node + npos,size);
      if((size + npos) == node->size){
        node = node->next;
      }
      position += size;
      bpos += size;
      size = 0;
    }else{
      memcpy((char*)buf + bpos,node + npos,ncap);
      node = node->next;
      ncap = node->size;
      bpos += ncap;
      npos = 0;
      position += ncap;
      size -= ncap;
    }
  }
}


void ByteArray::writeFint8(const int8_t val){
  write(&val,sizeof(val));
}

void ByteArray::writeFint16(int16_t val){
  if(m_isLittleSys != SYS_ENDIAN){
    val = byteSwap(val);
  }
  write(&val,sizeof(val));
}
void ByteArray::writeFint32(int32_t val){
  if(m_isLittleSys != SYS_ENDIAN){
    val = byteSwap(val);
  }
  write(&val,sizeof(val));
}
void ByteArray::writeFint64(int64_t val){
  if(m_isLittleSys != SYS_ENDIAN){
    val = byteSwap(val);
  }
  write(&val,sizeof(val));
}

void ByteArray::writeFuint8(const uint8_t val){
  write(&val,sizeof(val));
}
void ByteArray::writeFuint16( uint16_t val){
  if(m_isLittleSys != SYS_ENDIAN){
    val = byteSwap(val);
  }
  write(&val,sizeof(val));
}
void ByteArray::writeFuint32( uint32_t val){
  if(m_isLittleSys != SYS_ENDIAN){
    val = byteSwap(val);
  }
  write(&val,sizeof(val));
}
void ByteArray::writeFuint64( uint64_t val){
  if(m_isLittleSys != SYS_ENDIAN){
    val = byteSwap(val);
  }
  write(&val,sizeof(val));

}

void ByteArray::writeInt32( int32_t val){
  writeUint32(EncodeZigzag32(val));
}
void ByteArray::writeInt64( int64_t val){
  writeUint64(EncodeZigzag64(val));
}

void ByteArray::writeUint32( uint32_t val){
  /*
  0111 1111 = 127
  */
  uint8_t tmp[5];
  uint8_t i = 0;
  while(val > 127){
    tmp[i++] = (val & 0x7f) | 0x80;
    val >>= 7;
  }
  tmp[i++] = val;
  write(tmp,i);
}
void ByteArray::writeUint64( uint64_t val){
  /*
  0111 1111 = 127
  */
  uint8_t tmp[10];
  uint8_t i = 0;
  while(val > 127){
    tmp[i++] = (val & 0x7f) | 0x80;
    val >>= 7;
  }
  tmp[i++] = val;
  write(tmp,i);
}
void ByteArray::writeFloat( float val){
  uint32_t tmp;
  memcpy(&tmp,&val,sizeof(val));
  writeFuint32(tmp);
}

void ByteArray::writeDouble(double val){
  uint64_t tmp;
  memcpy(&tmp,&val,sizeof(val));
  writeFuint64(tmp);
}

#define XX(xx) \
  xx res; \
  read(&res,sizeof(res)); \
  if(m_isLittleSys != SYS_ENDIAN){ \
    return byteSwap(res); \
  } \
  return res; 

int8_t ByteArray::readFint8(){
  int8_t res;
  read(&res,sizeof(res));
  return res;
}
int16_t ByteArray::readFint16(){
  XX(int16_t)
}
int32_t ByteArray::readFint32(){
  XX(int32_t)
}
int64_t ByteArray::readFint64(){
  XX(int64_t)
}

uint8_t ByteArray::readFuint8(){
  uint8_t res;
  read(&res,sizeof(res));
  return res;
}
uint16_t ByteArray::readFuint16(){
  XX(uint16_t)
}
uint32_t ByteArray::readFuint32(){
  XX(uint32_t)
}
uint64_t ByteArray::readFuint64(){
  XX(uint64_t)
}

float ByteArray::readFloat(){
  float res;
  uint32_t v = readFuint32();
  memcpy(&res,&v,sizeof(v));
  return res;
}
double ByteArray::readDouble(){
  double res;
  uint64_t v = readFuint64();
  memcpy(&res,&v,sizeof(v));
  return res;
}

uint32_t ByteArray::readUint32(){
  uint32_t res = 0;
  for(int i = 0;i < 32;i += 7){
    uint8_t tmp = readFuint8();
    //128 = dec:1000 0000
    if(tmp < 128){
      res |= ((uint32_t)tmp) << i;
      break;
    }else{
      res |= ((uint32_t(tmp & 0x7f)) << i);
    }
  }
  return res;

}
uint64_t ByteArray::readUint64(){
  uint64_t res = 0;
  for(int i = 0;i < 64;i += 7){
    uint8_t tmp = readFuint8();
    //128 = dec:1000 0000
    if(tmp < 128){
      res |= (((uint64_t)tmp) << i);
      break;
    }else{
      res |= ((uint64_t(tmp & 0x7f)) << i);
    }
  }
  return res;
}
int32_t ByteArray::readInt32(){
  return DecodeZigzag32(readUint32());
}
int64_t ByteArray::readInt64(){
  return DecodeZigzag64(readUint64());
}

void ByteArray::writeStringF16(const std::string& val){
  writeFuint16(val.size());
  write(val.c_str(),val.size());
}
void ByteArray::writeStringF32(const std::string& val){
  writeFuint32(val.size());
  write(val.c_str(),val.size());
}
void ByteArray::writeStringF64(const std::string& val){
  writeFuint64(val.size());
  write(val.c_str(),val.size());
}
void ByteArray::writeStringVint(const std::string& val){
  writeUint64(val.size());
  write(val.c_str(),val.size());
}
void ByteArray::writeStringWithnotLength(const std::string& val){
  write(val.c_str(),val.size());
}

std::string ByteArray::readStringF16(){
  uint16_t len = readFuint16();
  std::string buff;
  buff.resize(len);
  read(buff.data(),len);
  return buff;
}
std::string ByteArray::readStringF32(){
  uint32_t len = readFuint32();
  std::string buff;
  buff.resize(len);
  read(buff.data(),len);
  return buff;
}
std::string ByteArray::readStringF64(){
  uint64_t len = readFuint32();
  std::string buff;
  buff.resize(len);
  read(buff.data(),len);
  return buff;
}
std::string ByteArray::readStringVint(){
  uint64_t len = readUint64();
  std::string buff;
  buff.resize(len);
  read(buff.data(),len);
  return buff;
}
void ByteArray::clear(){
  m_position = 0;
  m_size = 0;
  m_capacity = m_baseSize;
  Node* tmp = m_cur->next;
  while(tmp){
    m_cur = tmp;
    tmp = tmp->next;
    delete []m_cur;
  }
  m_cur = m_root;
  m_root->next = nullptr;
  memset(m_root->ptr,0,m_root->size);

}
void ByteArray::setPosition(size_t len){
  if(len > m_capacity){
    throw std::out_of_range("setPosition out of range");
  }
  m_position = len;

  if(m_position > m_size){
    m_size = m_position;
  }
  //从头开始计算，大于当前内存块的容量，则切换内存块
  m_cur = m_root;
  while(len > m_cur->size){
    m_cur = m_cur->next;
    len -= m_cur->size;
  }
  if(len == m_cur->size){
    m_cur = m_cur->next;
  }

}

void ByteArray::addCapacity(size_t size){
  if(size == 0){
    return;
  }
  size_t oldCap = getCapacity();
  if(oldCap >= size){
    return;
  }
  size_t remained_size = size - oldCap;
  size_t count = std::ceil(1.0 * remained_size / m_baseSize);

  Node* rail = m_root;
  while(rail->next != nullptr){
    rail = rail->next;
  }
  Node* cur = nullptr;
  while(count){
    Node* tmp = new Node(m_baseSize);
    if(cur == nullptr){
      cur = tmp;
    }
    rail->next = tmp;
    rail = rail->next;
    --count;
    m_capacity += m_baseSize;
  }

  //当最后一个内存块无内存时，将m_cur设为新建的内存块
  if(oldCap == 0){
    m_cur = cur;
  }
}

void ByteArray::setisLittleEndian(bool val){
  if(val){
    m_isLittleSys = SYS_LITTLE_ENDIAN;
  }else{
    m_isLittleSys = SYS_BIG_ENDIAN;
  }
}

std::string ByteArray::toString()const{
  std::string str;
  str.resize(getReadSize());
  if(str.empty()){
    return str;
  }
  read(str.data(),str.size(),m_position);
  return str;
}
std::string ByteArray::toHexString()const{
  std::string buff = toString();
  std::stringstream ss;

  for(size_t i = 0;i < buff.size();++i){
    if(i > 0 && i % 32 == 0){
      ss << std::endl;
    }
    ss << std::setw(2) << std::setfill('0') <<std::hex << int ((uint8_t)buff[i]) <<" ";
  }
  return ss.str();
};




uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const{
  if(getReadSize() < len){
    len = getReadSize();
  }
  if(len == 0){
    return 0;
  }

  size_t size = len;

  size_t pos = m_position % m_baseSize;
  size_t cap = m_cur->size - pos;
  Node* node = m_cur;

  struct iovec iov;

  while(len > 0){
    if(cap >= len){
      iov.iov_base = node->ptr + pos;
      iov.iov_len = len;
      len = 0;
    }else{
      iov.iov_base = node->ptr + pos;
      iov.iov_len = cap;
      len -= cap;
      node = node->next;
      cap = node->size;
      pos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}
uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const{
  if(m_size - position < len){
    len = m_size - position;
  }
  if(len == 0){
    return 0;
  }

  size_t size = len;

  size_t pos = position % m_baseSize;
  size_t count = position / m_baseSize;

  //根据下标找到内存块
  Node* node = m_root;
  while(count--){
    node = node->next;
  }
  
  size_t cap = node->size - pos;

  struct iovec iov;

  while(len > 0){
    if(cap >= len){
      iov.iov_base = node->ptr + pos;
      iov.iov_len = len;
      len = 0;
    }else{
      iov.iov_base = node->ptr + pos;
      iov.iov_len = cap;
      len -= cap;
      node = node->next;
      cap = node->size;
      pos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}
uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len){
  if(len == 0){
    return 0;
  }
  addCapacity(len);
  size_t size = len;

  size_t pos = m_position % m_baseSize;
  size_t cap = m_cur->size - pos;
  struct Node* node = m_cur;
  struct iovec iov;

  while(len > 0){
    if(cap >= len){
      iov.iov_base = node->ptr + pos;
      iov.iov_len = len;
      len = 0;
    }else{
      iov.iov_base = node->ptr + pos;
      iov.iov_len = cap;
      len -= cap;
      node = node->next;
      cap = node->size;
      pos = 0;
    }
    buffers.push_back(iov);
  }
  return size; 
}

bool ByteArray::writeToFile(const std::string& fileName) const{
  std::fstream fo;
  fo.open(fileName,std::ios::trunc | std::ios::binary);

  if(!fo){
    LOG_ERROR(logger) <<"ByteArray::writeToFile::fo.open() error, fileName: " <<fileName
                      <<" error: " <<errno <<"(" <<strerror(errno) <<")";
    return false;
  }
  size_t readSize = getReadSize();
  size_t pos = m_position % m_baseSize;
  size_t cap = 0;
  Node* tmp = m_cur;

  while(readSize > 0){
    cap = (readSize > m_cur->size ? m_cur->size : readSize) - pos;
    fo.write(m_cur->ptr + pos,cap);
    readSize -= cap;
    tmp = tmp->next;
    pos = 0;
  }
  return true;
}

bool ByteArray::readFromFile(const std::string& fileName){
  std::fstream fi;
  fi.open(fileName,std::ios::binary);
  if(!fi){
    LOG_ERROR(logger) <<"ByteArray::readFromFile::fi.open() error, fileName: " <<fileName
                      <<" error: " <<errno <<"(" <<strerror(errno) <<")";
    return false;   
  }

  std::shared_ptr<char> ptr = std::shared_ptr<char>(new char[m_baseSize],[](char* ptr){delete [] ptr;});
  while(!fi.eof()){
    fi.read(ptr.get(),m_baseSize);
    write(ptr.get(),fi.gcount());
  }

  return true;
}

}
