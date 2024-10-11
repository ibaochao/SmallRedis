#ifndef REDIS_SERVER_H
#define REDIS_SERVER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <future>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <unistd.h>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <signal.h>
#include<fcntl.h>
#include <cstring> 
#include <queue>
#include <string>

#include "ParserFlyweightFactory.h"

using namespace std;


class RedisServer {
	private:
		std::unique_ptr<ParserFlyweightFactory> flyweightFactory; // 解析器工厂
		int port;  // 端口
		std::atomic<bool> stop{false};  // 是否停止
		pid_t pid;  // 进程ID
		std::string logoFilePath;  // logo位置
		bool startMulti = false;
		bool fallback = false;
		std::queue<std::string>commandsQueue;  // 命令队列

	private:
		RedisServer(int port = 9000, const std::string& logoFilePath = MY_PROJECT_DIR_LOGO);  // 构造函数
		static void signalHandler(int sig);  // 信号处理函数
		void printLogo();  // 打印Logo
		void printStartMessage();  // 打印启动信息
		void replaceText(std::string &text, const std::string &toReplaceText, const std::string &replaceText);  // 辅助函数
		std::string getDate();  // 辅助函数
		string executeTransaction(std::queue<std::string>&commandsQueue);  // 执行事务
		
	public:
		string handleClient(string receivedData);  // 处理客户端
		static RedisServer* getInstance();  // 单例模式
		void start();  // 启动
};


#endif 
