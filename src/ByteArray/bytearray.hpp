#ifndef __BYTEARRAY_H__
#define __BYTEARRAY_H__

#include<stdint.h>
#include<string>
#include<memory>
#include<vector>
#include <sys/uio.h>

namespace gaiya{

class ByteArray
{

  public:
    typedef std::shared_ptr<ByteArray> ptr;
    ByteArray(size_t len = 4096);
    ~ByteArray();

    struct Node{
      Node();
      Node(size_t len);
      ~Node();
      char *ptr;
      Node *next;
      size_t size;
    };
  public:
    void writeFint8(const int8_t val);
    void writeFint16(int16_t val);
    void writeFint32(int32_t val);
    void writeFint64(int64_t val);

    void writeFuint8(const uint8_t val);
    void writeFuint16( uint16_t val);
    void writeFuint32( uint32_t val);
    void writeFuint64( uint64_t val);

    void writeInt32( int32_t val);
    void writeInt64( int64_t val);

    void writeUint32( uint32_t val);
    void writeUint64( uint64_t val);

    uint32_t readUint32();
    uint64_t readUint64();
    int32_t readInt32();
    int64_t readInt64();

    void writeFloat( float val);

    void writeDouble( double val);


    int8_t readFint8();
    int16_t readFint16();
    int32_t readFint32();
    int64_t readFint64();

    uint8_t readFuint8();
    uint16_t readFuint16();
    uint32_t readFuint32();
    uint64_t readFuint64();

    float readFloat();
    double readDouble();

    void writeStringF16(const std::string& val);
    void writeStringF32(const std::string& val);
    void writeStringF64(const std::string& val);
    void writeStringVint(const std::string& val);
    void writeStringWithnotLength(const std::string& val);
    std::string readStringWithnotLength();

    std::string readStringF16();
    std::string readStringF32();
    std::string readStringF64();
    std::string readStringVint();

    void clear();

    //像内存块写入buf
    void write(const void *buf, size_t size);

    //从m_position处读取size大小个字节
    void read(void *buf, size_t size);

    void read(void* buf, size_t size, size_t position) const;

    size_t getPosition()const {return m_position;};
    
    void setPosition(size_t len);

    size_t getSize() const {return m_size;};

    //当前内存块可读取的大小
    size_t getReadSize()const {return m_size - m_position;};

    bool readFromFile(const std::string& fileName);

    bool writeToFile(const std::string& fileName) const ;

    size_t getBaseSize()const {return m_baseSize;};

    bool isLittleEndian()const{return m_isLittleSys;};

    void setisLittleEndian(bool val);

    std::string toString()const;

    std::string toHexString()const;

    //获取读缓冲区的内容
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const;
    //从指定位置获取读缓冲区的内容
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;
    //获取写缓冲区的内容
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

  private:
    //当前内存块的可操作的容量
    size_t getCapacity() const { return m_capacity - m_position;}

    void addCapacity(size_t size);
  private:
    size_t m_baseSize;
    size_t m_position;
    size_t m_capacity;
    size_t m_size;
    bool m_isLittleSys;
    Node* m_root;
    Node* m_cur;

};



}

#endif
