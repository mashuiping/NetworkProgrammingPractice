#include "commandLineParser.h"
#include "clientAndServer.h"

int acceptCli(uint16_t port){
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listenfd >= 0);
  int yes = 1;
  //int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
  if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))){
    perror("setsockopt");
    exit(1);
  }
  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = INADDR_ANY;
  if(bind(listenfd, reinterpret_cast<struct sockaddr *>(&servaddr), sizeof(servaddr))){
    perror("bind");
    exit(1);
  }
  if(listen(listenfd, 5)){
    perror("listen");
    exit(1);
  }
  int sockfd = accept(listenfd, reinterpret_cast<struct sockaddr *>(NULL), NULL);
  if(sockfd < 0){
    perror("accept");
    exit(1);
  }
  close(listenfd);
  return sockfd;
}

int read_n(int sockfd, void *buf, int length){
  int nread = 0;
  while(nread < length){
    ssize_t nr = read(sockfd, static_cast<char*>(buf) + nread, length - nread);
    if(nr > 0){
      nread += static_cast<int>(nr);
    }
    else if(nr == 0){
      break;
    }
    else if(errno != EINTR){
      perror("read");
      break;
    }
  }
  return nread;
}

int write_n(int sockfd, const void * buf, int length){
  int nwrite = 0;
  while(nwrite < length){
    ssize_t nw = write(sockfd, static_cast<const char *>(buf) + nwrite, length - nwrite);
    if(nw > 0){
      nwrite += static_cast<int>(nw);
    }
    else if(nw == 0){
      break; //EOF
    }
    else if(errno != EINTR){
      perror("read");
      break;
    }
  }
  return nwrite;
}

void transmit(const Options & opt){
  //建立连接
  struct sockaddr_in addr = resolveAddr(opt.host.c_str(), opt.port);
  printf("connecting to %s:%d\n", inet_ntoa(addr.sin_addr), opt.port);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  assert(sockfd > 0);
  int connfd = connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
  if(connfd == -1){
    perror("connect");
    printf("Unable to connect %s\n", opt.host.c_str());
    close(sockfd);
    return ;
  }

  printf("connected\n");
  // start timing
  muduo::Timestamp start(muduo::Timestamp::now());
  // construct sessionMessage
  struct SessionMessage sessionMessage = {0, 0};
  sessionMessage.number = ntohl(opt.number);
  sessionMessage.length = ntohl(opt.length);
  // send sessionMessage
  if(write_n(sockfd, &sessionMessage, sizeof(sessionMessage)) != sizeof(sessionMessage)){
    perror("write SessionMessage");
    exit(1);
  }
  // construct payload
  const int total_len = static_cast<int>(sizeof(int32_t) + opt.length);
  PayloadMessage * payload = static_cast<PayloadMessage*>(malloc(total_len));
  assert(payload);
  payload->length = htonl(opt.length);
  for(int i = 0; i < opt.length; i++){
    payload->data[i] = "0123456789ABCDEF"[i%16];
  }
  double total_mb = 1.0 * opt.length * opt.number / 1024 / 1024;
  printf("%.3f MiB in total\n", total_mb);
  // send PayloadMessage
  for(int i = 0; i < opt.number; i++){
    int nw = write_n(sockfd, payload, total_len);
    assert(nw == total_len);

    int ack = 0;
    //god job! the ack package is 4 byte, sizeof(int)!;
    int nr = read_n(sockfd, &ack, sizeof(ack));
    assert(nr == sizeof(ack));
    ack = ntohl(ack);
    assert(ack == opt.length);
  }
  //释放内存
  free(payload);
  //关闭sock连接
  close(sockfd);
  //计算总共所用时间和传输速率
  double elapsed = timeDifference(muduo::Timestamp::now(), start);
  printf("%.3f seconds\n%.3f MiB/s\n",elapsed, total_mb / elapsed);
}

void receive(const Options &opt){
  int sockfd = acceptCli(opt.port);
  struct SessionMessage sessionMessage = {0, 0};
  if(read_n(sockfd, &sessionMessage, sizeof(sessionMessage)) != sizeof(sessionMessage)){
    perror("read SessionMessage");
    exit(1);
  }
  sessionMessage.number = ntohl(sessionMessage.number);
  sessionMessage.length = ntohl(sessionMessage.length);
  printf("receive number = %d\n receive length = %d\n",
          sessionMessage.number, sessionMessage.length);
  const int total_len = static_cast<int>(sizeof(int32_t) + sessionMessage.length);
  PayloadMessage * payload = static_cast<PayloadMessage*>(malloc(total_len));
  assert(payload);

  //接收payload
  for(int i = 0; i < sessionMessage.number; i++){
    payload->length = 0;
    if(read_n(sockfd, &payload->length, sizeof(payload->length)) != sizeof(payload->length)){
      perror("read length");
      exit(1);
    }
    payload->length = ntohl(payload->length);
    assert(payload->length == sessionMessage.length);
    if(read_n(sockfd, payload->data, payload->length) != payload->length){
      perror("read payload data");
      exit(1);
    }
    int32_t ack = htonl(payload->length);
    if(write_n(sockfd, &ack, sizeof(ack)) != sizeof(ack)){
      perror("write ack");
      exit(1);
    }
  }
  free(payload);
  close(sockfd);
}
