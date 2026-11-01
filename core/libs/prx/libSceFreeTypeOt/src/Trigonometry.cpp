#include "prx/libSceFreeTypeOt/include/Trigonometry.hpp"
#include "prx/libc/include/General.hpp"
#include "SceTypes.hpp"

#include <cstdint>
#include <limits>

namespace {

using FreeType::GuestVector;

constexpr std::int64_t AnglePi = 180LL << 16;
constexpr std::int64_t AnglePi2 = AnglePi / 2;
constexpr std::int64_t Angle2Pi = AnglePi * 2;
constexpr std::uint64_t HalfTurn = static_cast<std::uint64_t>(AnglePi);
constexpr std::uint32_t TrigScale = 0x9B74EDA8u;
constexpr std::int64_t UnitLength = 0x26DD3B6A >> 2;
constexpr int TrigIterations = 23;
constexpr std::int64_t ArctanTable[TrigIterations] = {
    2949120, 1740967, 919879, 466945, 234379, 117304, 58666, 29335, 14668, 7334, 3667, 1833,
    917, 458, 229, 115, 57, 29, 14, 7, 4, 2, 1};

std::int64_t Add(std::int64_t a, std::int64_t b) {
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(a) + static_cast<std::uint64_t>(b));
}

std::int64_t Subtract(std::int64_t a, std::int64_t b) {
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(a) - static_cast<std::uint64_t>(b));
}

std::int64_t Negate(std::int64_t value) {
    return Subtract(0, value);
}

std::int64_t Magnitude(std::int64_t value) {
    return value >= 0 ? value : Negate(value);
}

std::int64_t ShiftLeft(std::int64_t value, int shift) {
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(value) << shift);
}

std::int64_t Downscale(std::int64_t value) {
    const std::int64_t magnitude = Magnitude(value);
    const std::uint32_t v1 = static_cast<std::uint32_t>(magnitude) >> 16;
    const std::uint32_t v2 = static_cast<std::uint32_t>(magnitude & 0xFFFF);
    const std::uint32_t k1 = TrigScale >> 16;
    const std::uint32_t k2 = TrigScale & 0xFFFF;
    std::uint32_t high = k1 * v1;
    std::uint32_t low = k1 * v2 + k2 * v1;
    const std::uint32_t carry = (k2 * v2) >> 16;
    const std::uint32_t bound = low >= carry ? low : carry;
    low += carry;
    high += low >> 16;
    if (low < bound) {
        high += 0x10000;
    }
    const auto result = static_cast<std::int64_t>(high);
    return value >= 0 ? result : -result;
}

int Prenormalize(GuestVector& vector) {
    const std::int64_t x = vector.x;
    const std::int64_t y = vector.y;
    std::int64_t z = Magnitude(x) | Magnitude(y);
    int shift = 0;
    for (const int step : {16, 8, 4, 2, 1}) {
        if (z >= (std::int64_t{1} << step)) {
            z >>= step;
            shift += step;
        }
    }
    if (shift <= 27) {
        shift = 27 - shift;
        vector.x = ShiftLeft(x, shift);
        vector.y = ShiftLeft(y, shift);
        return shift;
    }
    shift -= 27;
    vector.x = x >> shift;
    vector.y = y >> shift;
    return -shift;
}

void PseudoRotate(GuestVector& vector, std::int64_t theta) {
    std::int64_t x = vector.x;
    std::int64_t y = vector.y;
    std::uint64_t halfTurns = 0;
    if (theta <= -AnglePi2) {
        halfTurns = (static_cast<std::uint64_t>(-AnglePi2) - static_cast<std::uint64_t>(theta)) / HalfTurn + 1;
        theta = static_cast<std::int64_t>(static_cast<std::uint64_t>(theta) + halfTurns * HalfTurn);
    } else if (theta > AnglePi2) {
        halfTurns = (static_cast<std::uint64_t>(theta) - static_cast<std::uint64_t>(AnglePi2) + HalfTurn - 1) / HalfTurn;
        theta = static_cast<std::int64_t>(static_cast<std::uint64_t>(theta) - halfTurns * HalfTurn);
    }
    if (halfTurns & 1) {
        x = Negate(x);
        y = Negate(y);
    }
    for (int i = 0; i < TrigIterations; ++i) {
        if (theta < 0) {
            const std::int64_t next = Add(x, y >> i);
            y = Subtract(y, x >> i);
            x = next;
            theta += ArctanTable[i];
        } else {
            const std::int64_t next = Subtract(x, y >> i);
            y = Add(y, x >> i);
            x = next;
            theta -= ArctanTable[i];
        }
    }
    vector.x = x;
    vector.y = y;
}

