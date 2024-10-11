#ifndef PARSER_FLYWEIGHT_FACTORY
#define PARSER_FLYWEIGHT_FACTORY

#include<unordered_map>

#include"CommandParser.h"


// 解析器工厂
class ParserFlyweightFactory{
	private:
		std::unordered_map<std::string, std::shared_ptr<CommandParser>> parserMaps;  // 命令解析器映射
		std::shared_ptr<CommandParser> createCommandParser(std::string& command);  // 创建命令解析器
	public:
		std::shared_ptr<CommandParser> getParser(std::string& command);  // 获取命令解析器
};


#endif