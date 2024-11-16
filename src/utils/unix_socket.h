
#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <memory>
#include <sstream>

class unix_socket {
public:
    class connection {
    public:
        explicit connection(int fd);
        void read(char* buf, size_t size) const;
        void write(const char* data, size_t size);
        void close();
    private:
        int fd;
    };

    explicit unix_socket(const std::string& path, bool server = false);
    [[nodiscard]] std::unique_ptr<connection> accept_connection() const;

    std::shared_ptr<connection> get_connection() const;
private:
    std::string path;
    bool server;
    std::shared_ptr<connection> server_connection;
    int fd;
};



#endif //UNIX_SOCKET_H
