#include"RedisHelper.h"
#include"FileCreator.h"


// 获取文件路径
std::string RedisHelper::getFilePath(){
    std::string folder = DEFAULT_DB_FOLDER; //文件夹名
    std::string fileName = DATABASE_FILE_NAME; //文件名
    std::string filePath=folder+"/"+fileName+dataBaseIndex; //文件路径
    return filePath;
}


//从文件中加载
void RedisHelper::loadData(std::string loadPath){
    redisDataBase->loadFile(loadPath);
}


// 构造函数
RedisHelper::RedisHelper(){
    FileCreator::createFolderAndFiles(DEFAULT_DB_FOLDER, DATABASE_FILE_NAME, DATABASE_FILE_NUMBER);
    std::string filePath=getFilePath();
    loadData(filePath);
}


// 数据持久化至文件
void RedisHelper::flush(){
    // 打开文件并覆盖写入
    std::string filePath=getFilePath();
    std::ofstream outputFile(filePath);
    // 检查文件是否成功打开
    if (!outputFile) {
        std::cout<<"文件："<<filePath<<"打开失败"<<std::endl;
        return ;
    }
    auto currentNode=redisDataBase->getHead();
    while(currentNode!=nullptr){
        std::string key=currentNode->key;
        RedisValue value=currentNode->value;
        if(!key.empty())
            outputFile<<key<<":"<<value.dump()<<std::endl;
        currentNode=currentNode->forward[0];
    }
    // 关闭文件
    outputFile.close();
}


// 析构函数
RedisHelper::~RedisHelper(){
	flush();
}


// 切换数据库, 0-15
std::string RedisHelper::select(int index){
    if(index<0||index>DATABASE_FILE_NUMBER-1){
        return "database index out of range.";
    }
    flush(); //选择数据库之前先写入一下
    redisDataBase=std::make_shared<SkipList<std::string, RedisValue>>();
    dataBaseIndex=std::to_string(index);
    std::string filePath=getFilePath(); //根据选择的数据库，修改文件路径，然后加载

    loadData(filePath);
    return "OK";
}


// 获取全部key
// keys *
std::string RedisHelper::keys(const std::string pattern){
    std::string res="";
    auto node=redisDataBase->getHead()->forward[0];
    int count=0;
    while(node!=nullptr){
        res+=std::to_string(++count)+") "+"\""+node->key+"\""+"\n";
        node=node->forward[0];
    }
    if(!res.empty())
        res.pop_back();
    else{
        res="this database is empty!";
    }
    return res;
}


// 获取key总数
// dbsize
std::string RedisHelper::dbsize()const{
    std::string res="(integer) " +std::to_string(redisDataBase->size());
    return res;
}


// 查询key是否存在
// exists key [key ...]
// 可查询多个，返回存在的个数
std::string RedisHelper::exists(const std::vector<std::string>&keys){
    int count=0;
    for(auto& key:keys){
        if(redisDataBase->searchItem(key)!=nullptr){
            count++;
        }
    }
    std::string res="(integer) " +std::to_string(count);
    return res;
}


// 删除key
// del key [key ...]
// 可删除多个，返回删除成功的个数
std::string RedisHelper::del(const std::vector<std::string>&keys){
    int count=0;
    for(auto& key:keys){
        if(redisDataBase->deleteItem(key)){
            count++;
        }
    }
        std::string res="(integer) " +std::to_string(count);
    return res;
}


// 重命名key
// rename key newkey
std::string RedisHelper::rename(const std::string&oldName,const std::string&newName){
    auto currentNode=redisDataBase->searchItem(oldName);
    std::string resMessage="";
    if(currentNode==nullptr){
        resMessage+=oldName+" does not exist!";
        return resMessage;
    }
    currentNode->key=newName;
    resMessage="OK";
    return resMessage;
}


// 字符串set
// set key value [NX|XX]
// nx：key不存在则添加，存在则报错；xx：key不存在则报错，存在则覆盖；默认情况下存在则覆盖，不存在则添加
std::string RedisHelper::set(const std::string& key, const RedisValue& value, const SET_MODEL model){
    
    if(model==XX){
        return setex(key,value);
    }else if(model==NX){
        return setnx(key,value);
    }else{
        auto currentNode=redisDataBase->searchItem(key);
        if(currentNode==nullptr){
            setnx(key,value);
        }else{
            setex(key,value);
        }
    }
    
    return "OK";
}


