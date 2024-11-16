#ifndef COMIC_ARCHIVE_H
#define COMIC_ARCHIVE_H
#include <vector>
#include "lvgl_renderer.h"
#include "ImageIo.h"

class ComicArchive {
public:
    static ComicArchive* Create(const char* filename);

    virtual ~ComicArchive() = default;
    virtual uint32_t GetImageCount() = 0;
    virtual bool GetImage(uint32_t id, uint32_t height, Image& image) = 0;

protected:
    ComicArchive() = default;
};


#endif //COMIC_ARCHIVE
