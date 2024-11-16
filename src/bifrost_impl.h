#ifndef BIFROST_IMPL_H
#define BIFROST_IMPL_H
#include "compositor/compositor.h"
#include "gui/lvgl_renderer.h"

#include <QObject>
#include <memory>
#include <thread>

#define DEFINE_HOOK(return_type, function_name, ...)                   \
    return_type (*function_name##_original)(__VA_ARGS__) = nullptr;    \
    static return_type function_name##_hook(__VA_ARGS__)               \
    {                                                                  \
        auto instance = bifrost_impl::instance.lock();                 \
        if (!instance) {                                               \
            throw std::runtime_error("bifrost_impl instance is null"); \
        }                                                              \
        return instance->function_name(__VA_ARGS__);                   \
    }                                                                  \
    return_type function_name(__VA_ARGS__);

class bifrost_impl : public std::enable_shared_from_this<bifrost_impl> {
public:
    void initialize();

private:
    bool hook_passthrough = false;
    void initialize_hooks();

    DEFINE_HOOK(QObject*, create_ep_fb, )

    void start_bifrost(QObject* epfb_inst);

    static std::weak_ptr<bifrost_impl> instance;
    std::shared_ptr<compositor> compositor_inst;
};

#endif // BIFROST_IMPL_H
