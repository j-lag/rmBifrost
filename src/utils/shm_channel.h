
#ifndef SHM_CHANNEL_H
#define SHM_CHANNEL_H
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include "../constants.h"

class shm_channel {
private:
    std::string identifier;

public:
    shm_channel(const std::string& identifier, int size, bool read_only);
    void *data = nullptr;

private:
    bool read_only;
    int fd;
};

#endif // SHM_CHANNEL_H
