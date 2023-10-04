#ifndef __IVECTOR_H__
#define __IVECTOR_H__

#include "FVector.h"

using namespace std;

class IVector {
    public:
        int x,y,z;
        IVector() : x(0), y(0), z(0) { }
        IVector(int vx, int vy, int vz) : x(vx), y(vy), z(vz) { }
        IVector(const IVector &v) : x(v.x), y(v.y), z(v.z) { }
        
        IVector& operator=(const IVector& v) {if (this!=&v) {x=v.x; y=v.y; z=v.z;}; return *this; }
        bool operator==(const IVector& v) const { return (x==v.x) && (y==v.y) && (z==v.z); }
        bool operator!=(const IVector& v) const { return !(*this == v); }
        
        IVector operator+(const IVector& v) const { return IVector(x+v.x, y+v.y, z+v.z); }
        IVector operator-(const IVector& v) const { return IVector(x-v.x, y-v.y, z-v.z); }
        IVector& operator+=(const IVector& v)       {x+=v.x; y+=v.y; z+=v.z; return *this; }
        IVector& operator-=(const IVector& v) {x-=v.x; y-=v.y; z-=v.z; return *this; }
        
        void raiseUp(const IVector& v)   { if (x<v.x) x=v.x; if (y<v.y) y=v.y; if (z<v.z) z=v.z; }
        void raiseDown(const IVector& v) { if (x>v.x) x=v.x; if (y>v.y) y=v.y; if (z>v.z) z=v.z; }

        
        FVector operator*(float s) const { return FVector(x*s, y*s, z*s); }
        FVector operator/(float a) const { return (*this)*(1/a); }
        
        FVector operator*(const FVector &s) { return FVector(x*s.x, y*s.y, z*s.z); }
        virtual void write(ostream& os) const;
};

static inline FVector operator/(float a, const IVector& v) { return FVector(a/v.x, a/v.y, a/v.z); }
static inline ostream& operator<<(ostream& os, const IVector &v) { v.write(os); return os; }
#endif
