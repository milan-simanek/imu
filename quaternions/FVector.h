#ifndef __FVECTOR_H__
#define __FVECTOR_H__

#include <math.h>
#include <iostream>

using namespace std;

class QRotation;

class FVector {
    public:
        float x,y,z;
        FVector() : x(0.0f), y(0.0f), z(0.0f) { }
        FVector(float vx, float vy, float vz) : x(vx), y(vy), z(vz) { }
        FVector(const FVector &v) : x(v.x), y(v.y), z(v.z) { }
        FVector(istream &is);
        
        FVector& operator=(const FVector& v) {if (this!=&v) {x=v.x; y=v.y; z=v.z;}; return *this; }
        bool operator==(const FVector& v) const { return (x==v.x) && (y==v.y) && (z==v.z); }
        bool operator!=(const FVector& v) const { return !(*this == v); }
        
        FVector operator+(const FVector& v) const { return FVector(x+v.x, y+v.y, z+v.z); }
        FVector& operator+=(const FVector& v) {x+=v.x; y+=v.y; z+=v.z; return *this; }
        FVector operator-(const FVector& v) const { return FVector(x-v.x, y-v.y, z-v.z); }
        FVector& operator-=(const FVector& v) {x-=v.x; y-=v.y; z-=v.z; return *this; }
        
        FVector operator*(float s) const { return FVector(x*s, y*s, z*s); }
        FVector operator*(FVector v) const { return FVector(x*v.x, y*v.y, z*v.z); }
        virtual FVector& operator*=(float s) { x*=s; y*=s; z*=s; return *this; }
        FVector operator/(float a) const { return (*this)*(1/a); }
        FVector& operator/=(float a)     { return (*this)*=(1/a); }
        
        void raiseUp(const FVector& v)   { if (x<v.x) x=v.x; if (y<v.y) y=v.y; if (z<v.z) z=v.z; }
        void raiseDown(const FVector& v) { if (x>v.x) x=v.x; if (y>v.y) y=v.y; if (z>v.z) z=v.z; }
        
        FVector rotateBy(const QRotation &Rot) const;
        FVector rotateBackBy(const QRotation &Rot) const;
        FVector crossProduct(const FVector& v) const;
        float dotProduct(const FVector& v) const { return x*v.x+y*v.y+z*v.z; }
        FVector mkUnitVector() const { return (*this)/norm(); }
        float norm2() const { return dotProduct(*this); }
        float norm() const { return sqrtf(norm2()); }
        inline FVector inverse() const { return FVector(-x,-y,-z); }
        FVector& selfInverse() { x=-x; y=-y; z=-z; return *this; }
        FVector anyOrthogonal() const;
        virtual void print(const char *msg="") const;
        virtual void print(ostream& os, const char *msg="") const;
        virtual void write(ostream& os) const;
};

static inline ostream& operator<<(ostream& os, const FVector &v) { v.write(os); return os; }
static inline bool operator>>(istream& is, FVector &v) { v=FVector(is); return true; }

#endif
