#ifndef __QROTATION_H__
#define __QROTATION_H__

#include <math.h>

#include "FVector.h"
#include "Quaternion.h"

using namespace std;

class QRotation : public Quaternion {	// QRotation = Versor = normalized quaternion
    public:
        QRotation() : Quaternion(1.0f, 0.0f, 0.0f, 0.0f) { }	// no rotation
        QRotation(const Quaternion &q) : Quaternion(q) { }	// copy constructor
        QRotation(const FVector& v1, const FVector& v2);	// from v1 to v2
        QRotation(const FVector &angle3D);			// 3 small angles clockwise in the direction of the axis

        QRotation& operator=(const Quaternion& q)	{ Quaternion::operator=(q); return *this; }
        QRotation operator*(float scale) const;
        QRotation operator*(QRotation& R) const		{ return QRotation(Quaternion::operator*(R)); }
};

#endif

