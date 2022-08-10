#ifndef _JKYI_BYTEARRAY_H_
#define _JKYI_BYTEARRAY_H_


#include<memory>
#include<string>
#include<stdint.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<vector>

namespace JKYi{

//定义一个序列化的类,拥有序列化和反序列化的功能，该类会作为来网络编程的时候数据的载体
class ByteArray{
public:
    typedef std::shared_ptr<ByteArray> ptr;

    //在类中定义链表节点类型，ByteArray底层使用链表来存储数据
    struct Node{
        Node(size_t s);

        Node();

        ~Node();

        char * ptr;
        size_t size;
        Node* next;
    };

    ByteArray(size_t base_size=4096);

    ~ByteArray();

    //下面是提供给用户写入数据的接口
   
    //首先是写入固定长度的接口
    //写入一个字节的数据
    void writeFint8(int8_t val);
    void writeFuint8(uint8_t val);
    void writeFint16(int16_t val);
    void writeFuint16(uint16_t val);
    void writeFint32(int32_t val);
    void writeFuint32(uint32_t val);
    void writeFint64(int64_t val);
    void writeFuint64(uint64_t val);

    //可变长度的写入，写入的数据会被压缩
    void writeInt32(int32_t val);
    void writeUint32(uint32_t val);
    void writeInt64(int64_t val);
    void writeUint64(uint64_t val);

    //固定长度的写入float类型的数据
    void writeFloat(float val);
    void writeDouble(double val);

    //写入string类型的数据，对于字符串，再写入之前会先将它的长度进行写入,所以下面函数分别表示使用不同长度的值来存储写入的字符串的长度
    void writeStringF16(const std::string& val);
    void writeStringF32(const std::string& val);
    void writeStringF64(const std::string& val);
    //默认使用压缩过的64位的无符号整形来记录字符串的长度
    void writeStringVint(const std::string& val);
    //不需要写入字符串的长度
    void writeStringWithoutLength(const std::string& val);
    
    //下面是提供给用户的从ByteArray中读取数据的接口
    //
    int8_t   readFint8();
    uint8_t  readFuint8();
    int16_t  readFint16();
    uint16_t readFuint16();
    int32_t  readFint32();
    uint32_t readFuint32();
    int64_t  readFint64();
    uint64_t readFuint64();
    //读入可变长度的数据,也就是压缩过的数据
    int32_t  readInt32();
    uint32_t readUint32();
    int64_t  readInt64();
    uint64_t readUint64();
    
    float   readFloat();
    double  readDouble();
    
    std::string readStringF16();
    std::string readStringF32();
    std::string readStringF64();
    std::string readStringVint();

    //清空底层的数据
    void clear();

    //真正写入数据的函数 
    void write(const void * buf,size_t len);

    //真正从链表中读出数据的函数
    void read(void * buf,size_t len);

    //从指定的位置开始读数据
    void read(void * buf,size_t len,size_t position)const;

    size_t getPosition()const { return m_position; }
    void setPosition(size_t v);

    //将ByteArray里面的数据写入到文件里面去
    bool writeToFile(const std::string& name)const;

    //从文件中读出内容，然后写入到ByteArray里面去
    bool readFromFile(const std::string& name);

    size_t getBaseSize()const { return m_baseSize; };

    //返回当前还有多少数据可以读
    size_t getReadSize()const { return m_size - m_position; }

    //判断当前是否是小端序
    bool isLittleEndian()const;
    //是否设置为小端序
    void setIsLittleEndian(bool val);

    //下面两个函数都是将ByteArray中的[m_position,m_size)间的数据以字符串的形式返回
    
    std::string toString()const;
    //将ByteArray中的数据以十六进制字符串的返回，没有副作用
    std::string toHexString()const;

    //下面定义的函数是为了和socket配合使用
    //操作的数据是从m_position开始的
    uint64_t getReadBuffers(std::vector<iovec>&buffers,uint64_t len= ~011 )const;
    //从指定位置开始读,并且没有副作用
    uint64_t getReadBuffers(std::vector<iovec>&buffers,uint64_t len,uint64_t position)const;
    
    //将ByteArray里面可以写入的节点以iovec的形式返回，方便socket进行操作.len表示想要获取的空间的大小
    uint64_t getWriteBuffers(std::vector<iovec>&buffers,uint64_t len);

    //返回当前ByteArray里面实际存储的数据的大小
    size_t getSize()const { return m_size;}
private:

    //保证至少有size大小的可写入空间
    void addCapacity(size_t size);

    //获取当前可以写入的空间的大小
    size_t getWriteSize()const { return m_capacity-m_position; }
private:
   
    //底层每一个链表节点管理的内存大小
    size_t m_baseSize;

    //当前操作的位置
    size_t m_position;

    //容量
    size_t m_capacity;

    //实际存储的数据的大小
    size_t m_size;

    //ByteArray采用的字节序,默认是大端序
    int8_t m_endian;

    //链表的头节点
    Node * m_root;

    //当前正在操作的节点
    Node * m_cur;
};

}

#endif
