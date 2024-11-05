#include "ImageIo.h"
#include <iostream>
#include <png.h>
#include <turbojpeg.h>

bool save_image_as_tga(const char* data, int width, int height, bool is_rgb, const char* filename)
{
    // Ouvre le fichier en mode binaire
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        spdlog::debug("Erreur: Impossible d'ouvrir le fichier {} pour l'écriture.", filename);
        return false;
    }

    // Taille d'un pixel en octets (3 pour RGB, 1 pour grayscale)
    int pixel_size = is_rgb ? 3 : 1;

    // Prépare l'en-tête TGA
    unsigned char header[18];
    std::memset(header, 0, sizeof(header));  // Initialise tout l'en-tête à 0
    header[2] = 2;  // Type d'image: 2 pour une image non compressée en Truecolor
    header[12] = width & 0xFF;        // Largeur (octet bas)
    header[13] = (width >> 8) & 0xFF; // Largeur (octet haut)
    header[14] = height & 0xFF;       // Hauteur (octet bas)
    header[15] = (height >> 8) & 0xFF; // Hauteur (octet haut)
    header[16] = pixel_size * 8;      // Profondeur de couleur: 24 bits pour RGB, 8 bits pour grayscale

    // Écrit l'en-tête dans le fichier
    file.write(reinterpret_cast<const char*>(header), sizeof(header));

    // Écrit les données d'image dans le fichier
    // Note : Les données TGA sont en ordre BGR pour RGB (on inverse R et B pour chaque pixel)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const char* pixel = data + (y * width + x) * pixel_size;
            if (is_rgb) {
                // Écrire en ordre BGR pour le format TGA
                file.put(pixel[2]); // B
                file.put(pixel[1]); // G
                file.put(pixel[0]); // R
            }
            else {
                // Grayscale (1 octet par pixel)
                file.put(pixel[0]);
            }
        }
    }

    file.close();
    spdlog::debug("Image sauvegardée en TGA avec succès dans {}", filename);
    return true;
}


struct PngReadStructs {
    png_structp png_ptr;
    png_infop info_ptr;
};

// Utility function to handle libpng errors
void cleanup_png(PngReadStructs& png_structs) {
    if (png_structs.info_ptr) png_destroy_info_struct(png_structs.png_ptr, &png_structs.info_ptr);
    if (png_structs.png_ptr) png_destroy_read_struct(&png_structs.png_ptr, nullptr, nullptr);
}


unsigned char* load_png_from_memory(const unsigned char* dataBuf, size_t size, int& width, int& height, bool& isrgb)
{
    spdlog::debug("Initializing PNG decoder");

    // Create PNG structures
    PngReadStructs png_structs;
    png_structs.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_structs.png_ptr) {
        spdlog::error("Failed to create PNG read structure");
        return nullptr;
    }

    png_structs.info_ptr = png_create_info_struct(png_structs.png_ptr);
    if (!png_structs.info_ptr) {
        spdlog::error("Failed to create PNG info structure");
        cleanup_png(png_structs);
        return nullptr;
    }

    // Set up error handling with libpng
    if (setjmp(png_jmpbuf(png_structs.png_ptr))) {
        spdlog::error("Error decoding PNG");
        cleanup_png(png_structs);
        return nullptr;
    }

    // Initialize memory read stream
    png_set_read_fn(png_structs.png_ptr, const_cast<unsigned char*>(dataBuf),
        [](png_structp png_ptr, png_bytep data, png_size_t length) {
            unsigned char** p = reinterpret_cast<unsigned char**>(png_get_io_ptr(png_ptr));
            memcpy(data, *p, length);
            *p += length;
        });

    // Read PNG header info
    png_read_info(png_structs.png_ptr, png_structs.info_ptr);

    // Get dimensions and color type
    int color_type, bit_depth;
    png_get_IHDR(png_structs.png_ptr, png_structs.info_ptr, &width, &height, &bit_depth, &color_type, nullptr, nullptr, nullptr);

    // Force conversion to RGB
    isrgb = true;
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_structs.png_ptr); // Convert palette images to RGB
    }
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_structs.png_ptr); // Convert grayscale or grayscale+alpha to RGB
    }
    if (png_get_valid(png_structs.png_ptr, png_structs.info_ptr, PNG_INFO_tRNS) || color_type & PNG_COLOR_MASK_ALPHA) {
        png_set_strip_alpha(png_structs.png_ptr); // Strip any alpha channel
    }
    if (bit_depth == 16) {
        png_set_strip_16(png_structs.png_ptr); // Convert 16-bit to 8-bit
    }

    // Update PNG info after transformations
    png_read_update_info(png_structs.png_ptr, png_structs.info_ptr);

    // Allocate memory for RGB data (3 bytes per pixel)
    unsigned char* rgb_data = new unsigned char[width * height * 3];
    if (!rgb_data) {
        spdlog::error("Failed to allocate memory for decoded PNG image");
        cleanup_png(png_structs);
        return nullptr;
    }

    // Read image row by row
    png_bytep row_pointers[height];
    for (int y = 0; y < height; ++y) {
        row_pointers[y] = rgb_data + y * width * 3;
    }
    png_read_image(png_structs.png_ptr, row_pointers);

    // Free libpng resources
    cleanup_png(png_structs);

    return rgb_data; // Return RGB buffer
}


