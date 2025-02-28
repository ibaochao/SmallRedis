#ifndef REDISVALUE_H 
#define REDISVALUE_H

#include<iostream>
#include<vector>
#include<map>
#include<initializer_list>
#include<memory>
#include<cmath>
#include<limits>



class RedisValueType;


// RedisValue
class RedisValue{
public:
    enum Type{
        NUL,NUMBER,BOOL,STRING,ARRAY,OBJECT  // 支持的数据类型（空类型，数值型，布尔型，字符串型，数组（列表）型，对象（哈希表）型）
    };
    typedef std::vector<RedisValue> array; // 自定义数组（列表）类型
    typedef std::map<std::string,RedisValue> object; // 自定义对象（哈希表）类型
	
	// 构造函数
    RedisValue() noexcept;  // 默认
    RedisValue(std::nullptr_t) noexcept;  // 空指针类型
    RedisValue(const std::string& value);  // 字符串类型，左值
    RedisValue(std::string&& value);  // 字符串类型，右值
    RedisValue(const char* value);  // C风格字符串类型
    RedisValue(const array&value);  // 数组（列表）类型，左值
    RedisValue(array&& values);  // 数组（列表）类型，右值
    RedisValue(const object& values);  // 对象（哈希表）类型，左值
    RedisValue(object && values);  // 对象（哈希表）类型，右值

    // 从具有 toJson 成员函数的类实例构造 RedisValue
    template<class T,class = decltype(&T::toJson)>
    RedisValue(const T & t) : RedisValue(t.toJson()){}

    // 从支持 begin/end 迭代器的容器构造 RedisValue 对象
    template <class M, typename std::enable_if<std::is_constructible<std::string, decltype(std::declval<M>().begin()->first)>::value
    && std::is_constructible<RedisValue, decltype(std::declval<M>().begin()->second)>::value, int>::type = 0>
    RedisValue(const M & m) : RedisValue(object(m.begin(), m.end())) {}

    template <class V, typename std::enable_if<std::is_constructible<RedisValue, decltype(*std::declval<V>().begin())>::value, int>::type = 0>
    RedisValue(const V & v) : RedisValue(array(v.begin(), v.end())) {}
    
    RedisValue(void*) = delete; // 禁止从 void* 构造

    // 类型判断函数
    Type type() const;
    bool isNull() const{ return type()==NUL;}
    bool isNumber() const { return type()==NUMBER;}
    bool isBoolean() const { return type()==BOOL;}
    bool isString() const { return type()==STRING; }
    bool isArray() const { return type() == ARRAY; }
    bool isObject() const { return type() == OBJECT; }

    // 获取值的函数
    std::string& stringValue() ;
    array& arrayItems() ;
    object &objectItems() ;

    // 重载 [] 操作符，用于访问数组元素和对象成员
    RedisValue & operator[] (size_t i) ;
    RedisValue & operator[] (const std::string &key) ;

    // 序列化函数
    void dump(std::string &out) const;
    std::string dump() const{
        std::string out;
        dump(out);
        return out;
    }

    // 解析 JSON 文本的静态函数
    static RedisValue parse(const std::string&in, std::string& err);
    static RedisValue parse(const char* in, std::string& err);

    // 解析多个 JSON 值的静态函数
    static std::vector<RedisValue> parseMulti(
        const std::string&in,
        std::string::size_type & parserStopPos,
        std::string& err
    );

    static std::vector<RedisValue>parseMulti(
        const std::string & in,
        std::string & err
    );

    // 重载比较运算符
    bool operator== (const RedisValue &rhs) const;
    bool operator< (const RedisValue &rhs) const;
    bool operator!= (const RedisValue &rhs) const {return !(*this==rhs);}
    bool operator<= (const RedisValue &rhs) const {return !(rhs<*this);}
    bool operator> (const RedisValue &rhs) const { return (rhs<*this);}
    bool operator>= (const RedisValue &rhs) const {return !(*this<rhs);}
    
    // 检查 RedisValue 对象是否符合指定形状
    typedef std::initializer_list<std::pair<std::string,Type>> shape;
    bool hasShape(const shape &types,std::string &err) ;
    
private:
    std::shared_ptr<RedisValueType> redisValue; // 指向实际存储的智能指针
};

#endif