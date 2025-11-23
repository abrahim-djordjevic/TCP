#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#define buffSize 256

using namespace std;

void error(string msg)
{
  cout << msg << endl;
  exit(1);
}

int main(int argc, char* argv[])
{
  int port, socketfd, connectionfd;
  char buff[buffSize] = {0};
  struct sockaddr_in servaddr, cli;
  bzero(&servaddr, sizeof(servaddr));

  if (argc < 2) { error("ERROR: NO PORT SPECIFIED"); }
  port = atoi(argv[1]);

  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if(socketfd < 0) { error("ERROR: SOCKET CREATION FAILED"); }
  cout << "SOCKET CREATION SUCCESSFUL" << endl;

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(port);

  if (connect(socketfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) { error("ERROR: CONNECTION TO SERVER FAILED"); }
  cout << "CONNECTED TO SERVER" << endl;

  for(;;)
  {
    cout << "ENTER MESSAGE:" << endl;
  
    std::string msg;
    getline(cin, msg);
    send(socketfd, msg.c_str(), msg.size(), 0);
    cout << "MSG SENT TO SERVER" << endl;

    if(msg == "exit")
    {
      cout << "CLIENT EXIT" << endl;
      break;
    }

    ssize_t valread = read(socketfd, buff, buffSize);
    cout << "MSG RECEIVED: " << buff << endl;
  }
  close(socketfd);
  return 0;
}

