#pragma once

#include <string>
#include <stdint.h>

//需要从命令行获取的信息
struct Options{
  uint16_t port;
  int length;
  int number;
  bool transmit, receive, nodelay;
  std::string host;
  Options()
    :port(0), length(0), number(0),
     transmit(false), receive(false), nodelay(false){}
};
//获取命令行信息
bool parseCommandLine(int argc, char *argv[], Options *opt);
//从命令中解析出协议族地址
struct sockaddr_in resolveAddr(const char * host, uint16_t port);