std::int64_t RoundAngle(std::int64_t angle) {
    return (angle + 16) & ~std::int64_t{31};
}

void PseudoPolarize(GuestVector& vector) {
    std::int64_t x = vector.x;
    std::int64_t y = vector.y;
    std::int64_t theta = 0;
    if (x < 0) {
        x = Negate(x);
        y = Negate(y);
        theta = AnglePi;
    }
    if (y > 0) {
        theta = -theta;
    }
    for (int i = 0; i < TrigIterations; ++i) {
        if (y > 0) {
            const std::int64_t next = Add(x, y >> i);
            y = Subtract(y, x >> i);
            x = next;
            theta += ArctanTable[i];
        } else {
            const std::int64_t next = Subtract(x, y >> i);
            y = Add(y, x >> i);
            x = next;
            theta -= ArctanTable[i];
        }
    }
    vector.x = x;
    vector.y = theta >= 0 ? RoundAngle(theta) : -RoundAngle(-theta);
}

GuestVector RotatedUnit(std::int64_t angle) {
    GuestVector vector{UnitLength, 0};
    PseudoRotate(vector, angle);
    return vector;
}

std::uint32_t Divide64By32(std::uint32_t high, std::uint32_t low, std::uint32_t divisor) {
    std::uint32_t remainder = high;
    std::uint32_t quotient = 0;
    if (remainder >= divisor) {
        return 0x7FFFFFFF;
    }
    for (int i = 0; i < 32; ++i) {
        remainder <<= 1;
        quotient <<= 1;
        remainder |= low >> 31;
        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= 1;
        }
        low <<= 1;
    }
    return quotient;
}

std::int64_t DivFix(std::int64_t a, std::int64_t b) {
    std::int32_t sign = static_cast<std::int32_t>(a);
    a = Magnitude(a);
    sign ^= static_cast<std::int32_t>(b);
    b = Magnitude(b);
    std::uint32_t quotient = 0;
    if (static_cast<std::uint32_t>(b) == 0) {
        quotient = 0x7FFFFFFF;
    } else if ((a >> 16) == 0) {
        quotient = static_cast<std::uint32_t>(Add(ShiftLeft(a, 16), b >> 1)) / static_cast<std::uint32_t>(b);
    } else {
        const std::uint32_t low = static_cast<std::uint32_t>(ShiftLeft(a, 16));
        const std::uint32_t sumLow = low + static_cast<std::uint32_t>(b >> 1);
        const std::uint32_t high = static_cast<std::uint32_t>(a >> 16) + (sumLow < low ? 1u : 0u);
        quotient = Divide64By32(high, sumLow, static_cast<std::uint32_t>(b));
    }
    const auto result = static_cast<std::int32_t>(sign < 0 ? 0u - quotient : quotient);
    return result;
}

void Rotate(GuestVector& vector, std::int64_t angle) {
    GuestVector v = vector;
    if (angle == 0 || (v.x == 0 && v.y == 0)) {
        return;
    }
    const int shift = Prenormalize(v);
    PseudoRotate(v, angle);
    v.x = Downscale(v.x);
    v.y = Downscale(v.y);
    if (shift > 0) {
        const std::int64_t half = std::int64_t{1} << (shift - 1);
        vector.x = Add(Add(v.x, half), v.x >> 63) >> shift;
        vector.y = Add(Add(v.y, half), v.y >> 63) >> shift;
        return;
    }
    vector.x = ShiftLeft(v.x, -shift);
    vector.y = ShiftLeft(v.y, -shift);
}

std::int64_t Length(GuestVector vector) {
    if (vector.x == 0) {
        return Magnitude(vector.y);
    }
    if (vector.y == 0) {
        return Magnitude(vector.x);
    }
    const int shift = Prenormalize(vector);
    PseudoPolarize(vector);
    const std::int64_t length = Downscale(vector.x);
    if (shift > 0) {
        return Add(length, std::int64_t{1} << (shift - 1)) >> shift;
    }
    return ShiftLeft(length, -shift);
}

}

