
#ifndef RESOURCES_H
#define RESOURCES_H
#include <string>
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(bifrost);

inline std::vector<uint8_t> get_resource_file(const std::string& filename)
{
    auto fs = cmrc::bifrost::get_filesystem();
    auto file = fs.open(filename);
    return {file.begin(), file.end()};
}
#endif //RESOURCES_H
