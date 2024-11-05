#include "ComicArchive.h"
#include <algorithm>
#include "ImageIo.h"

inline static void ToLower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
}

ComicArchive* ComicArchive::Create(const char* filename)
{
    ComicArchive* archive = nullptr;
    //get lower case extention
    std::string extension = filename; 
    extension = extension.substr(extension.find_last_of("."));
    ToLower(extension);

    if (extension == ".cbz") 
    {
        archive = new CBZComicArchive(filename);
    }
    else if (extension == ".cbr")
    {
    }

    return archive;

}



CBZComicArchive::CBZComicArchive(const char* archive_name)
{
    memset(&zip_archive, 0, sizeof(zip_archive));

    spdlog::debug("Opening archive: {}", archive_name);
    if (!mz_zip_reader_init_file(&zip_archive, archive_name, 0)) {
        spdlog::error("Failed to open archive: {}", archive_name);
        return;
    }

    // Iterate over all files in the archive
    int file_count = mz_zip_reader_get_num_files(&zip_archive);
    spdlog::debug("Found {} files in the archive", file_count);
    for (int i = 0; i < file_count; ++i) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) continue;

        // Check if it's a JPEG or PNG
        std::string filename = file_stat.m_filename; 
        ToLower(filename);
        if (filename.find(".jpg") != std::string::npos || filename.find(".jpeg") != std::string::npos || filename.find(".png") != std::string::npos) 
        {
            spdlog::debug("Found image file: {}", filename);
            image_files.push_back(file_stat);
        }
    }

    // Sort images lexicographically
    if (image_files.empty()) {
        spdlog::error("No JPEG or PNG images found in the archive.");
        mz_zip_reader_end(&zip_archive);
        return;
    }

    std::sort(image_files.begin(), image_files.end(), [](const mz_zip_archive_file_stat& a, const mz_zip_archive_file_stat& b) {
        return std::string(a.m_filename) < std::string(b.m_filename);
        });
    spdlog::debug("Sorted image files lexicographically");
}


CBZComicArchive::~CBZComicArchive()
{
    mz_zip_reader_end(&zip_archive);
}

uint32_t CBZComicArchive::GetImageCount()
{
    return image_files.size();
}

lv_img_dsc_t* CBZComicArchive::GetImage(uint32_t id, uint32_t resized_height)
{
    lv_img_dsc_t* img_dsc = nullptr;

    if (id < GetImageCount())
    {
        mz_zip_archive_file_stat& first_image_file = image_files[id];
        spdlog::debug("Extracting image: {}", first_image_file.m_filename);
        size_t uncompressed_size;
        void* image_data = mz_zip_reader_extract_to_heap(&zip_archive, first_image_file.m_file_index, &uncompressed_size, 0);
        if (image_data) 
        {
            //image data
            unsigned char*  pixels = nullptr;
            int             w;
            int             h;
            bool            isrgb;
            // Decode and resize the image to 128 pixels in height
            std::string extension = first_image_file.m_filename; 
            extension = extension.substr(extension.find_last_of("."));
            ToLower(extension);

            if (extension == ".png") {
                pixels =  load_png_from_memory((const unsigned char*)image_data, uncompressed_size, w, h, isrgb);
            }
            else if (extension == ".jpg" || extension == ".jpeg") {
                pixels = load_jpeg_from_memory((const unsigned char*)image_data, uncompressed_size, w, h, isrgb);
            }
            mz_free(image_data); // Free the image data from the archive

            if (pixels != nullptr)
            {
                // Resize the image to a height of "resized_height" pixels, maintaining aspect ratio
                int resized_width;
                spdlog::debug("Resizing image to target height {} pixels", resized_height);
                //save_image_as_tga((const char*)rgb_buf, width, height, is_rgb, "/home/root/in.tga");
                unsigned char* resized_image = resize_image(pixels, w, h, height, resized_width, is_rgb);
                delete[] pixels;
                //save_image_as_tga((const char*)resized_image, resized_width, 128, is_rgb, "/home/root/out.tga");

                // Create LVGL image descriptor
                spdlog::debug("Creating LVGL image descriptor");
                img_dsc = new lv_img_dsc_t;
                img_dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
                img_dsc->header.flags = LV_IMAGE_FLAGS_ALLOCATED;
                img_dsc->header.w = resized_width;
                img_dsc->header.h = resized_height;
                img_dsc->header.stride = resized_width * (is_rgb ? 3 : 1);
                img_dsc->header.cf = is_rgb ? LV_COLOR_FORMAT_RGB888 : LV_COLOR_FORMAT_L8;
                img_dsc->data = resized_image;  // Transfer ownership
                img_dsc->data_size = resized_width * resized_height * (is_rgb ? 3 : 1);
                spdlog::debug("LVGL image descriptor created: width={}, height={}, color format={}", (uint32_t)img_dsc->header.w, (uint32_t)img_dsc->header.h, is_rgb);
            }
            else
            {
                spdlog::error("Failed to load/resize image: {}", first_image_file.m_filename);
            }
        }
        else
        {
            spdlog::error("Failed to extract image: {}", first_image_file.m_filename);
        }
    }
    else
    {
        spdlog::debug("invalid image id: {} / {}", id, GetImageCount());
    }
    return(img_dsc);
}