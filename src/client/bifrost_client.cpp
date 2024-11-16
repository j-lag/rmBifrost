#include "bifrost/bifrost_client.h"
#include "bifrost_client_impl.h"
#include "../utils/data_structs.h"

bifrost_client::bifrost_client(std::string application_name, std::string window_title, bool prefer_full_screen, uint32_t swapchain_image_count)
    : impl(std::make_shared<bifrost_client_impl>(application_name, window_title, prefer_full_screen, swapchain_image_count))
{
}

void bifrost_client::start()
{
    impl->start();
}

void bifrost_client::stop()
{
    impl->stop();
}

std::pair<uint32_t, void *> bifrost_client::acquire_swapchain_image()
{
    return impl->acquire_swapchain_image();
}

void bifrost_client::submit_frame(uint32_t framebuffer_id, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, refresh_type refresh_type)
{
    impl->submit_frame(framebuffer_id, {x1, y1, x2, y2}, refresh_type);
}

std::pair<uint32_t, uint32_t> bifrost_client::get_swapchain_extent() const
{
    auto extent = impl->get_swapchain_extent();
    return {extent.x, extent.y};
}