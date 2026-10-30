#include "openlab/core/units.hpp"

#include <cmath>
#include <limits>

namespace openlab::core {
namespace {
constexpr int exponentLimit = 32;
}

Result<Dimension> Dimension::multipliedBy(const Dimension& other) const {
    Exponents result{};
    for (std::size_t i = 0; i < result.size(); ++i) {
        const int sum = static_cast<int>(exponents_[i]) + static_cast<int>(other.exponents_[i]);
        if (sum < -exponentLimit || sum > exponentLimit) {
            return Result<Dimension>::failure(makeError(ErrorCode::InvalidArgument, "Dimension exponent overflow."));
        }
        result[i] = static_cast<std::int8_t>(sum);
    }
    return Result<Dimension>::success(Dimension(result));
}

Result<Dimension> Dimension::dividedBy(const Dimension& other) const {
    Exponents result{};
    for (std::size_t i = 0; i < result.size(); ++i) {
        const int difference = static_cast<int>(exponents_[i]) - static_cast<int>(other.exponents_[i]);
        if (difference < -exponentLimit || difference > exponentLimit) {
            return Result<Dimension>::failure(makeError(ErrorCode::InvalidArgument, "Dimension exponent overflow."));
        }
        result[i] = static_cast<std::int8_t>(difference);
    }
    return Result<Dimension>::success(Dimension(result));
}

Result<Quantity> Quantity::fromSI(double value, Dimension dimension) {
    if (!std::isfinite(value)) {
        return Result<Quantity>::failure(makeError(ErrorCode::InvalidArgument, "Quantity must be finite."));
    }
    return Result<Quantity>::success(Quantity{value, dimension});
}

bool Quantity::isFinite() const noexcept {
    return std::isfinite(si_value);
}

bool Quantity::hasDimension(const Dimension& expected) const noexcept {
    return dimension == expected;
}

std::string_view baseDimensionSymbol(BaseDimension dimension) noexcept {
    switch (dimension) {
    case BaseDimension::Length: return "m";
    case BaseDimension::Mass: return "kg";
    case BaseDimension::Time: return "s";
    case BaseDimension::ElectricCurrent: return "A";
    case BaseDimension::ThermodynamicTemperature: return "K";
    case BaseDimension::AmountOfSubstance: return "mol";
    case BaseDimension::LuminousIntensity: return "cd";
    case BaseDimension::Count: return "?";
    }
    return "?";
}

} // namespace openlab::core
