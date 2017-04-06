#pragma once

#include "commandLineParser.h"
#include "protocol.h"

#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <muduo/base/Timestamp.h>
// write n byte to sockfd
int write_n(int sockfd, const void*buf, int length);
// read n byte from sockfd
int read_n(int sockfd, void *buf, int length);
// accept client
int acceptCli(uint16_t port);
// transmit SessionMessage or playload to server
void transmit(const Options &opt);
// receive SessionMessage or playload from client
void receive(const Options & opt);
