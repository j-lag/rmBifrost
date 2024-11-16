
#ifndef BIFROST_CLIENT_IMPL_H
#define BIFROST_CLIENT_IMPL_H
#include <cstdint>
#include <string>
#include <memory>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>

#include "../utils/data_structs.h"
#include "../utils/unix_socket.h"
#include "../compositor/packets/packet.h"
#include "../utils/shm_channel.h"

class bifrost_client_impl {
public:
    bifrost_client_impl(std::string application_name, std::string window_title, bool prefer_full_screen, uint32_t swapchain_image_count);
    void start();
    void stop();
    extent get_swapchain_extent() const;
    std::pair<uint32_t, void *> acquire_swapchain_image();
    void submit_frame(uint32_t framebuffer_id, rect dirty_region, refresh_type refresh_type);
    ~bifrost_client_impl();
private:
    std::string application_name;
    std::string window_title;
    bool prefer_full_screen;
    uint32_t preferred_swapchain_image_count;

    std::unique_ptr<unix_socket> socket;
    std::unique_ptr<shm_channel> channel;
    std::thread receive_thread;
    std::atomic<bool> running;

    std::string session_name;
    uint32_t swapchain_image_count;
    std::vector<uint64_t> swapchain_image_offsets;
    extent swapchain_extent;

    std::condition_variable swapchain_image_available_cv;
    std::mutex swapchain_image_available_mutex;
    std::queue<uint32_t> swapchain_image_available;

    void send_packet(const std::shared_ptr<packet>& packet) const;
    std::shared_ptr<packet> receive_packet() const;
    void create_shm_channel();
    void receive_thread_loop();
};



#endif //BIFROST_CLIENT_IMPL_H
