#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
using namespace std;

class Client {
  int clientDescriptor;
  struct sockaddr_in serverAddress;

public:
  Client(const string &serverIP, int serverPort);

  void start();
  string getCommandFromUser();
  void sendCommandToServer(const string &command);
  string getResponseFromServer();

  ~Client();
};
