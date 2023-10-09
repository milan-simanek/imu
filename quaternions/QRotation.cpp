#include "QRotation.h"

QRotation::QRotation(const FVector& v1, const FVector& v2) : QRotation() {
  /* For any NORMALIZED v1 and v2: Q(dot(v1,v2), cross(v1,v2)) results in DOUBLE rotation
       result2=Q(dot(v1.normalize(),v2.normalize()), cross(v1.normalize(),v2.normalize()))
     The real rotation is half-way between double-rotation and zero-rotation rot0=Q(1,0,0,0)
       (both doble-rotation and zero-rotation have to be normalized and the result has to be normalized)
       result=Q(dot(v1.normalize(),v2.normalize())+1, cross(v1.normalize(),v2.normalize())).normalize();
     Optimization 1: instead of normalizing v1 and v2, we can calculate products scaled by factor k=|v1|*|v2|
       result=Q(dot(v1,v2)+|v1|*|v2|, cross(v1,v2)).normalize();
     Optimization 2: instead of |v1|*|v2|=sqrt(v1.v1)*sqrt(v2.v2) we can calculate |v1|*|v2|=sqrt(v1.v1 * v2.v2)
  */
  float k=sqrtf(v1.norm2()*v2.norm2());
  float k_cos_theta=v1.dotProduct(v2);
  if (k_cos_theta/k == -1)
    *this=Quaternion(0, v1.anyOrthogonal().mkUnitVector());
  else
    *this=Quaternion(k_cos_theta+k, v1.crossProduct(v2)).normalize();
}

// create a rotation by 3 small angles clockwise in the direction of the axis
QRotation::QRotation(const FVector &angle3D) : Quaternion(0.0f, angle3D) {
  float theta=FVector::norm();	// theta means the summary angle of rotation
  if (theta==0.0f) *this=QRotation(); else {
    float halftheta=theta/2;
    FVector::operator*=(sin(halftheta)/theta);
    r=cos(halftheta);
  }
}

QRotation QRotation::operator*(float scale) const {
  float htheta=atan2(FVector::norm(), r);
  float new_htheta=htheta*scale;
  float vscale=(htheta<0.01) ? (new_htheta<0.01) ? scale 
                                                 : sin(new_htheta)/htheta
                             : sin(new_htheta)/sin(htheta);
  return QRotation(Quaternion(cos(new_htheta), FVector::operator*(vscale)));
}
