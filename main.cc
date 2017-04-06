#include "commandLineParser.h"
#include "clientAndServer.h"

#include <assert.h>

int main(int argc, char *argv[]) {
  Options options;
  parseCommandLine(argc, argv, &options);
  if(options.transmit){
    transmit(options);
  }
  else if(options.receive){
    receive(options);
  }
  else{
    assert(0);
  }
  return 0;
}
