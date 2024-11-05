#ifndef IMAGE_IO_H
#define IMAGE_IO_H

bool save_image_as_tga(const char* data, int width, int height, bool is_rgb, const char* filename);
unsigned char* load_png_from_memory(const unsigned char* dataBuf, size_t size, int& w, int& h, bool& isrgb);
unsigned char* load_jpeg_from_memory(const unsigned char* dataBuf, size_t size, int& w, int& h, bool& isrgb);
unsigned char* resize_image(const unsigned char* src, int src_w, int src_h, int tgt_h, int& tgt_w, bool is_rgb);
#endif //IMAGE_IO_H