// 字符串setnx
// key不存在则添加，存在则报错
std::string RedisHelper::setnx(const std::string& key, const RedisValue& value){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode!=nullptr){
        return "key: "+ key +"  exists!";
    }else{
        redisDataBase->addItem(key,value);
        
    }
    return "OK";
}


// 字符串setex
// key不存在则报错，存在则覆盖
std::string RedisHelper::setex(const std::string& key, const RedisValue& value){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode==nullptr){
        return "key: "+ key +" does not exist!";
    }else{
        currentNode->value=value;
    }
    return "OK";
}


// 字符串获取value
// get key
std::string RedisHelper::get(const std::string&key){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode==nullptr){
        return "key: "+ key +" does not exist!";
    }
    return currentNode->value.dump();
}



// 值递增
// 如果字符串中的值是数字类型的，可以使用incr命令每次递增，不是数字类型则报错。
// incr key
std::string RedisHelper::incr(const std::string& key){
    return incrby(key,1);
}


// 值递增, 按步长, 整数
// incrby key increment
std::string RedisHelper::incrby(const std::string& key,int increment){
    auto currentNode=redisDataBase->searchItem(key);
    std::string value="";
    if(currentNode==nullptr){
        value=std::to_string(increment);
        redisDataBase->addItem(key,value);
        return "(integer) "+value;
    }
    value=currentNode->value.dump();
    //去掉双引号
    value.erase(0, 1); 
    value.erase(value.size()-1);
    for(char ch:value){
        if(!isdigit(ch)){
            std::string res="The value of "+key +" is not a numeric type";
            return res;
        }
    }
    int curValue=std::stoi(value)+increment;
    value=std::to_string(curValue);
    currentNode->value=value;
    std::string res="(integer) "+value;
    return res;
}


// 值递增, 按步长, 小数
// incrbyfloat key increment
std::string RedisHelper::incrbyfloat(const std::string&key,double increment){
    auto currentNode=redisDataBase->searchItem(key);
    std::string value="";
    if(currentNode==nullptr){
        value=std::to_string(increment);
        redisDataBase->addItem(key,value);
        return "(float) "+value;
    }
    value=currentNode->value.dump();
    value.erase(0, 1); 
    value.erase(value.size()-1);
    double curValue=0.0;
    try {
        curValue = std::stod(value)+increment;
    } catch (std::invalid_argument const &e) {
        return "The value of "+key +" is not a numeric type";
    } 
    value=std::to_string(curValue);
    currentNode->value=value;
    std::string res="(float) "+value;
    return res;
}


// 值递减、
// decr key
std::string RedisHelper::decr(const std::string&key){
    return incrby(key,-1);
}


// 值递减, 按步长, 整数
// decr key increment
std::string RedisHelper::decrby(const std::string&key,int increment){
    return incrby(key,-increment);
}


// 批量set
// mset key value [key value ...]
std::string RedisHelper::mset(std::vector<std::string>&items){
    if(items.size()%2!=0){
        return "wrong number of arguments for MSET.";
    }
    for(int i=0;i<items.size();i+=2){
        std::string key=items[i];
        std::string value=items[i+1];
        set(key,value);
    }
    return "OK";
}


// 批量get
// mget key [key ...]
// Redis接收的是UTF-8的编码，如果是中文一个汉字将占3位返回。
std::string RedisHelper::mget(std::vector<std::string>&keys){
    if(keys.size()==0){
        return "wrong number of arguments for MGET.";
    }
    std::vector<std::string>values;
    std::string res="";
    for(int i=0;i<keys.size();i++){
        std::string& key=keys[i];
        std::string value="";
        auto currentNode=redisDataBase->searchItem(key);
        if(currentNode==nullptr){
            value="(nil)";
            res+=std::to_string(i+1)+") "+value+"\n";
        }else{
            value=currentNode->value.dump();
            res+=std::to_string(i+1)+") "+value+"\n";
        }
        
    }
    res.pop_back();
    return res;
}


