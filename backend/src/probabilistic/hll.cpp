#include "hll.hpp"

#include <algorithm>
#include <cmath>

HyperLogLog::HyperLogLog(std::uint8_t precision)
    : precision_(precision < 4 ? 4 : precision > 16 ? 16 : precision),
      registers_(std::size_t{1} << precision_, 0) {}

std::uint64_t HyperLogLog::mix(std::uint64_t value) {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

void HyperLogLog::add(std::uint64_t value) {
    const std::uint64_t hash = mix(value);
    const std::uint64_t index = hash >> (64 - precision_);
    std::uint64_t remainder = hash << precision_;

    std::uint8_t rank = 1;
    const std::uint8_t max_rank = static_cast<std::uint8_t>(64 - precision_ + 1);
    while ((remainder & (1ULL << 63)) == 0 && rank < max_rank) {
        ++rank;
        remainder <<= 1;
    }
    registers_[static_cast<std::size_t>(index)] =
        std::max(registers_[static_cast<std::size_t>(index)], rank);
}

double HyperLogLog::estimate() const {
    const double m = static_cast<double>(registers_.size());
    double alpha = 0.7213 / (1.0 + 1.079 / m);
    if (registers_.size() == 16) alpha = 0.673;
    else if (registers_.size() == 32) alpha = 0.697;
    else if (registers_.size() == 64) alpha = 0.709;

    double harmonic_sum = 0.0;
    std::size_t zero_registers = 0;
    for (std::uint8_t reg : registers_) {
        harmonic_sum += std::ldexp(1.0, -static_cast<int>(reg));
        if (reg == 0) ++zero_registers;
    }

    double raw_estimate = alpha * m * m / harmonic_sum;
    if (raw_estimate <= 2.5 * m && zero_registers > 0) {
        raw_estimate = m * std::log(m / static_cast<double>(zero_registers));
    }
    return raw_estimate;
}

void HyperLogLog::clear() {
    std::fill(registers_.begin(), registers_.end(), 0);
}
