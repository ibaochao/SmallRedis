#ifndef REDISHELPER_H
#define REDISHELPER_H

#include <memory>
#include <string>
#include <vector>

#include "SkipList.h" 
#include "RedisValue/RedisValue.h"

#define DEFAULT_DB_FOLDER "data_files"
#define DATABASE_FILE_NAME "db"
#define DATABASE_FILE_NUMBER 15


// RedisHelper, 用来操控跳表
class RedisHelper{
private:
    std::string dataBaseIndex="0"; // 默认数据库索引
    std::shared_ptr<SkipList<std::string, RedisValue>> redisDataBase = std::make_shared<SkipList<std::string, RedisValue>>(); // 跳表作为redis底层数据结构
public:
    RedisHelper();  // 构造函数
    ~RedisHelper();  // 析构函数
private:
    void loadData(std::string loadPath);  // 从文件中加载数据
    std::string getFilePath();  // 获取文件路径
public:
	// 常规操作
    void flush();  // 数据持久化至文件
    std::string select(int index);  // 切换数据库, 0-15
    std::string keys(const std::string pattern="*");  // 获取全部key
    std::string dbsize() const;  // 获取key总数
    std::string exists(const std::vector<std::string>& keys);  // 查询key是否存在
    std::string del(const std::vector<std::string>&keys);  // 删除key
    std::string rename(const std::string&oldName,const std::string&newName);  // 重命名key

    // 字符串操作
    std::string set(const std::string& key, const RedisValue& value,const SET_MODEL model=NONE);  // set
    std::string setnx(const std::string& key, const RedisValue& value);  // setnx
    std::string setex(const std::string& key, const RedisValue& value);  // setex
    std::string get(const std::string&key);  // 获取value
    
    std::string incr(const std::string& key);  // 值递增
    std::string incrby(const std::string& key, int increment);  // 值递增, 按步长, 整数
    std::string incrbyfloat(const std::string&key, double increment);  // 值递增, 按步长, 小数
    std::string decr(const std::string&key);  // 值递减
    std::string decrby(const std::string&key,int increment);  // 值递减, 按步长, 整数
    
    std::string mset(std::vector<std::string>&items);  // 批量set
    std::string mget(std::vector<std::string>&keys);  // 批量get
    std::string strlen(const std::string& key);  // value长度
    std::string append(const std::string&key,const std::string &value);  // 追加内容
    
    //列表操作
    std::string lpush(const std::string&key,const std::string &value);  // 左push
    std::string rpush(const std::string&key,const std::string &value);  // 右push
    std::string lpop(const std::string&key);  // 左pop
    std::string rpop(const std::string&key);  // 右pop
    std::string lrange(const std::string&key,const std::string &start,const std::string&end);  // 获取列表范围元素

    //哈希表操作
    std::string hset(const std::string&key,const std::vector<std::string>&filed);  // HSET key field value
    std::string hget(const std::string&key,const std::string&filed);  // HGET key field
    std::string hdel(const std::string&key,const std::vector<std::string>&filed);  // HDEL key field
    std::string hkeys(const std::string&key);  // HKEYS key
    std::string hvals(const std::string&key);  // HVALS key
};


#endif