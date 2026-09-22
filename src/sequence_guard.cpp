#include "raildiag/sequence_guard.hpp"

namespace raildiag {

SequenceGuard::SequenceGuard(std::size_t max_tracked_devices)
    : max_devices_(max_tracked_devices) {}

bool SequenceGuard::check_and_update(const std::string& device, std::uint64_t sequence) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = devices_.find(device);
    if (it == devices_.end()) {
        if (devices_.size() >= max_devices_ && !lru_order_.empty()) {
            const std::string oldest = lru_order_.back();
            devices_.erase(oldest);
            lru_order_.pop_back();
        }

        lru_order_.push_front(device);
        devices_[device] = Entry{sequence, lru_order_.begin()};
        return true;
    }

    if (sequence > it->second.sequence) {
        it->second.sequence = sequence;
        lru_order_.splice(lru_order_.begin(), lru_order_, it->second.lru_it);
        return true;
    }

    return false;
}

void SequenceGuard::reset_device(const std::string& device) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devices_.find(device);
    if (it != devices_.end()) {
        lru_order_.erase(it->second.lru_it);
        devices_.erase(it);
    }
}

void SequenceGuard::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    devices_.clear();
    lru_order_.clear();
}

std::size_t SequenceGuard::tracked_device_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return devices_.size();
}

}
