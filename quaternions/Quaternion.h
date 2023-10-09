#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include <math.h>

#include "FVector.h"

class Quaternion : public FVector {
    public:
    float r;
        Quaternion(float re, float qx, float qy, float qz) : FVector(qx,qy,qz), r(re) { }
        Quaternion(float re, const FVector &v) : FVector(v), r(re) { }
        Quaternion(const Quaternion &q) : FVector(q), r(q.r) { }
        Quaternion(istream &is);

        Quaternion& operator=(const Quaternion& q) { r=q.r; FVector::operator=(q); return *this; }
        bool operator==(const Quaternion& q) const { return (r==q.r) && FVector::operator==(q); }
        bool operator!=(const Quaternion& q) const { return !(*this == q); }
        
        Quaternion operator+(const Quaternion& q) const {return Quaternion(r+q.r, FVector::operator+(q)); }
        Quaternion& operator+=(const Quaternion& q) {r+=q.r; FVector::operator+=(q); return *this; }

        Quaternion operator*(float s) const { return Quaternion(r*s, FVector::operator*(s)); }
        Quaternion& operator*=(float s)	    { r*=s; FVector::operator*=(s); return *this; }
        Quaternion operator/(float a) const { return (*this)*(1/a); }
        Quaternion& operator/=(float a)     { return (*this)*=(1/a); }

        Quaternion operator*(const Quaternion& q) const;
        Quaternion& operator*=(const Quaternion& q) { return *this = *this * q; }
        Quaternion& selfInverse() { FVector::selfInverse(); return *this; }
        inline Quaternion inverse() const { return Quaternion(r,-x,-y,-z); }
        Quaternion& normalize();
        float dotProduct(const Quaternion& q) const;
        float norm2() const { return r*r+x*x+y*y+z*z; }
        float norm() const { return sqrtf(norm2()); }
        virtual void write(ostream& os) const;
};

static inline ostream& operator<<(ostream& os, const Quaternion &v) { v.write(os); return os; }
static inline bool operator>>(istream& is, Quaternion &v) { v=Quaternion(is); return true; }

#endif