extern "C" {

std::int64_t APS5_VABI sceFreeTypeOtCos_nid_no_patch(std::int64_t angle) {
    return RotatedUnit(angle).x / (1 << 12);
}
APS5_EXPORT("yD4-QaNSink", sceFreeTypeOtCos_nid_no_patch);

std::int64_t APS5_VABI sceFreeTypeOtSin_nid_no_patch(std::int64_t angle) {
    return RotatedUnit(Subtract(AnglePi2, angle)).x / (1 << 12);
}
APS5_EXPORT("L1uHkWTazkI", sceFreeTypeOtSin_nid_no_patch);

std::int64_t APS5_VABI sceFreeTypeOtTan_nid_no_patch(std::int64_t angle) {
    const GuestVector vector = RotatedUnit(angle);
    return DivFix(vector.y, vector.x);
}
APS5_EXPORT("dojyaLKzKN0", sceFreeTypeOtTan_nid_no_patch);

std::int64_t APS5_VABI sceFreeTypeOtAtan2_nid_no_patch(std::int64_t dx, std::int64_t dy) {
    if (dx == 0 && dy == 0) {
        return 0;
    }
    GuestVector vector{dx, dy};
    Prenormalize(vector);
    PseudoPolarize(vector);
    return vector.y;
}
APS5_EXPORT("p90G4PB-rPc", sceFreeTypeOtAtan2_nid_no_patch);

void APS5_VABI sceFreeTypeOtVectorUnit_nid_no_patch(GuestVector* vector, std::int64_t angle) {
    if (!vector) {
        NotImplemented_nid_no_patch(__func__);
    }
    const GuestVector unit = RotatedUnit(angle);
    vector->x = unit.x >> 12;
    vector->y = unit.y >> 12;
}
APS5_EXPORT("3WpTQrvGUcU", sceFreeTypeOtVectorUnit_nid_no_patch);

void APS5_VABI sceFreeTypeOtVectorRotate_nid_no_patch(GuestVector* vector, std::int64_t angle) {
    if (!vector) {
        NotImplemented_nid_no_patch(__func__);
    }
    Rotate(*vector, angle);
}
APS5_EXPORT("6wJwUIVwAxg", sceFreeTypeOtVectorRotate_nid_no_patch);

std::int64_t APS5_VABI sceFreeTypeOtVectorLength_nid_no_patch(GuestVector* vector) {
    if (!vector) {
        NotImplemented_nid_no_patch(__func__);
    }
    return Length(*vector);
}
APS5_EXPORT("1jmC2QcpRl0", sceFreeTypeOtVectorLength_nid_no_patch);

void APS5_VABI sceFreeTypeOtVectorPolarize_nid_no_patch(GuestVector* vector, std::int64_t* length, std::int64_t* angle) {
    if (!vector) {
        NotImplemented_nid_no_patch(__func__);
    }
    GuestVector v = *vector;
    if (v.x == 0 && v.y == 0) {
        return;
    }
    if (!length || !angle) {
        NotImplemented_nid_no_patch(__func__);
    }
    const int shift = Prenormalize(v);
    PseudoPolarize(v);
    const std::int64_t scaled = Downscale(v.x);
    *length = shift >= 0 ? scaled >> shift : ShiftLeft(scaled, -shift);
    *angle = v.y;
}
APS5_EXPORT("HMcNe70D9TI", sceFreeTypeOtVectorPolarize_nid_no_patch);

void APS5_VABI sceFreeTypeOtVectorFromPolar_nid_no_patch(GuestVector* vector, std::int64_t length, std::int64_t angle) {
    if (!vector) {
        NotImplemented_nid_no_patch(__func__);
    }
    GuestVector v{length, 0};
    Rotate(v, angle);
    *vector = v;
}
APS5_EXPORT("2rmvbvOCyjY", sceFreeTypeOtVectorFromPolar_nid_no_patch);

std::int64_t APS5_VABI sceFreeTypeOtAngleDiff_nid_no_patch(std::int64_t first, std::int64_t second) {
    if ((first > 0 && second < std::numeric_limits<std::int64_t>::min() + first) ||
        (first < 0 && second > std::numeric_limits<std::int64_t>::max() + first)) {
        NotImplemented_nid_no_patch(__func__);
    }
    std::int64_t delta = (second - first) % Angle2Pi;
    if (delta < 0) {
        delta += Angle2Pi;
    }
    if (delta > AnglePi) {
        delta -= Angle2Pi;
    }
    return delta;
}
APS5_EXPORT("D1l8+Bu+-Y8", sceFreeTypeOtAngleDiff_nid_no_patch);

}
