#ifndef BIFROST_H
#define BIFROST_H
#include <memory>

class bifrost_impl;

class bifrost {
  public:
    bifrost();

    bifrost(const bifrost &other) = delete;
    bifrost &operator=(const bifrost &other) = delete;

  private:
    std::shared_ptr<bifrost_impl> impl;
};

#endif