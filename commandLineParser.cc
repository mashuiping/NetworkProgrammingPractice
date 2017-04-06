#include "commandLineParser.h"

#include <boost/program_options.hpp>

#include <iostream>

#include <netdb.h>
#include <stdio.h>

using namespace boost::program_options;

bool parseCommandLine(int argc, char *argv[], Options *opt){
  options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "Help")
    ("port,p", value<uint16_t>(&opt->port)->default_value(5001), "TCP port")
    ("length,l", value<int>(&opt->length)->default_value(8192), "Buffer length")
    ("number,n", value<int>(&opt->number)->default_value(1025), "Number of buffers")
    ("trans,t", value<std::string>(&opt->host), "Transmit")
    ("recv,r", "Receive")
    ("nodelay,D", "set TCP_NODELAY")
    ;
  variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);

  opt->transmit = vm.count("trans");
  opt->receive = vm.count("recv");
  opt->nodelay = vm.count("nodelay");

  if(vm.count("help")){
    std::cout << desc << std::endl;
    return false;
  }
  if(opt->transmit == opt->receive){
    printf("either -t or -r must be specified.\n");
    return false;
  }

  printf("port = %d\n", opt->port);

  if(opt->transmit){
    printf("buffer length = %d\n", opt->length);
    printf("number of buffers = %d\n", opt->number);
  }
  else{
    printf("accepting...\n");
  }
  return true;
}
struct sockaddr_in resolveAddr(const char * host, uint16_t port){
  //struct hostent *gethostbyname(const char *hostname);
  /*struct hostent{
      char * h_name;
      char ** h_aliases;
      int h_addrtype;
      int h_length;
      char ** h_addr_list;
      #define h_addr h_addr_list[0]
*/
  struct hostent * getHost = gethostbyname(host);
  if(!getHost){
    printf(hstrerror(h_errno));
    exit(1);
  }
  assert(getHost->h_addrtype == AF_INET && getHost->h_length == sizeof(uint32_t));
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  //完全无关类型转换 (char *) to (struct in_addr *)
  addr.sin_addr = *reinterpret_cast<struct in_addr*>(getHost->h_addr);
  return addr;
}
