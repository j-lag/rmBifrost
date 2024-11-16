#include "unix_socket.h"
#include <spdlog/spdlog.h>


unix_socket::unix_socket(const std::string& path, bool server)
    : path(path), server(server)
{
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

    if (server) {
        unlink(path.c_str());
        if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            close(fd);
            throw std::runtime_error("Failed to bind socket");
        }

        if (listen(fd, 5) == -1) {
            close(fd);
            throw std::runtime_error("Failed to listen on socket");
        }
    } else {
        if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            close(fd);
            throw std::runtime_error("Failed to connect to server");
        }

        server_connection = std::make_shared<connection>(fd);
    }
}

std::unique_ptr<unix_socket::connection> unix_socket::accept_connection() const
{
    int new_fd = accept(fd, nullptr, nullptr);
    if (new_fd == -1) {
        throw std::runtime_error("Failed to accept connection");
    }
    return std::make_unique<connection>(new_fd);
}

unix_socket::connection::connection(int fd)
    : fd(fd)
{

}

void unix_socket::connection::read(char* buf, const size_t size) const
{
    size_t total_read = 0;
    while (total_read < size) {
        const ssize_t read_bytes = recv(fd, buf + total_read, size - total_read, 0);
        if (read_bytes == -1) {
            // check if the error is EAGAIN
            if (errno == EAGAIN) {
                continue;
            }

            spdlog::error("Failed to read from socket: {}", strerror(errno));
            throw std::runtime_error("Failed to read from socket");
        }
        if (read_bytes == 0) {
            spdlog::debug("Client disconnected (EOF)");
            throw std::runtime_error("Connection closed by peer");
        }
        total_read += read_bytes;
    }
}
void unix_socket::connection::write(const char* data, size_t size)
{
    size_t total_written = 0;
    while (total_written < size) {
        // Add MSG_NOSIGNAL flag to prevent SIGPIPE
        const ssize_t written_bytes = send(fd, data + total_written, size - total_written, MSG_NOSIGNAL);
        if (written_bytes == -1) {
            if (errno == EAGAIN) {
                continue;
            }
            if (errno == EPIPE) {
                spdlog::debug("Client disconnected (EPIPE)");
                throw std::runtime_error("Connection closed by peer");
            }
            spdlog::error("Failed to write to socket: {}", strerror(errno));
            throw std::runtime_error("Failed to write to socket");
        }
        total_written += written_bytes;
    }
}

void unix_socket::connection::close() {
    ::shutdown(fd, SHUT_RDWR);
}

std::shared_ptr<unix_socket::connection> unix_socket::get_connection() const
{
    if (server) {
        throw std::runtime_error("Server socket does not have a connection");
    }
    return server_connection;
}
