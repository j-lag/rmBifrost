#include "bifrost.h"

#include "bifrost_impl.h"

bifrost::bifrost() {
    impl = std::make_shared<bifrost_impl>();
    impl->initialize();
}
