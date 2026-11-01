#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRIGONOMETRY_H
#include <stdint.h>
int64_t ref_sin(int64_t a){return FT_Sin(a);}
int64_t ref_cos(int64_t a){return FT_Cos(a);}
int64_t ref_tan(int64_t a){return FT_Tan(a);}
int64_t ref_atan2(int64_t x,int64_t y){return FT_Atan2(x,y);}
void ref_unit(int64_t* v,int64_t a){FT_Vector r; FT_Vector_Unit(&r,a); v[0]=r.x; v[1]=r.y;}
void ref_rotate(int64_t* v,int64_t a){FT_Vector r={v[0],v[1]}; FT_Vector_Rotate(&r,a); v[0]=r.x; v[1]=r.y;}
int64_t ref_length(int64_t* v){FT_Vector r={v[0],v[1]}; return FT_Vector_Length(&r);}
void ref_polarize(int64_t* v,int64_t* l,int64_t* a){FT_Vector r={v[0],v[1]}; FT_Fixed ll=*l; FT_Angle aa=*a; FT_Vector_Polarize(&r,&ll,&aa); *l=ll; *a=aa;}
void ref_from(int64_t* v,int64_t l,int64_t a){FT_Vector r; FT_Vector_From_Polar(&r,l,a); v[0]=r.x; v[1]=r.y;}
int64_t ref_diff(int64_t a,int64_t b){return FT_Angle_Diff(a,b);}
