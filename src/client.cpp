#include <iostream>
#include <string>

#include "buttonrpc.hpp"


int main() {
    std::string hostName = "127.0.0.1";  // 服务端IP
    int port = 9000;  // 服务端Port

    buttonrpc client;  // 客户端
    client.as_client(hostName, port);
    client.set_timeout(2000);  // 连接超时

    std::string message;  // 临时消息
    while(true){
        std::cout << hostName << ":" << port << "> ";
        std::getline(std::cin, message);  // 键盘输入要发送的内容
        std::string res = client.call<std::string>("redis_command", message).val();
        if(res.find("stop") != std::string::npos){  // 遇到结束字符 
            break;
        }
        std::cout << res << std::endl;
    }
    return 0;
}
