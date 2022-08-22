#ifndef _JKYI_DB_H_
#define _JKYI_DB_H_


#include<memory>
#include<string>

namespace JKYi{

//数据库的虚基类,用以提供公共的接口

class ISQLData{
public:
    typedef std::shared_ptr<ISQLData> ptr;

    virtual ~ISQLData(){}

    virtual int getErrno()const = 0;
    virtual const std::string& getErrStr()const = 0;

    virtual int getDataCount() = 0;
    virtual int getColumnCount() = 0;
    virtual int getColumnBytes(int idx) = 0;
    virtual int getColumnType(int idx) = 0;
    virtual std::string getColumnName(int idx) = 0;

    virtual bool isNull(int idx) = 0;
    virtual int8_t getInt8(int idx) = 0;
    virtual uint8_t getUint8(int idx) = 0;
    virtual int16_t getInt16(int idx) = 0;
    virtual uint16_t getUint16(int idx) = 0;
    virtual int32_t getInt32(int idx) = 0;
    virtual uint32_t getUint32(int idx) = 0;
    virtual int64_t getInt64(int idx) = 0;
    virtual uint64_t getUint64(int idx) = 0;
    virtual float getFloat(int idx) = 0;
    virtual double getDouble(int idx) = 0;
    virtual std::string getString(int idx) = 0;
    virtual std::string getBlob(int idx) = 0;
    virtual time_t getTime(int idx) = 0;
    virtual bool next() = 0;
};

class ISQLUpdate{
public:
    virtual ~ISQLUpdate(){}
    virtual int execute(const char * format,...) = 0;
    virtual int execute(const std::string& sql) = 0;
    virtual int64_t getLastInsertId() = 0;
};

class ISQLQuery{
public:
    virtual ~ISQLQuery(){}
    virtual ISQLData::ptr query(const char * format,...) = 0;
    virtual ISQLData::ptr query(const std::string& sql) = 0;
};

//用来实现sql的预处理
class IStmt{
public:
    typedef std::shared_ptr<IStmt> ptr;

    virtual ~IStmt(){}
    virtual int bindInt8(int idx,const int8_t& value) = 0;
    virtual int bindUint8(int idx,const uint8_t& value) = 0;
    virtual int bindInt16(int idx,const int16_t& value) = 0;
    virtual int bindUint16(int idx,const uint16_t& value) = 0;
    virtual int bindInt32(int idx,const int32_t& value) = 0;
    virtual int bindUint32(int idx,const uint32_t& value) = 0;
    virtual int bindInt64(int idx,const int64_t& value) = 0;
    virtual int bindUint64(int idx,const uint64_t& value) = 0;
    virtual int bindFloat(int idx,const float& value) = 0;
    virtual int bindDouble(int idx,const double& value) = 0;
    virtual int bindString(int idx,const std::string& value) = 0;
    virtual int bindString(int idx,const char * value) = 0;
    virtual int bindBlob(int idx,const void * value,int64_t size) = 0;
    virtual int bindBlob(int idx,const std::string& value) = 0;
    virtual int bindTime(int idx,const time_t& value) = 0;
    virtual int bindNull(int idx) = 0;

    virtual int execute() = 0;
    //在主键字段被auto_increament修饰并且上一次插入操作用户并没有显式的指定主键值的情况下
    //返回上一次插入操作自动产生的主键id。如果用户有显式的指定值的话，那么返回的时0
    virtual int64_t getLastInsertId() = 0;
    virtual ISQLData::ptr query() = 0;

    virtual int getErrno() = 0;
    virtual std::string getErrStr() = 0;

};

class ITransaction:public ISQLUpdate{
public:
    typedef std::shared_ptr<ITransaction> ptr;
    virtual ~ITransaction(){}

    virtual bool begin() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0; 
};

class IDB:public ISQLUpdate,public ISQLQuery{
public:
    typedef std::shared_ptr<IDB> ptr;
    virtual ~IDB(){}

    virtual IStmt::ptr prepare(const std::string& stmt) = 0;
    virtual int getErrno() = 0;
    virtual std::string getErrStr() = 0;
    virtual ITransaction::ptr openTransaction(bool auto_commit = false) = 0;
};

}
#endif
