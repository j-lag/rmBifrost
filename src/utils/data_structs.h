#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#include <cstdint>
#include <algorithm>

struct point {
    uint32_t x;
    uint32_t y;

    point operator+(const point& other) const {
        return { .x = x + other.x, .y = y + other.y };
    }

    point operator-(const point& other) const {
        return { .x = x - other.x, .y = y - other.y };
    }

    bool operator==(const point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const point& other) const {
        return !(*this == other);
    }

    bool operator<(const point& other) const {
        return x < other.x || (x == other.x && y < other.y);
    }

    bool operator>(const point& other) const {
        return x > other.x || (x == other.x && y > other.y);
    }

    point operator+=(const point& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    point operator-=(const point& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
};

using extent = point;

struct rect {
    point p1;
    point p2;

    bool operator==(const rect& other) const {
        return p1 == other.p1 && p2 == other.p2;
    }

    bool operator!=(const rect& other) const {
        return !(*this == other);
    }

    bool contains(const point& p) const {
        return p.x >= p1.x && p.x <= p2.x && p.y >= p1.y && p.y <= p2.y;
    }

    bool intersects(const rect& other) const {
        return p1.x <= other.p2.x && p2.x >= other.p1.x && p1.y <= other.p2.y && p2.y >= other.p1.y;
    }

    rect intersection(const rect& other) const {
        return { .p1 = { std::max(p1.x, other.p1.x), std::max(p1.y, other.p1.y) }, .p2 = { std::min(p2.x, other.p2.x), std::min(p2.y, other.p2.y) } };
    }

    rect union_(const rect& other) const {
        return { .p1 = { std::min(p1.x, other.p1.x), std::min(p1.y, other.p1.y) }, .p2 = { std::max(p2.x, other.p2.x), std::max(p2.y, other.p2.y) } };
    }

    uint32_t width() const {
        return p2.x - p1.x;
    }

    uint32_t height() const {
        return p2.y - p1.y;
    }
};

#endif // DATA_STRUCTS_H