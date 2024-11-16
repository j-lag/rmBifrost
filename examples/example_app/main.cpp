#include <chrono>

#include "bifrost/bifrost_client.h"
#include <iostream>
#include <thread>

typedef unsigned int Rgb;                       
inline constexpr Rgb Rgba(int r, int g, int b, int a)
{ return ((a & 0xffu) << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu); }

int main()
{
    bifrost_client client("example_app", "Example App", false, 1);
    client.start();

    auto [image_index, image_ptr] = client.acquire_swapchain_image();
    auto image = reinterpret_cast<Rgb*>(image_ptr);

    auto extent = client.get_swapchain_extent();
    std::cout << "Swapchain extent: " << extent.first << "x" << extent.second << std::endl;

    for (int y = 0; y < extent.second; y++) {
        for (int x = 0; x < extent.first; x++) {
            image[y * extent.first + x] = Rgba(255, 255, 255, 255);
        }
    }

    for (int i = 0; i < 10; i++) {
        for (int y = i * 100; y < (i + 1) * 100; y++) {
            for (int x = 0; x < 100; x++) {
                if (i % 3 == 0) {
                    image[y * extent.first + x] = Rgba(255, 255, 0, 255);
                } else if (i % 3 == 1) {
                    image[y * extent.first + x] = Rgba(255, 0, 0, 255);
                } else {
                    image[y * extent.first + x] = Rgba(0, 0, 255, 255);
                }
            }
        }
    }

    std::cout << "Submitting frame" << std::endl;
    client.submit_frame(image_index, 0, 0, 100, 1000, COLOR_FAST);
    std::cout << "Submitted frame" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(100));

    return 0;
}