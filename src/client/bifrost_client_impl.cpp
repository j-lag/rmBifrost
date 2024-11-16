
#include "bifrost_client_impl.h"

#include <spdlog/spdlog.h>
#include "../compositor/packets/begin_session_request.h"
#include "../compositor/packets/begin_session_response.h"
#include "../compositor/packets/release_frame_packet.h"
#include "../compositor/packets/submit_frame_packet.h"
#include "../constants.h"

#include <cereal/archives/binary.hpp>
#include <sys/socket.h>

#include <sys/un.h>

bifrost_client_impl::bifrost_client_impl(std::string application_name, std::string window_title, bool prefer_full_screen, uint32_t swapchain_image_count)
    : socket(std::make_unique<unix_socket>("/run/bifrost_comp_ctl.sock", false))
    , application_name(application_name)
    , window_title(window_title)
    , prefer_full_screen(prefer_full_screen)
    , preferred_swapchain_image_count(swapchain_image_count)
{
    if (std::getenv(ENV_DEBUG)) {
        // spdlog::set_level(spdlog::level::debug);
    }
}

void bifrost_client_impl::start()
{
    auto request = std::make_shared<begin_session_request>();
    request->application_name = application_name;
    request->window_title = window_title;
    request->prefer_full_screen = prefer_full_screen;
    request->swapchain_image_count = preferred_swapchain_image_count;
    send_packet(request);

    create_shm_channel();

    running = true;
    receive_thread = std::thread([this] { receive_thread_loop(); });
}

void bifrost_client_impl::stop()
{
    running = false;
    receive_thread.join();
}

void bifrost_client_impl::create_shm_channel()
{
    auto pkt = receive_packet();
    if (std::dynamic_pointer_cast<begin_session_response>(pkt) == nullptr) {
        throw std::runtime_error("Received invalid response from server");
    }
    auto response = std::static_pointer_cast<begin_session_response>(pkt);
    spdlog::info("Session name: {}", response->session_name);
    session_name = response->session_name;
    swapchain_image_count = response->swapchain_image_count;
    swapchain_image_offsets = response->swapchain_image_offsets;
    swapchain_extent = response->swapchain_extent;
    for (uint32_t i = 0; i < swapchain_image_count; i++) {
        swapchain_image_available.push(i);
    }
    spdlog::info("Swapchain image count: {}", swapchain_image_count);
    std::string offsets_str;
    for (uint32_t i = 0; i < swapchain_image_count; i++) {
        offsets_str += std::to_string(swapchain_image_offsets[i]) + " ";
    }
    spdlog::info("Swapchain image offsets: {}", offsets_str);
    spdlog::info("Swapchain extent: {}x{}", swapchain_extent.x, swapchain_extent.y);

    channel = std::make_unique<shm_channel>(session_name, response->shared_memory_size, false);
    spdlog::info("Created shared memory channel");
}

void bifrost_client_impl::receive_thread_loop()
{
    while (running) {
        auto pkt = receive_packet();
        if (std::dynamic_pointer_cast<release_frame_packet>(pkt) != nullptr) {
            auto release_pkt = std::static_pointer_cast<release_frame_packet>(pkt);
            std::unique_lock<std::mutex> lock(swapchain_image_available_mutex);
            swapchain_image_available.push(release_pkt->framebuffer_id);
            swapchain_image_available_cv.notify_one();
        }
    }
}

void bifrost_client_impl::send_packet(const std::shared_ptr<packet>& packet) const
{
    std::ostringstream oss;
    cereal::BinaryOutputArchive archive(oss);
    archive(packet);
    spdlog::debug("Serialized packet size: {}", oss.str().size());
    const std::string serialized_packet = oss.str();
    size_t size = serialized_packet.size();
    socket->get_connection()->write(reinterpret_cast<const char*>(&size), sizeof(size));
    socket->get_connection()->write(serialized_packet.c_str(), size);
}

std::shared_ptr<packet> bifrost_client_impl::receive_packet() const
{
    size_t size;
    socket->get_connection()->read(reinterpret_cast<char*>(&size), sizeof(size));
    std::vector<char> buffer(size);
    socket->get_connection()->read(buffer.data(), size);
    std::istringstream iss(std::string(buffer.begin(), buffer.end()));
    cereal::BinaryInputArchive archive(iss);
    std::shared_ptr<packet> packet;
    archive(packet);
    return packet;
}

extent bifrost_client_impl::get_swapchain_extent() const
{
    return swapchain_extent;
}

std::pair<uint32_t, void *> bifrost_client_impl::acquire_swapchain_image()
{
    // block until an image is available
    std::unique_lock<std::mutex> lock(swapchain_image_available_mutex);
    swapchain_image_available_cv.wait(lock, [this] { return !swapchain_image_available.empty(); });
    uint32_t image_index = swapchain_image_available.front();
    swapchain_image_available.pop();
    return { image_index, channel->data + swapchain_image_offsets[image_index] };
}

void bifrost_client_impl::submit_frame(uint32_t framebuffer_id, rect dirty_region, refresh_type refresh_type)
{
    auto pkt = std::make_shared<submit_frame_packet>();
    pkt->framebuffer_id = framebuffer_id;
    pkt->dirty_rect = dirty_region;
    pkt->preferred_refresh_type = refresh_type;
    send_packet(pkt);
}

bifrost_client_impl::~bifrost_client_impl()
{
    stop();
}