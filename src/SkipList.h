#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <cstring>
#include <string>
#include <fstream>
#include <mutex>

#include "global.h"
#include "RedisValue/RedisValue.h"

#define MAX_SKIP_LIST_LEVEL 32  // 跳表最大层高
#define PROBABILITY_FACTOR 0.25  // 跳表随机层高的概率因子
#define DELIMITER ":"  // 跳表键值对分隔符
#define SAVE_PATH "data_file"  // 跳表数据保存文件路径


// 跳表节点, 模板
template<typename Key,typename Value>
class SkipListNode{
	public:  // 类似结构体，权限设置为公有
		Key key;  // key
		Value value;  // value
		std::vector<std::shared_ptr<SkipListNode<Key, Value>>> forward;  // forword, 共享指针
		SkipListNode(Key key, Value value, int maxLevel=MAX_SKIP_LIST_LEVEL): key(key), value(value), forward(maxLevel, nullptr){}  // 构造函数
};


// 跳表, 模板
template<typename Key, typename Value>
class SkipList{
	private:
		int currentLevel;  // 当前最大层数
		std::shared_ptr<SkipListNode<Key, Value>> head;  // 头节点, 共享指针
		std::mt19937 generator{ std::random_device{}()};  // 随机数生成器
		std::uniform_real_distribution<double> distribution;  // 均匀分布
		int elementNumber=0;  // 元素计数
		std::ofstream writeFile;  // 写文件流
		std::ifstream readFile;  // 读文件流
		std::mutex mutex;  // 互斥量, 跳表
	private:
		int randomLevel();  // 随机生成新节点层数
		bool parseString(const std::string&line, std::string&key, std::string&value);  // 解析字符串
		bool isVaildString(const std::string&line);  // 字符串是否有效
	public:
		SkipList();  // 构造函数
		~SkipList();  // 析构函数
		bool addItem(const Key& key, const Value& value);  // 添加节点
		std::shared_ptr<SkipListNode<Key,Value>> searchItem(const Key& key);  // 查找节点
		bool modifyItem(const Key& key, const Value& value);  // 修改节点
		bool deleteItem(const Key& key);  // 删除节点
		void printList();  // 打印跳表
		void dumpFile(std::string save_path);  // 保存跳表到文件
		void loadFile(std::string load_path);  // 从文件加载跳表
		int size();  // 返回跳表元素个数
		int getCurrentLevel(){return currentLevel;}  // 返回跳表的当前最大层数
		std::shared_ptr<SkipListNode<Key,Value>> getHead(){return head;}  // 返回跳表头节点, 智能指针
};


// 构造函数
template<typename Key,typename Value>
SkipList<Key,Value>::SkipList(): currentLevel(0), distribution(0, 1){  // 初始层数0, 0-1均匀分布
    Key key;
    Value value;
    this->head=std::make_shared<SkipListNode<Key, Value>>(key,value);  // 初始化头指针, 数组层数为最大层数
}


// 析构函数
template<typename Key,typename Value>
SkipList<Key,Value>::~SkipList(){
    if(this->readFile){
        readFile.close();  // 关闭读文件流
    }
    if(this->writeFile){
        writeFile.close();  // 关闭写文件流
    }
}


// 随机生成新节点的层高
template<typename Key, typename Value>
int SkipList<Key,Value>::randomLevel(){
    int level=1;  // 默认1开始
    while(distribution(generator)< PROBABILITY_FACTOR && level<MAX_SKIP_LIST_LEVEL){  // 随机数<0.25且层数未越界, 层数加一, (1/4)^(n-1)的概率层高为n
        level++;
    }
    return level;
}


// 添加节点
template<typename Key,typename Value>
bool SkipList<Key,Value>::addItem(const Key& key, const Value& value){
    mutex.lock();  // 加锁
    auto currentNode=this->head; // 头指针
    std::vector<std::shared_ptr<SkipListNode<Key,Value>>> update(MAX_SKIP_LIST_LEVEL, this->head);  // 辅助数组
	
    for(int i=currentLevel-1;i>=0;i--){  // 所有层，自顶向下
        while(currentNode->forward[i]!=nullptr && currentNode->forward[i]->key<key){  // 每一层，从左到右
            currentNode=currentNode->forward[i];
        }
        update[i]=currentNode;  // 更新，没找到，或找到(每层第一个大于等于目标key的节点)
    }
	
    int newLevel=this->randomLevel();  // 随机生成新节点的层数
    currentLevel=std::max(newLevel, currentLevel);  // 更新当前最大层数
    std::shared_ptr<SkipListNode<Key,Value>> newNode=std::make_shared<SkipListNode<Key,Value>>(key, value, newLevel);  // 生成新节点
	
	// 修改插入位置指针指向
    for(int i=0;i<newLevel;i++){
        newNode->forward[i]=update[i]->forward[i];
        update[i]->forward[i]=newNode;
    }
    elementNumber++;  // 元素节点加1
    mutex.unlock();  // 释放锁
    return true;  // 添加成功
}


// 查找节点
template<typename Key,typename Value>
std::shared_ptr<SkipListNode<Key,Value>> SkipList<Key,Value>::searchItem(const Key& key){
    mutex.lock();  // 加锁
    std::shared_ptr<SkipListNode<Key,Value>> currentNode=this->head; // 头指针
    if(!currentNode){  // 跳表为空，不允许查找
        mutex.unlock();  // 释放锁
        return nullptr;  // 返回空指针
    }
    for(int i=currentLevel-1;i>=0;i--){  // 所有层，自顶向下
        while(currentNode->forward[i]!=nullptr && currentNode->forward[i]->key<key){  // 每一层，从左到右
            currentNode=currentNode->forward[i];
        }
    }
    currentNode=currentNode->forward[0];  // 没找到，或找到(第一个大于或等于目标key的节点)
    if(currentNode && currentNode->key==key){  // 找到等于目标key的节点
        mutex.unlock();  // 释放锁
        return currentNode;  // 返回此节点共享指针
    }
	// 没找到等于目标key的节点
    mutex.unlock();  // 释放锁
    return nullptr;  // 返回空指针
}


