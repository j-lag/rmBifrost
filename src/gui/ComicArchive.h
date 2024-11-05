#ifndef COMIC_ARCHIVE_H
#define COMIC_ARCHIVE_H

#include <vector>
#include "lvgl_renderer.h"
#include "miniz.h"

class ComicArchive{
public:
	static ComicArchive* Create(const char* filename);
	virtual uint32_t GetImageCount() = 0;
	virtual lv_img_dsc_t* GetImage(uint32_t id, uint32_t height) = 0;
};


class CBZComicArchive : public ComicArchive {
public:
	CBZComicArchive(const char* archive_name);
	~CBZComicArchive();
	uint32_t GetImageCount() override;
	lv_img_dsc_t* GetImage(uint32_t id, uint32_t height) override;
private:
	mz_zip_archive zip_archive;
	std::vector<mz_zip_archive_file_stat> image_files;
};


#endif //COMIC_ARCHIVE
