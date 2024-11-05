#ifndef BOOKCONFIG_H
#define BOOKCONFIG_H

#include "cJSON.h"
#include <string>

// Class to store book information
class BookInfo {
public:
    std::string name;
    int currentPage = 0;
    int pageCount = 0;
};

// Singleton class to manage book configuration
class BookConfig {
public:
    // Get the singleton instance of the BookConfig class
    static BookConfig& GetInstance();

    // Deleted methods to prevent copying or assigning the singleton
    BookConfig(const BookConfig&) = delete;
    BookConfig& operator=(const BookConfig&) = delete;

    // Initialization with the JSON config file path
    void Init(const std::string& jsonConfigFile);

    // Getter and setter methods for current book and folder
    std::string GetCurrentBook();
    void SetCurrentBook(const std::string& currentBook);

    std::string GetCurrentFolder();
    void SetCurrentFolder(const std::string& currentFolder);

    // Methods to get and set book information
    BookInfo GetBookInfo(const std::string& bookName);
    void SetBookInfo(const BookInfo& bookInfo);

private:
    // Private constructor for the singleton pattern
    BookConfig() = default;

    // Private helper methods
    void SaveToFile();
    void LoadFromFile();
    void InitializeDefaults();

    // Member variables
    std::string configFilePath;
    cJSON* root = nullptr;
};

#endif // BOOKCONFIG_H
