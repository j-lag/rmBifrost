
#ifndef BEGIN_SESSION_REQUEST_H
#define BEGIN_SESSION_REQUEST_H

#include "packet.h"

#include <cereal/types/polymorphic.hpp>

class begin_session_request : public packet {
public:
    std::string application_name;
    std::string window_title;
    bool prefer_full_screen;
    uint8_t swapchain_image_count;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(application_name, window_title, prefer_full_screen, swapchain_image_count);
    }

    void poly() override {};
};

#include <cereal/archives/binary.hpp>

CEREAL_REGISTER_TYPE(begin_session_request);

CEREAL_REGISTER_POLYMORPHIC_RELATION(packet, begin_session_request);

#endif //BEGIN_SESSION_REQUEST_H
