#ifndef CHOOSEFILE_SCREEN_H
#define CHOOSEFILE_SCREEN_H
#include <condition_variable>
#include <vector>
#include <cstdint>
#include "lvgl_renderer.h"

class ChooseFile_screen : public std::enable_shared_from_this<ChooseFile_screen> {
public:

    void start(const char*folder);

    explicit ChooseFile_screen(const std::shared_ptr<lvgl_renderer> &shared) : lvgl_renderer_inst(shared) {
    }

    ~ChooseFile_screen();

private:
    static std::weak_ptr<ChooseFile_screen> instance;
    std::shared_ptr<lvgl_renderer> lvgl_renderer_inst;
    std::vector<lv_obj_t*> deletion_queue;

    std::condition_variable cv;
    std::mutex cv_m;

    char selected_file[256] = { 0 };
    char current_folder[512] = { 0 };
    bool isFinished = false;
    lv_obj_t* path_label = NULL;
    // Supported extensions list
    static const char* supported_extensions[];


    void create_file_browser(lv_obj_t* screen, const char* start_folder, const char** extensions);
    void list_files(const char* folder_path, const char** extensions, lv_obj_t* list);
    void folder_selected_cb(lv_event_t* e);
    void file_selected_cb(lv_event_t* e);
    void abort_cb(lv_event_t* e);
    void update_path_label();
};


#endif //CHOOSEFILE_SCREEN_H
