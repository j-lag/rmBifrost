#ifndef IMAGE_IO_H
#define IMAGE_IO_H

class Image
{
public:
	~Image() { if (data) delete[] data; }
	char* data = nullptr;
	int width = 0;
	int height = 0;
	bool is_rgb = false;
};

bool save_image_as_tga(Image& image, const char* filename);
bool load_image_as_tga(Image& image, const char* filename);
bool load_png_from_memory(const unsigned char* dataBuf, unsigned int size, Image& image);
bool load_jpeg_from_memory(const unsigned char* dataBuf, unsigned int size, Image& image);
char* resize_image_NearestNeighbor(const char* src, int src_w, int src_h, int tgt_h, int& tgt_w, bool is_rgb);
char* resize_image_lanczos(const char* src, int src_w, int src_h, int tgt_h, int& tgt_w, bool is_rgb, int a = 3);
#endif //IMAGE_IO_H
