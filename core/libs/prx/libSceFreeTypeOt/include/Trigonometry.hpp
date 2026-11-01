#pragma once

#include "prx/libSceFreeType/include/GuestFreeType.hpp"
#include "prx/libc/include/general/VabiMacros.hpp"

#include <cstdint>

extern "C" {

std::int64_t APS5_VABI sceFreeTypeOtCos_nid_no_patch(std::int64_t angle);
std::int64_t APS5_VABI sceFreeTypeOtSin_nid_no_patch(std::int64_t angle);
std::int64_t APS5_VABI sceFreeTypeOtTan_nid_no_patch(std::int64_t angle);
std::int64_t APS5_VABI sceFreeTypeOtAtan2_nid_no_patch(std::int64_t dx, std::int64_t dy);
void APS5_VABI sceFreeTypeOtVectorUnit_nid_no_patch(FreeType::GuestVector* vector, std::int64_t angle);
void APS5_VABI sceFreeTypeOtVectorRotate_nid_no_patch(FreeType::GuestVector* vector, std::int64_t angle);
std::int64_t APS5_VABI sceFreeTypeOtVectorLength_nid_no_patch(FreeType::GuestVector* vector);
void APS5_VABI sceFreeTypeOtVectorPolarize_nid_no_patch(FreeType::GuestVector* vector, std::int64_t* length, std::int64_t* angle);
void APS5_VABI sceFreeTypeOtVectorFromPolar_nid_no_patch(FreeType::GuestVector* vector, std::int64_t length, std::int64_t angle);
std::int64_t APS5_VABI sceFreeTypeOtAngleDiff_nid_no_patch(std::int64_t first, std::int64_t second);

}
