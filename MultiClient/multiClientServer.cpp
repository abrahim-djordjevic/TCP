#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/poll.h>
#define buffSize 256

using namespace std;

int endServer = 0;

void error(string msg)
{
    cout << msg << endl;
    endServer = 1;
}

void error(string msg, int socket)
{
  close(socket);
  error(msg);
}

int main(int argc, char* argv[])
{
  int port, socketfd = -1, rc, clientCnt = 0, on = 1, new_sd = -1, len;
  struct sockaddr_in servaddr, cli;
  char buff[buffSize] = {0};
  bzero(&servaddr, sizeof(servaddr));
  int    timeout;
  struct pollfd fds[200];
  int    nfds = 1, closeConn, removeConn;

  if (argc < 2) { error("ERROR: NO PORT SPECIFIED"); }
  port = atoi(argv[1]);

  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) { error("ERROR: SOCKET CREATION FAILED"); }
  cout << "SOCKET CREATION SUCCESSFUL" << endl;

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  // make socketfd reusable
  rc = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
  if (rc < 0) { error("ERROR: setsockopt FAILED", socketfd); }
  
  // set socket to be nonblocking
  rc = ioctl(socketfd, FIONBIO, (char *)&on);
  if (rc < 0) { error("ERROR: ioctl FAILED", socketfd); }

  if ((bind(socketfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) { error("ERROR: SOCKET BINDING FAILED", socketfd); }
  cout << "SOCKET BINDING SUCCESSFUL" << endl;

  if ((listen(socketfd, 5)) < 0) { error("ERROR: SERVER LISTEN FAILED", socketfd); }
  cout << "SERVER LISTEN SUCCESSFUL" << endl;

  memset(fds, 0, sizeof(fds));
  fds[0].fd = socketfd;
  fds[0].events = POLLIN;
  timeout = 3 * 60 * 1000;

  do
  {
    cout << "WAITING ON POLL..." << endl;
    rc = poll(fds, nfds, timeout);

    if (rc < 0) { error("ERROR: POLL FAILED", socketfd); }
    if (rc == 0) { error("ERROR: POLL TIMEOUT", socketfd); }
    clientCnt = nfds;
    cout << clientCnt << endl;
    for (int i = 0; i < clientCnt; i++)
    {
      if (fds[i].revents == 0) { continue; }

      if (fds[i].revents != POLLIN) 
      {
        error("ERROR: UNEXPECRED POLLIN", socketfd);
        break;
      }

      if (fds[i].fd == socketfd)
      {
        cout << "LISTENING SOCKET IS READABLE" << endl;
        do
        {
          new_sd = accept(socketfd, NULL, NULL);
          if (new_sd < 0) 
          {
            if (errno != EWOULDBLOCK){ error("ERROR: SOCKET ACCEPT FAILED", socketfd); }
            break;
          }

          cout << "NEW INCOMING CONNECTION " << new_sd << endl;
          fds[nfds].fd = new_sd;
          fds[nfds].events = POLLIN;
          nfds++;

        } while (new_sd != -1);
      }
      else
      {
        cout << "SOCKET IS READABLE " << fds[i].fd << endl;
        closeConn = 0;

          rc = recv(fds[i].fd, buff, sizeof(buff), 0);
          if (rc < 0)
          {
            if (errno != EWOULDBLOCK){ error("ERROR: recv FAILED", socketfd); }
            closeConn = 1;
            break;
          }

          if (rc == 0)
          {
            cout << "CONNECTION CLOSED BY CLIENT" << endl;
            closeConn = 1;
            break;
          }

          // data recieved
          len = rc;
          cout << len << " BYTES RECIEVED" << endl;

          rc = send(fds[i].fd, buff, len, 0);
          if (rc < 0) 
          {
            error("ERROR: send FAILED", socketfd);
            closeConn = 1;
            break;
          }

        if (closeConn == 1) 
        {
          close(fds[i].fd);
          fds[i].fd = -1;
          removeConn = 1;
        }
      }
    }

    if (removeConn == 1)
    {
      removeConn = 0;
      for (int i = 0; i < nfds; i++)
      {
        if (fds[i].fd == -1)
        {
          // move fd values from i to end of array 1 place backwards
          for (int j = i; j < nfds; j++) { fds[j].fd = fds[j + 1].fd; }
          // go back 1 index becuase we removed a connection
          i--;
          nfds--;
        }
      }
    }

  } while (endServer == 0);

  for (int i = 0; i < nfds; i++)
  {
    if (fds[i].fd >= 0) { close(fds[i].fd); }
  }
}
