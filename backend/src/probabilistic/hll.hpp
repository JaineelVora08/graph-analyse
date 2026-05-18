#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class HyperLogLog {
public:
    explicit HyperLogLog(std::uint8_t precision = 10);

    void add(std::uint64_t value);
    double estimate() const;
    void clear();

private:
    std::uint8_t precision_;
    std::vector<std::uint8_t> registers_;

    static std::uint64_t mix(std::uint64_t value);
};
