#include "prx/libSceFreeType/include/Api.hpp"
#include "prx/libSceFreeTypeOt/include/Trigonometry.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace {

struct AngleCase {
    std::int64_t angle;
    std::int64_t sine;
    std::int64_t cosine;
    std::int64_t tangent;
    std::int64_t unitX;
    std::int64_t unitY;
};

struct VectorCase {
    std::int64_t x;
    std::int64_t y;
    std::int64_t length;
    std::int64_t polarLength;
    std::int64_t polarAngle;
    std::int64_t atan2;
};

struct RotationCase {
    std::int64_t x;
    std::int64_t y;
    std::int64_t angle;
    std::int64_t rotatedX;
    std::int64_t rotatedY;
};

struct PolarCase {
    std::int64_t length;
    std::int64_t angle;
    std::int64_t x;
    std::int64_t y;
};

struct DiffCase {
    std::int64_t first;
    std::int64_t second;
    std::int64_t difference;
};

constexpr AngleCase Angles[] = {
    {0, 0, 65536, 0, 65536, -1},
    {1, 0, 65536, 0, 65536, 0},
    {-1, 0, 65536, 0, 65536, 0},
    {32768, 571, 65533, 572, 65533, 571},
    {1966080, 32768, 56755, 37837, 56755, 32768},
    {2949120, 46340, 46340, 65536, 46340, 46340},
    {3932160, 56755, 32768, 113512, 32768, 56755},
    {5832704, 65526, 1143, 3754679, 1143, 65526},
    {5898240, 65536, 0, -2147483647, -1, 65536},
    {5963776, 65526, -1143, -3754679, -1144, 65526},
    {8847360, 46340, -46340, -65536, -46341, 46340},
    {11796480, 0, -65536, 0, -65537, 0},
    {-5898240, -65536, 0, -2147483647, 0, -65537},
    {-11796480, 0, -65536, 0, -65537, 0},
    {17694720, -65536, 0, -2147483647, 0, -65537},
    {23527424, -1143, 65526, -1144, 65526, -1144},
    {23592960, 0, 65536, 0, 65536, -1},
    {-23592960, 0, 65536, 0, 65536, -1},
    {47198265, 215, 65535, 215, 65535, 215},
    {-65536007, 64540, 11380, 371670, 11380, 64540},
    {4886718345, 46666, 46012, 66468, 46012, 46666},
    {-4886718345, -46666, 46012, -66468, 46012, -46667},
    {1099511627776, 45525, -47142, -63287, -47143, 45525},
};

constexpr VectorCase Vectors[] = {
    {1, 0, 1, 1, 0, 0},
    {0, -5, 5, 5, -5898240, -5898240},
    {3, 4, 5, 5, 3481920, 3481920},
    {-3, 4, 5, 5, 8314560, 8314560},
    {-7, -9, 11, 11, -8380416, -8380416},
    {65536, 65536, 92682, 92681, 2949120, 2949120},
    {1048576, -262144, 1080847, 1080847, -919872, -919872},
    {2147483647, 1, 2147483640, 2147483640, 0, 0},
    {1099511627776, 3, 14528925312, 14528925312, 0, 0},
    {-34359738368, 17179869184, 38414306768, 38414306768, 10055520, 10055520},
    {123456789, -987654, 123460741, 123460740, -30048, -30048},
};

