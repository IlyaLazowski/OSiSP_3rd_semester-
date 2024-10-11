#include "Client.h"
#include <sys/types.h>
using namespace std;

Client::Client(const string &serverIP, int serverPort) {
  clientDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (clientDescriptor == -1) {
    cerr << "Error creating socket" << endl;
    exit(EXIT_FAILURE);
  }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(serverPort);
  if (inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr) <= 0) {
    cerr << "Invalid server IP address" << endl;
    exit(EXIT_FAILURE);
  }

  if (connect(clientDescriptor, (struct sockaddr *)&serverAddress,
              sizeof(serverAddress)) < 0) {
    cerr << "Error connecting to server" << endl;
    exit(EXIT_FAILURE);
  }
}

void Client::start() {
  while (true) {
    string command = getCommandFromUser();
    sendCommandToServer(command);
    string response = getResponseFromServer();
    cout << response << endl;
  }
}

string Client::getCommandFromUser() {
  cout << "> ";
  string command;
  getline(std::cin, command);
  return command;
}

void Client::sendCommandToServer(const string &command) {
  uint32_t commandSize = htonl(command.size());
  if (write(clientDescriptor, &commandSize, sizeof(commandSize)) == -1) {
    cerr << "Error sending command size" << endl;
    close(clientDescriptor);
    exit(EXIT_FAILURE);
  }

  if (write(clientDescriptor, command.c_str(), command.size()) == -1) {
    cerr << "Error sending command" << endl;
    close(clientDescriptor);
    exit(EXIT_FAILURE);
  }
}

string Client::getResponseFromServer() {
  uint32_t responseSize;
  if (read(clientDescriptor, &responseSize, sizeof(responseSize)) == -1) {
    cerr << "Error receiving response size. Server may have closed."
              << endl;
    close(clientDescriptor);
    exit(EXIT_FAILURE);
  }
  responseSize = ntohl(responseSize);

  char *buffer = new char[responseSize + 1];
  ssize_t bytesRead = read(clientDescriptor, buffer, responseSize);
  if (bytesRead == -1 || bytesRead == 0 ) {
    cerr << "Server closed connection." << endl;
    delete[] buffer;
    close(clientDescriptor);
    exit(EXIT_SUCCESS);
  }

  buffer[responseSize] = '\0';
  string response(buffer);
  delete[] buffer;
  return response;
}

Client::~Client() { close(clientDescriptor); }
