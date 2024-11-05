#include "ChooseFile_screen.h"
#include <iostream>
#include "ComicArchive.h"
#include <thread>
#include <spdlog/spdlog.h>
#include <dirent.h>
#include "../constants.h"
#include <fstream>
#include <iostream>
#include <cstring>


const char* ChooseFile_screen::supported_extensions[] = { ".cbr", ".cbz", NULL };

void ChooseFile_screen::start(const char* folder) {
    spdlog::debug("starting ChooseFile_screen new");
    instance = shared_from_this();
    lvgl_renderer_inst->set_global_refresh_hint(COLOR_FAST);
    create_file_browser(lv_screen_active(), folder, supported_extensions);
    spdlog::debug("refresh");
    lvgl_renderer_inst->refresh({ 0, 0 }, { SCREEN_WIDTH, SCREEN_HEIGHT }, FULL);
    // wait for the user to select an option
    std::unique_lock<std::mutex> lk(cv_m);
    cv.wait(lk, [this] { return isFinished == true; });
    lk.unlock();

    spdlog::debug("exiting ChooseFile_screen");
}

ChooseFile_screen::~ChooseFile_screen() {
    for (auto obj : deletion_queue) {
        lv_obj_delete(obj);
    }
}

//event lambda wrapper
#define EVENT(function) [](lv_event_t* event) {if (auto instance = ChooseFile_screen::instance.lock()) { instance->function(event);}}

// Create the file browser UI using lv_display_t
void ChooseFile_screen::create_file_browser(lv_obj_t* screen, const char* start_folder, const char** extensions) {
    // Copy the start folder to the current folder global variable
    strncpy(current_folder, start_folder, sizeof(current_folder) - 1);

    // Create a vertical box container to hold the layout
    lv_obj_t* container = lv_obj_create(screen);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    deletion_queue.push_back(container);

    // Create a label at the top to display the current folder path
    path_label = lv_label_create(container);
    lv_obj_set_style_text_font(path_label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_align(path_label, LV_TEXT_ALIGN_CENTER, 0);

    deletion_queue.push_back(path_label);

    // Add margin/padding around the label
    lv_obj_set_style_pad_all(path_label, 10, 0);  // 10px margin/padding
    update_path_label();  // Update with the current path

    // Create the file list below the path label
    lv_obj_t* file_list = lv_list_create(container);
    lv_obj_set_size(file_list, lv_pct(100), lv_pct(80));
    deletion_queue.push_back(file_list);

    // Populate the list with files
    list_files(current_folder, extensions, file_list);


    //Create the "Go Back" button at the bottom
    spdlog::debug("button");
    auto abort_btn = lv_obj_create(container);
    lv_obj_set_size(abort_btn, LV_PCT(30), LV_SIZE_CONTENT);
    lv_obj_set_style_border_color(abort_btn, lv_color_black(), 0);
    lv_obj_set_style_border_width(abort_btn, 5, 0);
    lv_obj_set_style_bg_color(abort_btn, lv_color_white(), 0);
    lv_obj_set_style_radius(abort_btn, 0, 0);
    lv_obj_set_flex_flow(abort_btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(abort_btn, 40, 0);
    lv_obj_set_flex_align(abort_btn, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t* label = lv_label_create(abort_btn);
    lv_label_set_text_fmt(label, LV_SYMBOL_LEFT " Go Back");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_40, 0);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_margin_top(label, 10, 0);
    lv_obj_align(abort_btn, LV_ALIGN_BOTTOM_LEFT, 0, -30);
    lv_obj_add_event_cb(abort_btn, EVENT(abort_cb), LV_EVENT_CLICKED, NULL);
    deletion_queue.push_back(abort_btn);
    deletion_queue.push_back(label);
}

void ChooseFile_screen::list_files(const char* folder_path, const char** extensions, lv_obj_t* list) {
    struct dirent* entry;
    DIR* dp = opendir(folder_path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    std::vector<std::string> directories;
    std::vector<std::string> files;

    while ((entry = readdir(dp))) {
        // Skip "." and ".."
        if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0) 
            continue;        
        if (entry->d_type == DT_DIR)
            directories.push_back(entry->d_name);
        else 
            files.push_back(entry->d_name);
    }
    closedir(dp);

    // Sort directories and files lexicographically
    std::sort(directories.begin(), directories.end());
    std::sort(files.begin(), files.end());


    // Clear the current list before populating it
    lv_obj_clean(list);


    // Add "Parent Directory" button to go back
    if (strcmp(folder_path, "/") != 0) {  // If not in root
        lv_obj_t* btn = lv_list_add_btn(list, LV_SYMBOL_UP, "..");
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_30, 0);
        lv_obj_add_event_cb(btn, EVENT(folder_selected_cb), LV_EVENT_CLICKED, NULL);
    }


    for (const auto& dir : directories)
    {
        spdlog::debug("adding folder: {}", dir);
        // Add a folder button
        lv_obj_t* btn = lv_list_add_btn(list, LV_SYMBOL_DIRECTORY, dir.c_str());
        lv_label_set_long_mode(lv_obj_get_child(btn, 1), LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_30, 0);
        lv_obj_add_event_cb(btn, EVENT(folder_selected_cb), LV_EVENT_CLICKED, NULL);
    }


    for (const auto& file : files) 
    {
        spdlog::debug("adding file: {}", file);
        // Check if file has a supported extension
        const char* ext = strrchr(file.c_str(), '.');
        bool supported = false;
        if (ext) {
            for (const char** e = extensions; *e != NULL; e++) {
                if (strcmp(ext, *e) == 0) {
                    supported = true;
                    break;
                }
            }
        }
        if (supported) 
        {
            //lv_obj_t* btn = lv_list_add_btn(list, LV_SYMBOL_FILE, file.c_str());
            //lv_label_set_long_mode(lv_obj_get_child(btn, 1), LV_LABEL_LONG_WRAP);
            //lv_obj_set_style_text_font(btn, &lv_font_montserrat_30, 0);

            
            // Create a list button for each supported file with a custom icon
            lv_obj_t* btn = lv_list_add_btn(list, NULL, NULL);  // Leave text as NULL, since we handle it separately
            lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);      // Set button to vertical flex layout

            // First row: Create a container for the icon and label
            lv_obj_t* first_row = lv_obj_create(btn);
            lv_obj_set_size(first_row, LV_PCT(100), LV_SIZE_CONTENT);  // Full width, height adjusts to content
            lv_obj_set_flex_flow(first_row, LV_FLEX_FLOW_ROW);         // Horizontal layout for icon and label
            lv_obj_set_style_pad_row(first_row, 4, 0);                 // Optional: space between icon and label
            lv_obj_set_scrollbar_mode(first_row, LV_SCROLLBAR_MODE_OFF);

            // Add the directory symbol icon
            lv_obj_t* symbol_label = lv_label_create(first_row);
            lv_label_set_text(symbol_label, LV_SYMBOL_FILE);

            // Add the main label for the text next to the symbol
            lv_obj_t* label = lv_label_create(first_row);
            lv_label_set_text(label, file.c_str());                   // Set text from entry
            lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);         // Enable text wrapping if needed
            
            ///////////////////////////////////////////////////////////////
            //get first image of archive
            std::string fullfilename = std::string(current_folder) + file;
            ComicArchive* archive = ComicArchive::Create(fullfilename.c_str());
            if (archive)
            {
                lv_img_dsc_t* img_dsc =  archive->GetImage(0, 128);                
                if (img_dsc)
                {
                    lv_obj_t* icon = lv_img_create(btn);
                    lv_img_set_src(icon, img_dsc);
                }
                delete archive;
            }
            ///////////////////////////////////////////////////////////////            
            lv_obj_add_event_cb(btn, EVENT(file_selected_cb), LV_EVENT_CLICKED, NULL);
        }
    }
}

