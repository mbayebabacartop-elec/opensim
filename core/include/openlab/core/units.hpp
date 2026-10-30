#pragma once

#include "result.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace openlab::core {

enum class BaseDimension : std::size_t {
    Length,
    Mass,
    Time,
    ElectricCurrent,
    ThermodynamicTemperature,
    AmountOfSubstance,
    LuminousIntensity,
    Count
};

class Dimension {
public:
    using Exponents = std::array<std::int8_t, static_cast<std::size_t>(BaseDimension::Count)>;

    constexpr Dimension() = default;
    explicit constexpr Dimension(Exponents exponents) : exponents_(exponents) {}

    [[nodiscard]] constexpr const Exponents& exponents() const noexcept { return exponents_; }
    [[nodiscard]] constexpr bool isDimensionless() const noexcept {
        for (const auto exponent : exponents_) {
            if (exponent != 0) return false;
        }
        return true;
    }

    [[nodiscard]] Result<Dimension> multipliedBy(const Dimension& other) const;
    [[nodiscard]] Result<Dimension> dividedBy(const Dimension& other) const;
    [[nodiscard]] constexpr bool operator==(const Dimension&) const noexcept = default;

private:
    Exponents exponents_{};
};

namespace si {
inline constexpr Dimension dimensionless{};
inline constexpr Dimension length{{1, 0, 0, 0, 0, 0, 0}};
inline constexpr Dimension mass{{0, 1, 0, 0, 0, 0, 0}};
inline constexpr Dimension time{{0, 0, 1, 0, 0, 0, 0}};
inline constexpr Dimension electricCurrent{{0, 0, 0, 1, 0, 0, 0}};
inline constexpr Dimension thermodynamicTemperature{{0, 0, 0, 0, 1, 0, 0}};
inline constexpr Dimension amountOfSubstance{{0, 0, 0, 0, 0, 1, 0}};
inline constexpr Dimension luminousIntensity{{0, 0, 0, 0, 0, 0, 1}};
inline constexpr Dimension voltage{{2, 1, -3, -1, 0, 0, 0}};
inline constexpr Dimension resistance{{2, 1, -3, -2, 0, 0, 0}};
inline constexpr Dimension inductance{{2, 1, -2, -2, 0, 0, 0}};
inline constexpr Dimension angularVelocity{{0, 0, -1, 0, 0, 0, 0}};
inline constexpr Dimension torque{{2, 1, -2, 0, 0, 0, 0}};
inline constexpr Dimension momentOfInertia{{2, 1, 0, 0, 0, 0, 0}};
inline constexpr Dimension rotationalFriction{{2, 1, -1, 0, 0, 0, 0}};
inline constexpr Dimension power{{2, 1, -3, 0, 0, 0, 0}};
} // namespace si

struct Quantity {
    double si_value{};
    Dimension dimension{};

    [[nodiscard]] static Result<Quantity> fromSI(double value, Dimension dimension);
    [[nodiscard]] bool isFinite() const noexcept;
    [[nodiscard]] bool hasDimension(const Dimension& expected) const noexcept;
};

[[nodiscard]] std::string_view baseDimensionSymbol(BaseDimension dimension) noexcept;

} // namespace openlab::core