// 获取value长度
// strlen key
std::string RedisHelper::strlen(const std::string& key){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode==nullptr){
        return "(integer) 0";
    }
    return "(integer) "+std::to_string(currentNode->value.dump().size());
}


// 追加内容
// append key value
std::string RedisHelper::append(const std::string&key,const std::string &value){
    auto currentNode=redisDataBase->searchItem(key);
    if(currentNode==nullptr){
        redisDataBase->addItem(key,value);
        return "(integer) "+std::to_string(value.size());
    }
    currentNode->value=currentNode->value.dump()+value;
    return "(integer) "+std::to_string(currentNode->value.dump().size());
}


// 列表lpush
// LPUSH key value：将一个值插入到列表头部
std::string RedisHelper::lpush(const std::string&key,const std::string &value){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int size = 0;
    if(currentNode==nullptr){
        std::vector<RedisValue>data;
        RedisValue redisList(data) ;
        RedisValue::array& valueList = redisList.arrayItems();
        valueList.insert(valueList.begin(),value);
        redisDataBase->addItem(key,redisList);
        size = 1;
    }else{
        if(currentNode->value.type()!=RedisValue::ARRAY){
            resMessage="The key:" +key+" "+"already exists and the value is not a list!";
            return resMessage;
        }else{
            RedisValue::array& valueList = currentNode->value.arrayItems();
            valueList.insert(valueList.begin(),value);
            size = valueList.size();
        }
    }

    resMessage="(integer) "+std::to_string(size);
    return resMessage;
}


// 列表rpush
// RPUSH key value：将一个值插入到列表尾部
std::string RedisHelper::rpush(const std::string&key,const std::string &value){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int size = 0;
    if(currentNode==nullptr){
        std::vector<RedisValue>data;
        RedisValue redisList(data) ;
        RedisValue::array& valueList = redisList.arrayItems();
        valueList.push_back(value);
        redisDataBase->addItem(key,redisList);
        size = 1;
    }else{
        if(currentNode->value.type()!=RedisValue::ARRAY){
            resMessage="The key:" +key+" "+"already exists and the value is not a list!";
            return resMessage;
        }else{
            RedisValue::array& valueList = currentNode->value.arrayItems();
            valueList.push_back(value);
            size = valueList.size();
        }
    }

    resMessage="(integer) "+std::to_string(size);
    return resMessage;
}


// 列表lpop
// LPOP key：移出并获取列表的第一个元素
std::string RedisHelper::lpop(const std::string&key){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int size = 0;
    if(currentNode==nullptr||currentNode->value.type()!=RedisValue::ARRAY){
        resMessage="(nil)";
    }else{
        
        RedisValue::array& valueList = currentNode->value.arrayItems();
        resMessage = (*valueList.begin()).dump();
        valueList.erase(valueList.begin());
        resMessage.erase(0,1);
        resMessage.erase(resMessage.size()-1);
    }
    return resMessage;
}


// 列表rpop
// RPOP key：移出并获取列表的最后一个元素
std::string RedisHelper::rpop(const std::string&key){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int size = 0;
    if(currentNode==nullptr||currentNode->value.type()!=RedisValue::ARRAY){
        resMessage="(nil)";
    }else{
        RedisValue::array& valueList = currentNode->value.arrayItems();
        resMessage = (valueList.back()).dump();
        valueList.pop_back();
        resMessage.erase(0,1);
        resMessage.erase(resMessage.size()-1);
    }
    return resMessage;
}


// 列表lrange
// LRANGE key start stop：获取列表指定范围内的元素
std::string RedisHelper::lrange(const std::string&key,const std::string &start,const std::string&end){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int size = 0;
    if(currentNode==nullptr||currentNode->value.type()!=RedisValue::ARRAY){
        resMessage="(nil)";
    }else{
        
        RedisValue::array& valueList = currentNode->value.arrayItems();
        int left = std::stoi(start);
        int right = std::stoi(end);
        left = std::max(left,0);
        right = std::min(right,int(valueList.size()-1));
        if(right<left||left>=valueList.size()){
            resMessage="(empty list or set)";
        }
        for(int i=left;i<=right;i++){
            auto item = valueList[i];
            resMessage+=std::to_string(i+1)+") "+item.dump();
            if(i!=right){
                resMessage+="\n";
            }
        }
    }
    return resMessage;
}