unsigned char* load_jpeg_from_memory(const unsigned char* dataBuf, size_t size, int& width, int& height, bool& isrgb)
{
    spdlog::debug("Initializing TurboJPEG decoder");
    tjhandle handle = tjInitDecompress();
    if (!handle) {
        spdlog::error("Failed to initialize TurboJPEG: {}", tjGetErrorStr());
        return nullptr;
    }

    int jpeg_subsample, jpeg_colorspace;
    if (tjDecompressHeader3(handle, buf, size, &width, &height, &jpeg_subsample, &jpeg_colorspace) != 0) {
        spdlog::error("Failed to read JPEG header: {}", tjGetErrorStr());
        tjDestroy(handle);
        return nullptr;
    }

    spdlog::debug("JPEG header read: width={}, height={}, colorspace={}", width, height, jpeg_colorspace);

    is_rgb = (jpeg_colorspace == TJCS_RGB || jpeg_colorspace == TJCS_YCbCr);
    int pixel_format = is_rgb ? TJPF_RGB : TJPF_GRAY;

    // Allocate buffer for the decompressed image in RGB or grayscale format
    unsigned char* rgb_buf = new unsigned char[width * height * (is_rgb ? 3 : 1)]);
    spdlog::debug("RGB buffer allocated with size: {}", width * height * (is_rgb ? 3 : 1));

    // Decompress JPEG to RGB or grayscale
    if (tjDecompress2(handle, buf, size, rgb_buf, width, 0 /* pitch */, height, pixel_format, TJFLAG_FASTDCT) != 0) {
        spdlog::error("Failed to decompress JPEG: {}", tjGetErrorStr());
        tjDestroy(handle);
        delete[] rgb_buf;
        return nullptr;
    }
    spdlog::debug("JPEG decompression completed");

    // Clean up the TurboJPEG handle
    tjDestroy(handle);

    return rgb_buf;
}


unsigned char* resize_image(const unsigned char* src, int src_w, int src_h, int tgt_h, int& tgt_w, bool is_rgb) 
{
    spdlog::debug("Resizing image from {}x{} to height {}", src_w, src_h, tgt_h);
    tgt_w = (tgt_h * src_w) / src_h;  // Calculate target width to maintain aspect ratio
    int pixel_size = is_rgb ? 3 : 1;  // 3 bytes per pixel for RGB, 1 byte for grayscale
    spdlog::debug("Target width calculated as {}, pixel size: {}", tgt_w, pixel_size);

    // Allocate the resized image buffer
    unsigned char* resized_img = new unsigned char[tgt_w * tgt_h * pixel_size];
    spdlog::debug("Resized image buffer allocated, size: {} bytes", tgt_w * tgt_h * pixel_size);

    // Nearest-neighbor resize
    for (int y = 0; y < tgt_h; y++) {
        for (int x = 0; x < tgt_w; x++) {
            int src_x = x * src_w / tgt_w;
            int src_y = y * src_h / tgt_h;
            const unsigned char* src_pixel = src + (src_y * src_w + src_x) * pixel_size;
            unsigned char* tgt_pixel = resized_img.get() + (y * tgt_w + x) * pixel_size;

            // Copy pixels depending on RGB or grayscale
            if (is_rgb) {
                tgt_pixel[0] = src_pixel[0];
                tgt_pixel[1] = src_pixel[1];
                tgt_pixel[2] = src_pixel[2];
            }
            else {
                tgt_pixel[0] = src_pixel[0];
            }
        }
    }
    spdlog::debug("Image resizing completed");

    return resized_img;
}

