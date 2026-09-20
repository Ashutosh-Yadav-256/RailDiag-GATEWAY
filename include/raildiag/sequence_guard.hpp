#ifndef RAILDIAG_SEQUENCE_GUARD_HPP
#define RAILDIAG_SEQUENCE_GUARD_HPP

#include <cstdint>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

namespace raildiag {

class SequenceGuard {
public:
    explicit SequenceGuard(std::size_t max_tracked_devices = 1024);

    [[nodiscard]] bool check_and_update(const std::string& device, std::uint64_t sequence);
    void reset_device(const std::string& device);
    void clear();
    [[nodiscard]] std::size_t tracked_device_count() const;

private:
    struct Entry {
        std::uint64_t sequence{};
        std::list<std::string>::iterator lru_it;
    };

    std::size_t max_devices_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Entry> devices_;
    std::list<std::string> lru_order_;
};

}

#endif
