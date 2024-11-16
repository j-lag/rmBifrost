
#ifndef PACKET_H
#define PACKET_H

#include "../../utils/data_structs.h"

class packet {
    virtual void poly() = 0;
};

template<class Archive>
void serialize(Archive& archive, point& point) {
    archive(point.x, point.y);
}

template<class Archive>
void serialize(Archive& archive, rect& rect) {
    archive(rect.p1, rect.p2);
}


#endif //PACKET_H