constexpr RotationCase Rotations[] = {
    {1, 0, 1966080, 1, 1},
    {1, 0, -2949120, 1, -1},
    {1, 0, 11796480, -1, 0},
    {1, 0, 1193046, 1, 0},
    {0, -5, 1966080, 3, -4},
    {0, -5, -2949120, -4, -4},
    {0, -5, 11796480, 0, 5},
    {0, -5, 1193046, 2, -5},
    {3, 4, 1966080, 1, 5},
    {3, 4, -2949120, 5, 1},
    {3, 4, 11796480, -3, -4},
    {3, 4, 1193046, 2, 5},
    {-3, 4, 1966080, -5, 2},
    {-3, 4, -2949120, 1, 5},
    {-3, 4, 11796480, 3, -4},
    {-3, 4, 1193046, -4, 3},
    {-7, -9, 1966080, -2, -11},
    {-7, -9, -2949120, -11, -1},
    {-7, -9, 11796480, 7, 9},
    {-7, -9, 1193046, -4, -11},
    {65536, 65536, 1966080, 23988, 89524},
    {65536, 65536, -2949120, 92682, 0},
    {65536, 65536, 11796480, -65536, -65536},
    {65536, 65536, 1193046, 41782, 82730},
    {1048576, -262144, 1966080, 1039165, 297265},
    {1048576, -262144, -2949120, 556091, -926819},
    {1048576, -262144, 11796480, -1048576, 262144},
    {1048576, -262144, 1193046, 1077989, 78561},
    {2147483647, 1, 1966080, 1859775336, 1073741880},
    {2147483647, 1, -2949120, 1518500312, -1518500192},
    {2147483647, 1, 11796480, -2147483672, 424},
    {2147483647, 1, 1193046, 2039997520, 670892104},
    {1099511627776, 3, 1966080, 34142699616, 7264488768},
    {1099511627776, 3, -2949120, 26329232864, -26329184176},
    {1099511627776, 3, 11796480, -14528925360, 209280},
    {1099511627776, 3, 1193046, 1226117408, 9655939408},
    {-34359738368, 17179869184, 1966080, -38345292240, -2301667040},
    {-34359738368, 17179869184, -2949120, -12148003104, 36442956976},
    {-34359738368, 17179869184, 11796480, 34359735056, -17179875712},
    {-34359738368, 17179869184, 1193046, -38007096864, 5585705792},
    {123456789, -987654, 1966080, 107410541, 60873065},
    {123456789, -987654, -2949120, 86598759, -87995507},
    {123456789, -987654, 11796480, -123456789, 987678},
    {123456789, -987654, 1193046, 117586066, 37630728},
};

constexpr PolarCase Polars[] = {
    {65536, 0, 65536, 0},
    {65536, 1966080, 56756, 32768},
    {65536, -7864320, -32768, -56756},
    {65536, 1193046, 62256, 20474},
    {1048576, 0, 1048576, 0},
    {1048576, 1966080, 908093, 524288},
    {1048576, -7864320, -524288, -908093},
    {1048576, 1193046, 996093, 327584},
    {3, 0, 3, 0},
    {3, 1966080, 3, 2},
    {3, -7864320, -2, -3},
    {3, 1193046, 3, 1},
    {2147483647, 0, 2147483647, 0},
    {2147483647, 1966080, 1859775336, 1073741880},
    {2147483647, -7864320, -1073741856, -1859775368},
    {2147483647, 1193046, 2039997520, 670892104},
    {1099511627776, 0, 1099511627776, 0},
    {1099511627776, 1966080, 34142699616, 7264488768},
    {1099511627776, -7864320, -7264488736, -34142699664},
    {1099511627776, 1193046, 1226117408, 9655939408},
};

constexpr DiffCase Diffs[] = {
    {0, 655360, 655360},
    {655360, 0, -655360},
    {0, 12451840, -11141120},
    {0, -12451840, 11141120},
    {22937600, 655360, 1310720},
    {1099511627776, 5, -8912891},
    {-1099511627776, 1099511627776, -5767168},
};

void Require(bool condition, const char* message, std::int64_t input) {
    if (condition) return;
    std::fprintf(stderr, "FreeTypeOt trigonometry: %s (input %lld)\n", message, static_cast<long long>(input));
    std::exit(1);
}

