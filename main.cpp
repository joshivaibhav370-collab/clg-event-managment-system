// main.cpp - A tiny web server.
// The browser opens http://localhost:8080
//   /api/...   -> handled by CampusConnectSystem (C++), answers in JSON
//   anything else -> a file from the "web" folder (HTML, CSS, JS)

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "CampusConnect.h"

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <netinet/in.h>
    #include <unistd.h>
    typedef int SOCKET;
    #define closesocket close
#endif

const int PORT = 8080;

// "Tech%20Fest" or "Tech+Fest" -> "Tech Fest"
std::string urlDecode(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') {
            out += ' ';
        } else if (s[i] == '%' && i + 2 < s.size()) {
            out += (char) std::strtol(s.substr(i + 1, 2).c_str(), nullptr, 16);
            i += 2;
        } else {
            out += s[i];
        }
    }
    return out;
}

// getParam("id=E101&name=Quiz", "name") -> "Quiz"
std::string getParam(const std::string& query, const std::string& key) {
    std::stringstream ss(query);
    std::string pair;
    while (std::getline(ss, pair, '&')) {
        size_t eq = pair.find('=');
        if (eq != std::string::npos && pair.substr(0, eq) == key) {
            return urlDecode(pair.substr(eq + 1));
        }
    }
    return "";
}

bool endsWith(const std::string& s, const std::string& end) {
    return s.size() >= end.size() && s.compare(s.size() - end.size(), end.size(), end) == 0;
}

std::string contentType(const std::string& path) {
    if (endsWith(path, ".html")) return "text/html; charset=utf-8";
    if (endsWith(path, ".css"))  return "text/css";
    if (endsWith(path, ".js"))   return "application/javascript";
    if (endsWith(path, ".png"))  return "image/png";
    if (endsWith(path, ".jpg"))  return "image/jpeg";
    return "text/plain";
}

bool readFile(const std::string& path, std::string& data) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::stringstream buffer;
    buffer << file.rdbuf();
    data = buffer.str();
    return true;
}

void sendResponse(SOCKET client, const std::string& status, const std::string& type, const std::string& body) {
    std::string response =
        "HTTP/1.1 " + status + "\r\n" +
        "Content-Type: " + type + "\r\n" +
        "Content-Length: " + std::to_string(body.size()) + "\r\n" +
        "Cache-Control: no-store\r\n" +
        "Connection: close\r\n\r\n" + body;

    size_t sent = 0;
    while (sent < response.size()) {
        int n = send(client, response.c_str() + sent, (int)(response.size() - sent), 0);
        if (n <= 0) break;
        sent += n;
    }
}

// Don't let an idle browser connection freeze the server forever
void setReadTimeout(SOCKET s) {
#ifdef _WIN32
    DWORD ms = 1000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&ms, sizeof(ms));
#else
    timeval tv{1, 0};
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
}

// All the API routes the HTML pages use
std::string handleApi(CampusConnectSystem& sys, const std::string& path, const std::string& q) {
    if (path == "/api/events")        return sys.eventsJson();
    if (path == "/api/students")      return sys.studentsJson();
    if (path == "/api/registrations") return sys.registrationsJson();
    if (path == "/api/waiting")       return sys.waitingJson();
    if (path == "/api/stats")         return sys.statsJson();
    if (path == "/api/undo")          return sys.undoLastAction();

    if (path == "/api/register") return sys.registerStudent(getParam(q, "student"), getParam(q, "event"));
    if (path == "/api/cancel")   return sys.cancelRegistration(getParam(q, "student"), getParam(q, "event"));

    if (path == "/api/addEvent") {
        std::string id = getParam(q, "id"), name = getParam(q, "name");
        std::string date = getParam(q, "date"), venue = getParam(q, "venue");
        int capacity = std::atoi(getParam(q, "capacity").c_str());
        if (id.empty() || name.empty() || capacity <= 0) {
            return jsonResult(false, "ID, name and a positive capacity are required.");
        }
        if (!sys.addEvent(id, name, date, venue, capacity)) {
            return jsonResult(false, "Event ID " + id + " already exists.");
        }
        return jsonResult(true, "Event " + id + " added.");
    }

    if (path == "/api/addStudent") {
        std::string id = getParam(q, "id"), name = getParam(q, "name"), dept = getParam(q, "dept");
        int semester = std::atoi(getParam(q, "semester").c_str());
        if (id.empty() || name.empty() || semester <= 0) {
            return jsonResult(false, "ID, name and a valid semester are required.");
        }
        if (!sys.addStudent(id, name, dept, semester)) {
            return jsonResult(false, "Student ID " + id + " already exists.");
        }
        return jsonResult(true, "Student " + id + " added.");
    }

    return jsonResult(false, "Unknown API: " + path);
}

int main() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    CampusConnectSystem system;

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   // only this computer can connect
    addr.sin_port = htons(PORT);

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cout << "Error: port " << PORT << " is already in use. Close the other program and retry.\n";
        return 1;
    }
    listen(server, 10);

    std::cout << "=============================================\n";
    std::cout << " CampusConnect server is running\n";
    std::cout << " Open in browser: http://localhost:" << PORT << "\n";
    std::cout << " Press Ctrl+C to stop\n";
    std::cout << "=============================================\n";

    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == (SOCKET)-1) continue;
        setReadTimeout(client);

        char buffer[4096];
        int n = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            closesocket(client);
            continue;
        }

        // First line looks like: GET /api/events?x=1 HTTP/1.1
        std::stringstream firstLine(std::string(buffer, n));
        std::string method, url;
        firstLine >> method >> url;

        std::string path = url, query;
        size_t qPos = url.find('?');
        if (qPos != std::string::npos) {
            path = url.substr(0, qPos);
            query = url.substr(qPos + 1);
        }

        if (path.rfind("/api/", 0) == 0) {
            std::cout << "API  " << path << "\n";
            sendResponse(client, "200 OK", "application/json", handleApi(system, path, query));
        } else {
            if (path == "/") path = "/index.html";
            std::string data;
            if (path.find("..") == std::string::npos && readFile("web" + path, data)) {
                sendResponse(client, "200 OK", contentType(path), data);
            } else {
                sendResponse(client, "404 Not Found", "text/plain", "404 - File not found: " + path);
            }
        }
        closesocket(client);
    }

    return 0;
}
