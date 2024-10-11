#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>
using namespace std;

class Server {
  int serverDescriptor;
  struct sockaddr_in serverAddress;
  vector<thread> clientThreads;
  atomic<bool> isRunning;
  mutex output_mutex;
  mutex file_mutex;
  mutex clientThreads_mutex;

public:
  Server(int port);
  void start();
  void handleClient(int clientDescriptor);
  void parseMessage(string &message, int clientDescriptor, bool *isRunning);
  string getFileContents(const string &filePath);
  void changeDirectory(const string &path, int clientDescriptor);
  int getServerDescriptor() const { return serverDescriptor; }
  string getCurrentTime() const;
  void stop() { isRunning = false; }
  string listDirectory(const string &path, const string &prefix);
  void writeToSocket(int clientDescriptor, const string &message);
  string readFromSocket(int clientDescriptor);
  void handleFileCommands(const string &command, int clientDescriptor, bool *isRunning);
  ~Server();
};

