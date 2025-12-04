//
// Created by Vos on 13/11/2025.
//

#pragma once

#include <bitset>
#include <optional>

class ChannelPool {
public:
    std::optional<int> acquire() {
        for (size_t i = 0; i < 15; ++i) {
            if (!occupied.test(i)) {
                occupied.set(i);
                // Start at channel 2
                return i+2;
            }
        }
        return std::nullopt;
    }

    void release(int index) {
        // Start at channel 2
        occupied.reset(static_cast<size_t>(index-2));
    }

    void reset() {
        occupied.reset();
    }

private:
    std::bitset<15> occupied;
};
