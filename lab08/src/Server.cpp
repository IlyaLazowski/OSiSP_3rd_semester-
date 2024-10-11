#include "Server.h"
#include <filesystem>
#include <mutex>
#include <unistd.h>
using namespace std;

Server::Server(int port) {
  serverDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (serverDescriptor == -1) {
    lock_guard<mutex> lock(output_mutex);
    cerr << "Error creating socket" << endl;
    exit(EXIT_FAILURE);
  }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverDescriptor, (struct sockaddr *)&serverAddress,
           sizeof(serverAddress)) == -1) {
    lock_guard<mutex> lock(output_mutex);
    cerr << "Error binding socket" << endl;
    exit(EXIT_FAILURE);
  }

  if (listen(serverDescriptor, 5) == -1) {
    lock_guard<mutex> lock(output_mutex);
    cerr << "Error listening on socket" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Server is ready." << endl;
}

void Server::start() {
  if (fcntl(serverDescriptor, F_SETFL, O_NONBLOCK) < 0) {
    lock_guard<mutex> lock(output_mutex);
    cerr << "Error setting socket to non-blocking mode" << endl;
    exit(EXIT_FAILURE);
  }
  while (isRunning) {
    int client_descriptor = accept(serverDescriptor, NULL, NULL);
    if (client_descriptor == -1) {
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        usleep(1000);
        continue;
      } else {
        lock_guard<mutex> lock(output_mutex);
        cerr << "Error accepting connection" << endl;
        exit(EXIT_FAILURE);
      }
    }
    {
      lock_guard<mutex> lock(output_mutex);
      cout << getCurrentTime() << " Client connected" << endl;
    }

    thread client_thread(
        [this](int client_descriptor) {
          this->handleClient(client_descriptor);
        },
        client_descriptor);
    lock_guard<mutex> lock(clientThreads_mutex);
    clientThreads.push_back(move(client_thread));
  }
}

void Server::handleClient(int clientDescriptor) {
  bool isRunning = true;
  while (isRunning) {
    string message = readFromSocket(clientDescriptor);
    if (message.empty()) {
      break;
    }

    parseMessage(message, clientDescriptor, &isRunning);
  }
}

void Server::parseMessage(string &message, int clientDescriptor, bool *isRunning) {
  string response;
  if (message.substr(0, 4) == "ECHO") {
    response = message.substr(5);
  } else if (message.substr(0, 4) == "QUIT") {
    lock_guard<mutex> lock(output_mutex);
    cout << getCurrentTime() << " Client disconnected" << endl;
    *isRunning = false;
    close(clientDescriptor);
    return;
  } else if (message.substr(0, 4) == "INFO") {
    response = getFileContents("/root/Рабочий стол/lab08/src/serverInfo.txt");
  } else if (message.substr(0, 2) == "CD") {
    changeDirectory(message.substr(3), clientDescriptor);
    return;
  } else if (message.substr(0, 4) == "LIST") {

    response = listDirectory(".", "");
  } else if (message[0] == '@') {
    handleFileCommands(message.substr(1), clientDescriptor, isRunning);
    return;
  } else {
    response = "Invalid command";
  }

  writeToSocket(clientDescriptor, response);
}

string Server::listDirectory(const string &path,
                                  const string &prefix) {
  string result;
  for (const auto &entry : filesystem::directory_iterator(path)) {
    if (entry.is_directory()) {
      result += prefix + entry.path().filename().string() + "/\n";
      result += listDirectory(entry.path().string(), prefix + "  ");
    } else if (entry.is_symlink()) {
      auto target = filesystem::read_symlink(entry.path());
      result += prefix + entry.path().filename().string() + " --> ";
      if (filesystem::is_regular_file(target)) {
        result += " " + target.filename().string() + "\n";
      } else if (filesystem::is_symlink(target)) {
        result += "> " + target.filename().string() + "\n";
      }
    } else if (entry.is_regular_file()) {
      result += prefix + entry.path().filename().string() + "\n";
    }
  }
  return result;
}

void Server::handleFileCommands(const string &filename, int clientDescriptor, bool *isRunning) {
  ifstream file(filename);
  if (!file.is_open()) {
    string response = "Error opening file";
    writeToSocket(clientDescriptor, response);
  } else {
    string line;
    while (getline(file, line)) {
      parseMessage(line, clientDescriptor, isRunning);
      if (!*isRunning) {
        break;
      }
    }
  }
}

string Server::getFileContents(const string &filePath) {
  lock_guard<mutex> lock(file_mutex);
  ifstream file(filePath);
  if (!file.is_open()) {
    return "Error opening file";
  }
  stringstream buffer;
  buffer << file.rdbuf();
  file.close();
  return buffer.str();
}

void Server::changeDirectory(const string &path, int clientDescriptor) {
  string response;
  filesystem::path rootPath = "/root/Рабочий стол/lab08"; // Замените на ваш корневой каталог

  if (path.empty()) {
    response = "Invalid path";
  } else {
    filesystem::path fsPath(path);
    if (!filesystem::exists(fsPath) ||
        !filesystem::is_directory(fsPath)) {
      response = "Path is not a directory";
    } else {
      // Получаем относительный путь от корневого каталога до нового пути
      filesystem::path relativePath = filesystem::relative(fsPath, rootPath);

      // Проверяем, содержит ли относительный путь ".."
      if (relativePath.string().find("..") != string::npos) {
        response = "Cannot change directory outside the root directory";
      } else {
        error_code ec;
        filesystem::current_path(fsPath, ec);
        if (ec) {
          response = "Error changing directory";
        } else {
          response = "Directory changed";
        }
      }
    }
  }

  writeToSocket(clientDescriptor, response);
}


string Server::getCurrentTime() const {
  auto now = chrono::system_clock::now();
  auto now_c = chrono::system_clock::to_time_t(now);
  char buffer[100];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
                localtime(&now_c));
  return string(buffer);
}

void Server::writeToSocket(int clientDescriptor, const string &message) {
  uint32_t responseSize = htonl(message.size());
  if (write(clientDescriptor, &responseSize, sizeof(responseSize)) == -1) {
    lock_guard<mutex> lock(output_mutex);
    cerr << "Error sending response size" << endl;
    return;
  }

  if (write(clientDescriptor, message.c_str(), message.size()) == -1) {
    lock_guard<mutex> lock(output_mutex);
    cerr << "Error sending response" << endl;
  }
}

string Server::readFromSocket(int clientDescriptor) {
  uint32_t commandSize;
  if (read(clientDescriptor, &commandSize, sizeof(commandSize)) == -1) {
    lock_guard<mutex> lock(output_mutex);
    cerr << "Error reading command size from client" << endl;
    return "";
  }
  commandSize = ntohl(commandSize);

  char *buffer = new char[commandSize + 1];
  int bytesRead = read(clientDescriptor, buffer, commandSize);
  if (bytesRead == -1) {
    lock_guard<mutex> lock(output_mutex);
    cerr << "Error reading command from client" << endl;
    delete[] buffer;
    return "";
  }
  buffer[commandSize] = '\0';

  string message(buffer, commandSize);
  delete[] buffer;

  return message;
}

Server::~Server() {
  lock_guard<mutex> lock(clientThreads_mutex);
  for (auto &thread : clientThreads) if (thread.joinable()) thread.join();
  close(serverDescriptor);
}

