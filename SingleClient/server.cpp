#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
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
  socklen_t len;
  char buff[buffSize] = {0};
  struct sockaddr_in servaddr, cli;
  bzero(&servaddr, sizeof(servaddr));
  int clientCnt = 0;
  pid_t childpid;

  if (argc < 2) { error("ERROR: NO PORT SPECIFIED"); }
  port = atoi(argv[1]);

  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if(socketfd < 0) { error("ERROR: SOCKET CREATION FAILED"); }
  cout << "SOCKET CREATION SUCCESSFUL" << endl;

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if((bind(socketfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) { error("ERROR: SOCKET BINDING FAILED"); }
  cout << "SOCKET BINDING SUCCESSFUL" << endl;

  if((listen(socketfd, 5)) < 0) { error("ERROR: SERVER LISTEN FAILED"); }
  cout << "SERVER LISTEN SUCCESSFUL" << endl;
  len = sizeof(cli); 

  connectionfd = accept(socketfd, (struct sockaddr *) &cli, &len);
  if(connectionfd < 0) { error("SERVER ACCEPT FAILED"); }
  cout << "SERVER ACCEPT SUCCESSFUL" << endl;
  
  for(;;)
  {
    // Read and echo the received message
    ssize_t valread = read(connectionfd, buff, buffSize);

    if(strncmp(buff, "exit", 4) == 0)
    {
      cout << "SERVER EXIT" << endl;
      break;
    }

    cout << "MSG FROM CLIENT: " << buff << endl;
    send(connectionfd, buff, valread, 0);
    cout << "MSG SENT" << endl;
  }
  // Close the socket
  close(socketfd);
  close(connectionfd);
  cout << "SOCKET CONNECTION CLOSED" << endl;
  return 0;
}

