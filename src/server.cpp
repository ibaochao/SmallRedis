#include "RedisServer.h"
#include "buttonrpc.hpp"


int main() {
	buttonrpc server;  // 服务端
    server.as_server(9000);  // 服务端端口
	RedisServer::getInstance()->start();
    server.bind("redis_command", &RedisServer::handleClient, RedisServer::getInstance());  // 绑定
    server.run();
    RedisServer::getInstance()->start();  // 启动
}
