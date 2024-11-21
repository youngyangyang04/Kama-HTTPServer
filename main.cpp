#include <string>

#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include "HttpServer.h"

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  
  std::string serverName = "HttpServer";
  int port = 8888;
  
  // 参数解析
  int opt;
  const char* str = "p:";
  while ((opt = getopt(argc, argv, str)) != -1)
  {
    switch (opt)
    {
      case 'p':
      {
        port = atoi(optarg);
        break;
      }
      default:
        break;
    }
  }
  
  muduo::net::InetAddress listenAddr(port);
  HttpServer server(listenAddr, serverName);
  server.setThreadNum(4);
  server.start();
}