// 哈希表hset
// HSET key field value：向哈希表中添加一个字段及其值
std::string RedisHelper::hset(const std::string&key,const std::vector<std::string>&filed){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int count = 0;
    if(currentNode==nullptr){
        std::map<std::string,RedisValue>data;
        RedisValue redisHash(data) ;
        RedisValue::object& valueMap = redisHash.objectItems();
        for(int i=0;i<filed.size();i+=2){
            std::string hkey=filed[i];
            RedisValue hval=filed[i+1];
            if(!valueMap.count(hkey)){
                valueMap[hkey] = hval;
                count++;
            }
        }
        redisDataBase->addItem(key,valueMap);
    }else{
        if(currentNode->value.type()!=RedisValue::OBJECT){
            resMessage="The key:" +key+" "+"already exists and the value is not a hashtable!";
            return resMessage;
        }else{
            RedisValue::object& valueMap = currentNode->value.objectItems();
            for(int i=0;i<filed.size();i+=2){
                std::string hkey=filed[i];
                RedisValue hval=filed[i+1];
                if(!valueMap.count(hkey)){
                    valueMap[hkey] = hval;
                    count++;
                }
            }
        }
    }

    resMessage="(integer) "+std::to_string(count);
    return resMessage;
}


// 哈希表hget
// HGET key field：获取哈希表中指定字段的值
std::string RedisHelper::hget(const std::string&key,const std::string&filed){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int count = 0;
    if(currentNode==nullptr||currentNode->value.type()!=RedisValue::OBJECT){
        resMessage="(nil)";
    }else{
        RedisValue::object& valueMap = currentNode->value.objectItems();
        if(!valueMap.count(filed)){
            resMessage="(nil)";
        }else{
            resMessage = valueMap[filed].stringValue();
        }
        
    }
    return resMessage;
}


// 哈希表hdel
// HDEL key field：删除哈希表 key 中的一个或多个指定字段
std::string RedisHelper::hdel(const std::string&key,const std::vector<std::string>&filed){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int count = 0;
    if(currentNode==nullptr||currentNode->value.type()!=RedisValue::OBJECT){
        count = 0;
    }else{
        RedisValue::object& valueMap = currentNode->value.objectItems();
        for(auto& hkey:filed){
            if(valueMap.count(hkey)){
                count++;
                valueMap.erase(hkey);
            }
        }
    }
    resMessage="(integer) "+std::to_string(count);
    return resMessage;
}


// 哈希表hkeys
// HKEYS key：获取哈希表中的所有字段名
std::string RedisHelper::hkeys(const std::string&key){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int count = 0;
    if(currentNode==nullptr){
        resMessage="The key:" +key+" "+"does not exist!";
        return resMessage;
    }else{
        if(currentNode->value.type()!=RedisValue::OBJECT){
            resMessage="The key:" +key+" "+"already exists and the value is not a hashtable!";
            return resMessage;
        }else{
            RedisValue::object& valueMap = currentNode->value.objectItems();
            int index = 1;
            for(auto hkey:valueMap){
                resMessage+=std::to_string(index)+") "+hkey.first+"\n";
                index++;
            }
            resMessage.erase(resMessage.size()-1);
        }
    }
    return resMessage;
}


// 哈希表hvals
// HVALS key：获取哈希表中的所有值
std::string RedisHelper::hvals(const std::string&key){
    auto currentNode=redisDataBase->searchItem(key);
    std::string resMessage = "";
    int count = 0;
    if(currentNode==nullptr){
        resMessage="The key:" +key+" "+"does not exist!";
        return resMessage;
    }else{
        if(currentNode->value.type()!=RedisValue::OBJECT){
            resMessage="The key:" +key+" "+"already exists and the value is not a hashtable!";
            return resMessage;
        }else{
            RedisValue::object& valueMap = currentNode->value.objectItems();
            int index = 1;
            for(auto hkey:valueMap){
                resMessage+=std::to_string(index)+") "+hkey.second.stringValue()+"\n";
                index++;
            }
            resMessage.erase(resMessage.size()-1);
        }
    }
    return resMessage;
}