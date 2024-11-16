#include "ImageIo.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <iostream>
#include <png.h>
#include <turbojpeg.h>
#include <spdlog/spdlog.h>

bool save_image_as_tga(Image& image, const char* filename)
{
    // Ouvre le fichier en mode binaire
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        spdlog::debug("Erreur: Impossible d'ouvrir le fichier {} pour l'écriture.", filename);
        return false;
    }

    // Taille d'un pixel en octets (3 pour RGB, 1 pour grayscale)
    int pixel_size = image.is_rgb ? 3 : 1;

    // Prépare l'en-tête TGA
    unsigned char header[18];
    std::memset(header, 0, sizeof(header));  // Initialise tout l'en-tête à 0
    header[2] = 2;  // Type d'image: 2 pour une image non compressée en Truecolor
    header[12] = image.width & 0xFF;        // Largeur (octet bas)
    header[13] = (image.width >> 8) & 0xFF; // Largeur (octet haut)
    header[14] = image.height & 0xFF;       // Hauteur (octet bas)
    header[15] = (image.height >> 8) & 0xFF; // Hauteur (octet haut)
    header[16] = pixel_size * 8;      // Profondeur de couleur: 24 bits pour RGB, 8 bits pour grayscale

    // Écrit l'en-tête dans le fichier
    file.write(reinterpret_cast<const char*>(header), sizeof(header));

    // Écrit les données d'image dans le fichier
    // Note : Les données TGA sont en ordre BGR pour RGB (on inverse R et B pour chaque pixel)
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            const char* pixel = image.data + (y * image.width + x) * pixel_size;
            if (image.is_rgb) {
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

bool load_image_as_tga(Image& image, const char* filename)
{
    // Ouvre le fichier en mode binaire
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        spdlog::debug("Erreur: Impossible d'ouvrir le fichier {} pour la lecture.", filename);
        return false;
    }

    // Lis l'en-tête TGA
    unsigned char header[18];
    file.read(reinterpret_cast<char*>(header), sizeof(header));

    if (!file) {
        spdlog::debug("Erreur: Impossible de lire l'en-tête du fichier {}", filename);
        return false;
    }

    // Vérifie le type d'image (seuls les types non compressés Truecolor ou grayscale sont pris en charge)
    if (header[2] != 2) {
        spdlog::debug("Erreur: Type d'image TGA non pris en charge (type: {}).", header[2]);
        return false;
    }

    // Récupère les dimensions et la profondeur de couleur
    int width = header[12] | (header[13] << 8);
    int height = header[14] | (header[15] << 8);
    int bits_per_pixel = header[16];

    if (bits_per_pixel != 24 && bits_per_pixel != 8) {
        spdlog::debug("Erreur: Profondeur de couleur non prise en charge ({} bits).", bits_per_pixel);
        return false;
    }

    bool is_rgb = (bits_per_pixel == 24);
    int pixel_size = is_rgb ? 3 : 1;

    // Alloue la mémoire pour les données de l'image
    char* data = new char[width * height * pixel_size];
    if (!data) {
        spdlog::debug("Erreur: Échec d'allocation mémoire pour l'image.");
        return false;
    }

    // Lis les données de l'image
    file.read(data, width * height * pixel_size);

    if (!file) {
        spdlog::debug("Erreur: Échec de lecture des données d'image dans {}.", filename);
        delete[] data;
        return false;
    }

    // Inverser l'ordre des couleurs pour les images RGB (BGR -> RGB)
    if (is_rgb) {
        for (int i = 0; i < width * height; ++i) {
            std::swap(data[i * 3], data[i * 3 + 2]); // Échange B et R
        }
    }

    // Ferme le fichier
    file.close();

    // Libère les données précédentes de l'image (si elles existent)
    if (image.data) {
        delete[] image.data;
    }

    // Remplit les champs de l'objet Image
    image.data = data;
    image.width = width;
    image.height = height;
    image.is_rgb = is_rgb;

    spdlog::debug("Image chargée avec succès depuis {}", filename);
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


bool load_png_from_memory(const unsigned char* dataBuf, unsigned int size, Image& image)
{
    spdlog::debug("Initializing PNG decoder");

    // Create PNG structures
    PngReadStructs png_structs;
    png_structs.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_structs.png_ptr) {
        spdlog::error("Failed to create PNG read structure");
        return false;
    }

    png_structs.info_ptr = png_create_info_struct(png_structs.png_ptr);
    if (!png_structs.info_ptr) {
        spdlog::error("Failed to create PNG info structure");
        cleanup_png(png_structs);
        return false;
    }

    // Set up error handling with libpng
    if (setjmp(png_jmpbuf(png_structs.png_ptr))) {
        spdlog::error("Error decoding PNG");
        cleanup_png(png_structs);
        return false;
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
    png_get_IHDR(png_structs.png_ptr, png_structs.info_ptr, (unsigned int*) &image.width, (unsigned int*) &image.height, &bit_depth, &color_type, nullptr, nullptr, nullptr);

    // Force conversion to RGB
    image.is_rgb = true;
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
    image.data = new char[image.width * image.height * 3];
    if (!image.data) {
        spdlog::error("Failed to allocate memory for decoded PNG image");
        cleanup_png(png_structs);
        return false;
    }

    // Read image row by row
    png_bytep row_pointers[image.height];
    for (int y = 0; y < image.height; ++y) {
        row_pointers[y] = (unsigned char*)image.data + y * image.width * 3;
    }
    png_read_image(png_structs.png_ptr, row_pointers);

    // Free libpng resources
    cleanup_png(png_structs);

    return true; // Return RGB buffer
}


bool load_jpeg_from_memory(const unsigned char* dataBuf, unsigned int size, Image& image)
{
    spdlog::debug("Initializing TurboJPEG decoder");
    tjhandle handle = tjInitDecompress();
    if (!handle) {
        spdlog::error("Failed to initialize TurboJPEG: {}", tjGetErrorStr());
        return false;
    }

    int jpeg_subsample, jpeg_colorspace;
    if (tjDecompressHeader3(handle, dataBuf, size, &image.width, &image.height, &jpeg_subsample, &jpeg_colorspace) != 0) {
        spdlog::error("Failed to read JPEG header: {}", tjGetErrorStr());
        tjDestroy(handle);
        return false;
    }

    spdlog::debug("JPEG header read: width={}, height={}, colorspace={}", image.width, image.height, jpeg_colorspace);

    image.is_rgb = (jpeg_colorspace == TJCS_RGB || jpeg_colorspace == TJCS_YCbCr);
    int pixel_format = image.is_rgb ? TJPF_RGB : TJPF_GRAY;

    // Allocate buffer for the decompressed image in RGB or grayscale format
    image.data = new char[image.width * image.height * (image.is_rgb ? 3 : 1)];
    spdlog::debug("RGB buffer allocated with size: {}", image.width * image.height * (image.is_rgb ? 3 : 1));

    // Decompress JPEG to RGB or grayscale
    if (tjDecompress2(handle, dataBuf, size, (unsigned char*)image.data, image.width, 0 /* pitch */, image.height, pixel_format, TJFLAG_FASTDCT) != 0) {
        spdlog::error("Failed to decompress JPEG: {}", tjGetErrorStr());
        tjDestroy(handle);
        delete[] image.data;
        image.data = nullptr;
        return false;
    }
    spdlog::debug("JPEG decompression completed");

    // Clean up the TurboJPEG handle
    tjDestroy(handle);

    return true;
}


char* resize_image_NearestNeighbor(const char* src, int src_w, int src_h, int tgt_h, int& tgt_w, bool is_rgb) 
{
    spdlog::debug("Resizing image from {}x{} to height {}", src_w, src_h, tgt_h);
    tgt_w = (tgt_h * src_w) / src_h;  // Calculate target width to maintain aspect ratio
    int pixel_size = is_rgb ? 3 : 1;  // 3 bytes per pixel for RGB, 1 byte for grayscale
    spdlog::debug("Target width calculated as {}, pixel size: {}", tgt_w, pixel_size);

    // Allocate the resized image buffer
    char* resized_img = new char[tgt_w * tgt_h * pixel_size];
    spdlog::debug("Resized image buffer allocated, size: {} bytes", tgt_w * tgt_h * pixel_size);

    // Nearest-neighbor resize
    for (int y = 0; y < tgt_h; y++) {
        for (int x = 0; x < tgt_w; x++) {
            int src_x = x * src_w / tgt_w;
            int src_y = y * src_h / tgt_h;
            const char* src_pixel = src + (src_y * src_w + src_x) * pixel_size;
            char* tgt_pixel = resized_img + (y * tgt_w + x) * pixel_size;

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




// Lanczos kernel function
inline float lanczos_kernel(float x, int a = 3) {
    if (x == 0.0f) return 1.0f;
    if (std::abs(x) >= a) return 0.0f;
    x *= M_PI;
    return (a * std::sin(x) * std::sin(x / a)) / (x * x);
}

// Resizing function using Lanczos filter
char* resize_image_lanczos(const char* src, int src_w, int src_h, int tgt_h, int& tgt_w, bool is_rgb, int a)
{
    spdlog::debug("Resizing image from {}x{} to height {} using Lanczos filter", src_w, src_h, tgt_h);
    tgt_w = (tgt_h * src_w) / src_h;  // Calculate target width to maintain aspect ratio
    int pixel_size = is_rgb ? 3 : 1;  // 3 bytes per pixel for RGB, 1 byte for grayscale
    spdlog::debug("Target width calculated as {}, pixel size: {}", tgt_w, pixel_size);

    // Allocate the resized image buffer
    char* resized_img = new char[tgt_w * tgt_h * pixel_size];
    spdlog::debug("Resized image buffer allocated, size: {} bytes", tgt_w * tgt_h * pixel_size);

    // Precompute Lanczos kernel weights for efficiency
    std::vector<float> kernel(2 * a + 1);
    for (int i = 0; i <= 2 * a; i++) {
        kernel[i] = lanczos_kernel(i - a, a);
    }

    // Resizing with Lanczos filter
    for (int y = 0; y < tgt_h; y++) {
        for (int x = 0; x < tgt_w; x++) {
            float src_x = static_cast<float>(x * src_w) / tgt_w;
            float src_y = static_cast<float>(y * src_h) / tgt_h;

            int src_x_int = static_cast<int>(src_x);
            int src_y_int = static_cast<int>(src_y);

            float x_weight = src_x - src_x_int;
            float y_weight = src_y - src_y_int;

            float pixel_sum[3] = { 0.0f, 0.0f, 0.0f };
            float weight_sum = 0.0f;

            // Lanczos resampling kernel application
            for (int ky = -a; ky <= a; ky++) {
                int src_y_sample = std::clamp(src_y_int + ky, 0, src_h - 1);
                for (int kx = -a; kx <= a; kx++) {
                    int src_x_sample = std::clamp(src_x_int + kx, 0, src_w - 1);

                    float kernel_x = lanczos_kernel(src_x - (src_x_sample), a);
                    float kernel_y = lanczos_kernel(src_y - (src_y_sample), a);
                    float weight = kernel_x * kernel_y;

                    const char* src_pixel = src + (src_y_sample * src_w + src_x_sample) * pixel_size;
                    weight_sum += weight;

                    if (is_rgb) {
                        pixel_sum[0] += weight * src_pixel[0];
                        pixel_sum[1] += weight * src_pixel[1];
                        pixel_sum[2] += weight * src_pixel[2];
                    }
                    else {
                        pixel_sum[0] += weight * src_pixel[0];
                    }
                }
            }

            char* tgt_pixel = resized_img + (y * tgt_w + x) * pixel_size;
            if (is_rgb) {
                tgt_pixel[0] = std::clamp(static_cast<int>(pixel_sum[0] / weight_sum), 0, 255);
                tgt_pixel[1] = std::clamp(static_cast<int>(pixel_sum[1] / weight_sum), 0, 255);
                tgt_pixel[2] = std::clamp(static_cast<int>(pixel_sum[2] / weight_sum), 0, 255);
            }
            else {
                tgt_pixel[0] = std::clamp(static_cast<int>(pixel_sum[0] / weight_sum), 0, 255);
            }
        }
    }

    spdlog::debug("Image resizing completed using Lanczos filter");
    return resized_img;
}