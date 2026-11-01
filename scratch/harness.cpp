#include "prx/libSceFreeType/include/Api.hpp"
#include "prx/libSceFreeTypeOt/include/Trigonometry.hpp"
#include <cstdint>
#include <cstdio>
extern "C" {
int64_t ref_sin(int64_t); int64_t ref_cos(int64_t); int64_t ref_tan(int64_t); int64_t ref_atan2(int64_t,int64_t);
void ref_unit(int64_t*,int64_t); void ref_rotate(int64_t*,int64_t); int64_t ref_length(int64_t*);
void ref_polarize(int64_t*,int64_t*,int64_t*); void ref_from(int64_t*,int64_t,int64_t); int64_t ref_diff(int64_t,int64_t);
}
static uint64_t s = 0x9E3779B97F4A7C15ULL;
static uint64_t rnd(){ s ^= s << 13; s ^= s >> 7; s ^= s << 17; return s; }
static long long bad[16]; static long long total[16];
static void check(int k, bool ok, const char* what, long long a, long long b){ total[k]++; if(!ok){ if(bad[k]<3) std::printf("MISMATCH %s %lld %lld\n", what, a, b); bad[k]++; } }
int main(){
  const int64_t full = 360LL << 16;
  for (int64_t a = -2*full; a <= 2*full; a++) {
    check(0, FT_Sin_nid_postfix(a)==ref_sin(a), "sin", a, 0);
    check(1, FT_Cos_nid_postfix(a)==ref_cos(a), "cos", a, 0);
    check(2, FT_Tan_nid_postfix(a)==ref_tan(a), "tan", a, 0);
    FreeType::GuestVector u{0,0}; int64_t r[2]; FT_Vector_Unit_nid_postfix(&u, a); ref_unit(r, a);
    check(3, u.x==r[0] && u.y==r[1], "unit", a, 0);
  }
  for (int i = 0; i < 200000; i++) {
    int64_t a = (int64_t)(rnd() % (1ULL<<37)) - (1LL<<36);
    check(0, FT_Sin_nid_postfix(a)==ref_sin(a), "sinL", a, 0); check(2, FT_Tan_nid_postfix(a)==ref_tan(a), "tanL", a, 0);
    FreeType::GuestVector u{0,0}; int64_t ur[2]; FT_Vector_Unit_nid_postfix(&u, a); ref_unit(ur, a); check(3, u.x==ur[0] && u.y==ur[1], "unitL", a, 0);
    int64_t x = (int64_t)(rnd() % 2000001) - 1000000, y = (int64_t)(rnd() % 2000001) - 1000000;
    FreeType::GuestVector v{x,y}; int64_t r[2]={x,y}; FT_Vector_Rotate_nid_postfix(&v, a); ref_rotate(r, a);
    check(4, v.x==r[0] && v.y==r[1], "rotateL", x, a);
  }
  for (int i = 0; i < 20000000; i++) {
    int sh = 1 + (int)(rnd() % 61); uint64_t m = (1ULL << sh) - 1;
    int64_t x = (int64_t)(rnd() % (2*m+1) - m), y = (int64_t)(rnd() % (2*m+1) - m);
    if (i % 7 == 0) x = 0;
    if (i % 11 == 0) y = 0;
    int64_t a = (int64_t)(rnd() % (8*full)) - 4*full;
    FreeType::GuestVector v{x,y}; int64_t r[2]={x,y};
    FT_Vector_Rotate_nid_postfix(&v, a); ref_rotate(r, a); check(4, v.x==r[0] && v.y==r[1], "rotate", x, y);
    FreeType::GuestVector w{x,y}; int64_t q[2]={x,y};
    check(5, FT_Vector_Length_nid_postfix(&w)==ref_length(q), "length", x, y);
    int64_t l1=5,a1=6,l2=5,a2=6; FT_Vector_Polarize_nid_postfix(&w,&l1,&a1); ref_polarize(q,&l2,&a2); check(6, l1==l2 && a1==a2, "polarize", x, y);
    check(7, FT_Atan2_nid_postfix(x,y)==ref_atan2(x,y), "atan2", x, y);
    FreeType::GuestVector f{0,0}; int64_t g[2]; FT_Vector_From_Polar_nid_postfix(&f, x, a); ref_from(g, x, a); check(8, f.x==g[0] && f.y==g[1], "from_polar", x, a);
    int64_t d1 = (int64_t)(rnd() % (1ULL<<50)) - (1LL<<49), d2 = (int64_t)(rnd() % (1ULL<<50)) - (1LL<<49);
    check(9, FT_Angle_Diff_nid_postfix(d1,d2)==ref_diff(d1,d2), "angle_diff", d1, d2);
  }
  const char* names[] = {"FT_Sin","FT_Cos","FT_Tan","FT_Vector_Unit","FT_Vector_Rotate","FT_Vector_Length","FT_Vector_Polarize","FT_Atan2","FT_Vector_From_Polar","FT_Angle_Diff"};
  for (int k = 0; k < 10; k++) std::printf("%-22s %lld checked, %lld mismatches\n", names[k], total[k], bad[k]);
  return 0;
}
