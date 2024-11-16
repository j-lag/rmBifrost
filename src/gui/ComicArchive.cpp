#include "ComicArchive.h"
#include <archive.h>
#include <archive_entry.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <spdlog/spdlog.h>
#include "lvgl.h"
#include "ImageIo.h"

// Concrete implementation of ComicArchive
class ComicArchiveImpl : public ComicArchive {
public:
    ComicArchiveImpl(const char* archive_name);
    ~ComicArchiveImpl() override;

    uint32_t GetImageCount() override;
    bool GetImage(uint32_t id, uint32_t height, Image& image) override;

private:
    std::string archive_name;
    std::vector<std::pair<std::string, size_t> > image_files;

    bool loadImageFromArchive(uint32_t id, Image& image);
};

ComicArchive* ComicArchive::Create(const char* filename) {
    // Use libarchive to determine the type
    struct archive* arch = archive_read_new();
    archive_read_support_format_all(arch);
    archive_read_support_filter_all(arch);

    if (archive_read_open_filename(arch, filename, 10240) != ARCHIVE_OK) {
        spdlog::error("Failed to open archive: {}", filename);
        archive_read_free(arch);
        return nullptr;
    }

    struct archive_entry* entry;
    if (archive_read_next_header(arch, &entry) == ARCHIVE_OK) {
        spdlog::info("Opened archive: {}", filename);
        archive_read_close(arch);
        archive_read_free(arch);
        return new ComicArchiveImpl(filename);
    }

    spdlog::error("Invalid archive format: {}", filename);
    archive_read_close(arch);
    archive_read_free(arch);
    return nullptr;
}

ComicArchiveImpl::ComicArchiveImpl(const char* archive_name) : archive_name(archive_name) {
    struct archive* arch = archive_read_new();
    archive_read_support_format_all(arch);
    archive_read_support_filter_all(arch);

    if (archive_read_open_filename(arch, archive_name, 10240) != ARCHIVE_OK) {
        spdlog::error("Failed to open archive: {}", archive_name);
        archive_read_free(arch);
        return;
    }

    struct archive_entry* entry;
    size_t entryId = 1;
    while (archive_read_next_header(arch, &entry) == ARCHIVE_OK) 
    {
        const char* filename = archive_entry_pathname(entry);
        std::string fname(filename ? filename : "");

        std::transform(fname.begin(), fname.end(), fname.begin(), ::tolower);

        if (fname.find(".jpg") != std::string::npos || fname.find(".jpeg") != std::string::npos || fname.find(".png") != std::string::npos) {
            image_files.push_back(std::make_pair(fname, entryId));
        }
        archive_read_data_skip(arch);
        ++entryId;
    }

    archive_read_close(arch);
    archive_read_free(arch);

    std::sort(image_files.begin(), image_files.end(), [](const std::pair<std::string, size_t>& a, const std::pair<std::string, size_t>& b) {
        return a.first < b.first;
        });
    spdlog::debug("Found and sorted {} image files in archive.", image_files.size());
}

ComicArchiveImpl::~ComicArchiveImpl() {
    // No explicit cleanup needed; libarchive handles resources internally.
}

uint32_t ComicArchiveImpl::GetImageCount() {
    return image_files.size();
}

bool ComicArchiveImpl::GetImage(uint32_t id, uint32_t resized_height, Image& image) {
    if (id >= GetImageCount()) {
        spdlog::error("Invalid image ID: {} / {}", id, GetImageCount());
        return false;
    }

    if (!loadImageFromArchive(id, image)) {
        spdlog::error("Failed to load image with ID {}", id);
        return false;
    }

    // Resize the image if needed
    if (resized_height != 0)
    {
        // Resize the image to a height of "resized_height" pixels, maintaining aspect ratio
        int resized_width;
        spdlog::debug("Resizing image to target height {} pixels", resized_height);
        //save_image_as_tga((const char*)rgb_buf, width, height, is_rgb, "/home/root/in.tga");
        char* resized_image = resize_image_lanczos(image.data, image.width, image.height, resized_height, resized_width, image.is_rgb);
        delete[] image.data;
        image.data = resized_image;
        image.width = resized_width;
        image.height = resized_height;
    }
    return true;
}

bool ComicArchiveImpl::loadImageFromArchive(uint32_t id, Image& image) {
    struct archive* arch = archive_read_new();
    archive_read_support_format_all(arch);
    archive_read_support_filter_all(arch);

    if (archive_read_open_filename(arch, archive_name.c_str(), 10240) != ARCHIVE_OK) {
        spdlog::error("Failed to open archive: {}", archive_name);
        archive_read_free(arch);
        return false;
    }
    
    struct archive_entry* entry;
    void* buffer = nullptr;
    size_t buffer_size = 0;
    bool success = false;

    //skip to file
    for (size_t i = 0; i < image_files[id].second; ++i)
        archive_read_next_header(arch, &entry);

    //read it   
    const char* filename = archive_entry_pathname(entry);
    spdlog::error("check: {} > {}", filename, image_files[id].first);
    buffer_size = archive_entry_size(entry);
    buffer = malloc(buffer_size);
    if (buffer) 
    {
        spdlog::error("allocated");
        ssize_t retcode = archive_read_data(arch, buffer, buffer_size);
        spdlog::error("retcode {}", retcode);
        if (retcode > 0) {
            spdlog::error("loaded");
            std::string ext = image_files[id].first.substr(image_files[id].first.find_last_of("."));
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            spdlog::error("extention:  {}", ext);
            if (ext == ".png") {
                success = load_png_from_memory((const unsigned char*)buffer, buffer_size, image);
            }
            else if (ext == ".jpg" || ext == ".jpeg") {
                success = load_jpeg_from_memory((const unsigned char*)buffer, buffer_size, image);
            }
        }
        free(buffer);
    }
    
    archive_read_close(arch);
    archive_read_free(arch);
    return success;
}