template <typename TCall> bool Throws(TCall call) {
    try {
        call();
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

void CheckAngles() {
    for (const AngleCase& test : Angles) {
        Require(FT_Sin_nid_postfix(test.angle) == test.sine, "FT_Sin matches FreeType 2.4.11", test.angle);
        Require(FT_Cos_nid_postfix(test.angle) == test.cosine, "FT_Cos matches FreeType 2.4.11", test.angle);
        Require(FT_Tan_nid_postfix(test.angle) == test.tangent, "FT_Tan matches FreeType 2.4.11", test.angle);
        FreeType::GuestVector unit{7, 7};
        FT_Vector_Unit_nid_postfix(&unit, test.angle);
        Require(unit.x == test.unitX && unit.y == test.unitY, "FT_Vector_Unit matches FreeType 2.4.11", test.angle);
    }
}

void CheckVectors() {
    for (const VectorCase& test : Vectors) {
        FreeType::GuestVector vector{test.x, test.y};
        Require(FT_Vector_Length_nid_postfix(&vector) == test.length, "FT_Vector_Length matches FreeType 2.4.11", test.x);
        Require(vector.x == test.x && vector.y == test.y, "FT_Vector_Length leaves the vector unchanged", test.x);
        std::int64_t length = -1;
        std::int64_t angle = -1;
        FT_Vector_Polarize_nid_postfix(&vector, &length, &angle);
        Require(length == test.polarLength && angle == test.polarAngle, "FT_Vector_Polarize matches FreeType 2.4.11", test.x);
        Require(FT_Atan2_nid_postfix(test.x, test.y) == test.atan2, "FT_Atan2 matches FreeType 2.4.11", test.x);
    }
    for (const RotationCase& test : Rotations) {
        FreeType::GuestVector vector{test.x, test.y};
        FT_Vector_Rotate_nid_postfix(&vector, test.angle);
        Require(vector.x == test.rotatedX && vector.y == test.rotatedY, "FT_Vector_Rotate matches FreeType 2.4.11", test.x);
    }
    for (const PolarCase& test : Polars) {
        FreeType::GuestVector vector{7, 7};
        FT_Vector_From_Polar_nid_postfix(&vector, test.length, test.angle);
        Require(vector.x == test.x && vector.y == test.y, "FT_Vector_From_Polar matches FreeType 2.4.11", test.length);
    }
}

void CheckEdges() {
    FreeType::GuestVector zero{0, 0};
    std::int64_t length = 11;
    std::int64_t angle = 13;
    FT_Vector_Polarize_nid_postfix(&zero, &length, &angle);
    Require(length == 11 && angle == 13, "FT_Vector_Polarize leaves the outputs of a zero vector unwritten", 0);
    FT_Vector_Polarize_nid_postfix(&zero, nullptr, nullptr);
    Require(FT_Atan2_nid_postfix(0, 0) == 0, "FT_Atan2 of a zero vector is 0", 0);
    FreeType::GuestVector vector{3, 4};
    FT_Vector_Rotate_nid_postfix(&vector, 0);
    Require(vector.x == 3 && vector.y == 4, "FT_Vector_Rotate by 0 keeps the vector", 0);
    FT_Vector_Rotate_nid_postfix(&zero, 1966080);
    Require(zero.x == 0 && zero.y == 0, "FT_Vector_Rotate keeps a zero vector", 0);
    Require(Throws([] { FT_Vector_Unit_nid_postfix(nullptr, 0); }), "FT_Vector_Unit throws for a null vector", 0);
    Require(Throws([] { FT_Vector_Rotate_nid_postfix(nullptr, 1); }), "FT_Vector_Rotate throws for a null vector", 0);
    Require(Throws([] { FT_Vector_Length_nid_postfix(nullptr); }), "FT_Vector_Length throws for a null vector", 0);
    Require(Throws([] { FT_Vector_From_Polar_nid_postfix(nullptr, 1, 0); }), "FT_Vector_From_Polar throws for a null vector", 0);
    Require(Throws([&] { FT_Vector_Polarize_nid_postfix(&vector, nullptr, &angle); }) &&
                Throws([&] { FT_Vector_Polarize_nid_postfix(&vector, &length, nullptr); }) && length == 11 && angle == 13,
        "FT_Vector_Polarize throws for a null output of a nonzero vector without writing", 0);
    for (const DiffCase& test : Diffs) {
        Require(FT_Angle_Diff_nid_postfix(test.first, test.second) == test.difference, "FT_Angle_Diff matches FreeType 2.4.11", test.first);
    }
    Require(Throws([] { FT_Angle_Diff_nid_postfix(INT64_MIN, 1); }) && Throws([] { FT_Angle_Diff_nid_postfix(1, INT64_MIN); }),
        "FT_Angle_Diff throws where the signed subtraction overflows", 0);
}

}  // namespace

int main() {
    CheckAngles();
    CheckVectors();
    CheckEdges();
    std::puts("Guest FreeTypeOt trigonometry tests passed");
    return 0;
}