// 修改节点
template<typename Key,typename Value>
bool SkipList<Key,Value>::modifyItem(const Key&key, const Value& value){
    std::shared_ptr<SkipListNode<Key,Value>> targetNode=this->searchItem(key);  // 先查找目标节点
    mutex.lock();  // 加锁
    if(targetNode==nullptr){  // 没找到
        mutex.unlock();  // 释放锁
        return false;  // 修改失败
    }
    targetNode->value=value;  // 新值覆盖旧值
    mutex.unlock();  // 释放锁
    return true;  // 修改成功

}


// 删除节点
template<typename Key,typename Value>
bool SkipList<Key,Value>::deleteItem(const Key& key){
    mutex.lock();  // 加锁
    std::shared_ptr<SkipListNode<Key,Value>> currentNode=this->head; // 头指针
    std::vector<std::shared_ptr<SkipListNode<Key,Value>>> update(MAX_SKIP_LIST_LEVEL, head);  // 辅助数组
	
    for(int i=currentLevel-1;i>=0;i--){  // 所有层，自顶向下
        while(currentNode->forward[i]!=nullptr && currentNode->forward[i]->key<key){  // 每一层，从左到右
            currentNode=currentNode->forward[i];
        }
        update[i]=currentNode;  // 更新，没找到，或找到(每层第一个大于等于目标key的节点)
    }
    currentNode=currentNode->forward[0];  // 没找到，或找到(第一个大于或等于目标key的节点)
    if(currentNode==nullptr || currentNode->key!=key){  // 没找到
        mutex.unlock();  // 释放锁
        return false;  // 修改失败
    }
	// 找到了
    for(int i=0;i<currentLevel;i++){
        if(update[i]->forward[i]!=currentNode){  // 后面不是目标节点的不用修改
            break;
        }
        update[i]->forward[i]=currentNode->forward[i];  // 需要修改指针指向
    }
    currentNode.reset();  // 共享指针重置, 即删除此节点
    while(currentLevel>1 && head->forward[currentLevel-1]==nullptr){  // 如果删除的是最高层最左边节点, 从顶至下依次判断是否需要减一
        currentLevel--;
    }
    elementNumber--;  // 节点减1
    mutex.unlock();  // 释放锁
    return true;  // 修改成功
}


//打印跳表
template<typename Key,typename Value>
void SkipList<Key,Value>::printList(){
    mutex.lock();  // 加锁
    for(int i=currentLevel;i>=0;i--){  // 所有层，自顶向下
        auto node=this->head->forward[i];
        std::cout<<"Level"<<i+1<<":";
        while(node!=nullptr){  // 每一层，从左到右
            std::cout<<node->key<<DELIMITER<<node->value<<"; ";
            node=node->forward[i];
        }
        std::cout<<std::endl;
    }
    mutex.unlock();  // 释放锁
}


// 字符串是否有效
template<typename Key,typename Value>
bool SkipList<Key,Value>::isVaildString(const std::string&line){
    if(line.empty()){  // 空, 无效
        return false;
    }
    if(line.find(DELIMITER)==std::string::npos){  // 空, 不含分隔符无效
        return false;
    }
    return true;  // 其它有效
}


// 解析字符串
template<typename Key,typename Value>
bool SkipList<Key,Value>::parseString(const std::string&line, std::string&key, std::string&value){  // 解析到的内容直接写入key, vlaue
    if(!isVaildString(line)){  // 字符串无效
        return false;
    }
    int index=line.find(DELIMITER);  // 分隔符位置
    key=line.substr(0, index);  // 分割key
    value=line.substr(index+1);  // 分割value
    return true;  // 返回成功
}


// 保存跳表到文件
template<typename Key,typename Value>
void SkipList<Key,Value>::dumpFile( std::string save_path){
    mutex.lock();  // 加锁
    writeFile.open(save_path);  // 打开文件
    auto node=this->head->forward[0];  // 只遍历底层就能遍历全部节点
    while(node!=nullptr){
        writeFile<<node->key<<DELIMITER<<node->value.dump()<<"\n"; //写入文件 key本身就是字符串, 需要将value转换为字符串
        node=node->forward[0];
    }
    writeFile.flush();  // 刷新缓冲区
    writeFile.close();  // 关闭文件
    mutex.unlock();  // 释放锁
}


// 从文件加载跳表
template<typename Key,typename Value>
void SkipList<Key,Value>::loadFile(std::string load_path){
	// mutex.lock();  // 加锁
    readFile.open(load_path);  // 打开文件
    if(!readFile.is_open()){  // 打开文件失败
        mutex.unlock();  // 释放锁
        return;  // 返回
    }
    std::string line;
    std::string key;
    std::string value;
    std::string err;
    while(std::getline(readFile, line)){  // 读取数据
        if(parseString(line, key, value)){
            addItem(key, RedisValue::parse(value, err)); // 将value字符串转换为RedisValue对象, 然后插入
        }
    }
    readFile.close();  // 关闭文件
	// mutex.unlock();  // 释放锁
} 


// 返回跳表元素个数
template<typename Key,typename Value>
int SkipList<Key,Value>::size(){
    mutex.lock();  // 加锁
    int ret=this->elementNumber;  // 读取元素节点个数
    mutex.unlock();  // 释放锁
    return ret;  // 返回
}


#endif