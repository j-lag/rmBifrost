
#ifndef SUBMIT_FRAME_H
#define SUBMIT_FRAME_H

#include "packet.h"
#include "../../constants.h"

#include <cereal/types/polymorphic.hpp>
#include <cstdint>

struct submit_frame_packet : packet {
    uint32_t framebuffer_id;
    rect dirty_rect;
    refresh_type preferred_refresh_type;

    template<class Archive>
    void serialize(Archive& archive) {
        archive(framebuffer_id, dirty_rect, preferred_refresh_type);
    }

    void poly() override {};
};

#include <cereal/archives/binary.hpp>

CEREAL_REGISTER_TYPE(submit_frame_packet);

CEREAL_REGISTER_POLYMORPHIC_RELATION(packet, submit_frame_packet);

#endif //SUBMIT_FRAME_H