// Callback for folder selection (navigating into directories)
void ChooseFile_screen::folder_selected_cb(lv_event_t* e) {
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    const char* foldername = lv_label_get_text(lv_obj_get_child(btn, 1));  // Retrieve folder name

    if (strcmp(foldername, "..") == 0) {
        // Go back to parent directory
        for (int i = strlen(current_folder) - 2; i >= 0; --i)
        {
            if (current_folder[i] == '/')
            {
                current_folder[i + 1] = '\0';
                break;
            }
        }
    }
    else {
        // Append the selected folder to the current path
        strncat(current_folder, foldername, sizeof(current_folder) - strlen(current_folder) - 1);
        strncat(current_folder, "/", sizeof(current_folder) - strlen(current_folder) - 1);
    }

    // Re-list the files in the new folder
    lv_obj_t* list = lv_obj_get_parent(btn);
    list_files(current_folder, supported_extensions, list);

    // Update the path label
    update_path_label();
}

// Callback for file selection
void ChooseFile_screen::file_selected_cb(lv_event_t* e) {
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    const char* filename = lv_label_get_text(lv_obj_get_child(lv_obj_get_child(btn, 0), 1));  // Retrieve file name
    //const char* filename = lv_label_get_text(lv_obj_get_child(btn, 1));  // Retrieve file name
    // Copy the selected filename
    strncpy(selected_file, current_folder, sizeof(selected_file) - 1);
    strncpy(selected_file, filename, sizeof(selected_file) - 1);
    // Close the UI or perform additional actions here
    spdlog::debug("Selected file: {}\n", selected_file);
    isFinished = true;
    cv.notify_one();
}

// Callback for abort button
void ChooseFile_screen::abort_cb(lv_event_t* e) {
    // Close the UI or handle the abort process
    spdlog::debug("File selection aborted.\n");
    isFinished = true;
    cv.notify_one();
}

// Update the path label with the current folder path
void ChooseFile_screen::update_path_label() {
    lv_label_set_text(path_label, current_folder);
    lv_label_set_long_mode(path_label, LV_LABEL_LONG_WRAP);
}

std::weak_ptr<ChooseFile_screen> ChooseFile_screen::instance;
