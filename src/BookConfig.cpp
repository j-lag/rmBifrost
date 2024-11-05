#include "BookConfig.h"
#include <fstream>
#include <iostream>

// Singleton instance method
BookConfig& BookConfig::GetInstance() {
    static BookConfig instance;
    return instance;
}

// Initialize the JSON configuration file path and load the file
void BookConfig::Init(const std::string& jsonConfigFile) {
    configFilePath = jsonConfigFile;
    LoadFromFile();
}

// Load the JSON configuration from the file
void BookConfig::LoadFromFile() {
    std::ifstream file(configFilePath);

    if (file) {
        std::string jsonStr((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        file.close();

        if (!jsonStr.empty()) {
            root = cJSON_Parse(jsonStr.c_str());
        }
    }

    if (root == nullptr) {
        // If file is empty or invalid, initialize with default values
        root = cJSON_CreateObject();
        InitializeDefaults();
        SaveToFile();
    }
}

// Save the JSON object to the file
void BookConfig::SaveToFile() {
    char* jsonString = cJSON_Print(root);
    std::ofstream file(configFilePath);

    if (file.is_open()) {
        file << jsonString;
        file.close();
    }

    cJSON_free(jsonString);  // Free the string allocated by cJSON_Print
}

// Initialize the JSON object with default values
void BookConfig::InitializeDefaults() {
    cJSON_AddStringToObject(root, "current_book", "");
    cJSON_AddStringToObject(root, "current_folder", "/home/root/");
    cJSON_AddObjectToObject(root, "book_status");
}

// Get the current book name from the JSON config
std::string BookConfig::GetCurrentBook() {
    cJSON* currentBookItem = cJSON_GetObjectItem(root, "current_book");
    return currentBookItem && cJSON_IsString(currentBookItem) ? currentBookItem->valuestring : "";
}

// Set the current book name and update the JSON file
void BookConfig::SetCurrentBook(const std::string& currentBook) {
    cJSON_ReplaceItemInObject(root, "current_book", cJSON_CreateString(currentBook.c_str()));
    SaveToFile();
}

// Get the current folder from the JSON config
std::string BookConfig::GetCurrentFolder() {
    cJSON* currentFolderItem = cJSON_GetObjectItem(root, "current_folder");
    return currentFolderItem && cJSON_IsString(currentFolderItem) ? currentFolderItem->valuestring : "/";
}

// Set the current folder and update the JSON file
void BookConfig::SetCurrentFolder(const std::string& currentFolder) {
    cJSON_ReplaceItemInObject(root, "current_folder", cJSON_CreateString(currentFolder.c_str()));
    SaveToFile();
}

// Get book information (current page and page count) from the JSON file
BookInfo BookConfig::GetBookInfo(const std::string& bookName) {
    BookInfo bookInfo;
    bookInfo.name = bookName;

    cJSON* bookStatus = cJSON_GetObjectItem(root, "book_status");
    cJSON* bookItem = cJSON_GetObjectItem(bookStatus, bookName.c_str());

    if (bookItem) {
        cJSON* currentPageItem = cJSON_GetObjectItem(bookItem, "current_page");
        cJSON* pageCountItem = cJSON_GetObjectItem(bookItem, "page_count");

        bookInfo.currentPage = currentPageItem && cJSON_IsNumber(currentPageItem) ? currentPageItem->valueint : 0;
        bookInfo.pageCount = pageCountItem && cJSON_IsNumber(pageCountItem) ? pageCountItem->valueint : 0;
    }

    return bookInfo;
}

// Set book information and update the JSON file
void BookConfig::SetBookInfo(const BookInfo& bookInfo) {
    cJSON* bookStatus = cJSON_GetObjectItem(root, "book_status");
    if (!bookStatus) {
        bookStatus = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "book_status", bookStatus);
    }

    cJSON* bookItem = cJSON_CreateObject();
    cJSON_AddNumberToObject(bookItem, "current_page", bookInfo.currentPage);
    cJSON_AddNumberToObject(bookItem, "page_count", bookInfo.pageCount);

    cJSON_ReplaceItemInObject(bookStatus, bookInfo.name.c_str(), bookItem);

    SaveToFile();
}
