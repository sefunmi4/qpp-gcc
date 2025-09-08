#ifndef QPP_SIM_WAVEBUFFER_HPP
#define QPP_SIM_WAVEBUFFER_HPP

#include <cstddef>
#include <vector>

namespace qpp::sim {

/// Simple time-grid generator
class WaveBuffer {
public:
    WaveBuffer(std::size_t samples = 0, double dt = 1.0)
        : dt_(dt), times_(samples) {
        for (std::size_t i = 0; i < samples; ++i)
            times_[i] = i * dt_;
    }

    std::size_t size() const { return times_.size(); }
    double dt() const { return dt_; }
    double operator[](std::size_t i) const { return times_[i]; }
    const std::vector<double>& data() const { return times_; }

private:
    double dt_ = 1.0;
    std::vector<double> times_;
};

} // namespace qpp::sim

#endif // QPP_SIM_WAVEBUFFER_HPP
