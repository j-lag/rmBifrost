
#ifndef BEGIN_SESSION_RESPONSE_H
#define BEGIN_SESSION_RESPONSE_H

#include "packet.h"

#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <vector>

class begin_session_response : public packet {
public:
    uint32_t swapchain_image_count;
    std::string session_name;
    size_t shared_memory_size;
    std::vector<uint64_t> swapchain_image_offsets;
    extent swapchain_extent;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(swapchain_image_count, session_name, shared_memory_size, swapchain_image_offsets, swapchain_extent);
    }

    void poly() override {};
};

#include <cereal/archives/binary.hpp>

CEREAL_REGISTER_TYPE(begin_session_response);

CEREAL_REGISTER_POLYMORPHIC_RELATION(packet, begin_session_response);

#endif //BEGIN_SESSION_RESPONSE_H
