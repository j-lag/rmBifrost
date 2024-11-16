#include "shm_channel.h"

#include <cstring>
#include <spdlog/spdlog.h>

shm_channel::shm_channel(const std::string& identifier, int size, bool read_only)
    : identifier("/bifrost_" + identifier)
    , read_only(read_only)
{
    fd = shm_open(identifier.c_str(), O_CREAT | (!read_only ? O_RDWR : O_RDONLY), 0666);
    if (fd == -1) {
        throw std::runtime_error("shm_open failed");
    }

    if (size != -1 && ftruncate(fd, size) == -1) {
        throw std::runtime_error("ftruncate failed");
    }

    data = mmap(nullptr, size, !read_only ? PROT_WRITE | PROT_READ : PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        spdlog::error("mmap failed: {}", strerror(errno));
        throw std::runtime_error("mmap failed");
    }
}