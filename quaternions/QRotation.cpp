#include "QRotation.h"

QRotation::QRotation(const FVector& v1, const FVector& v2) : QRotation() {
  float an=v1.norm();
  if (an==0.0f) return;
  FVector a=v1.mkUnitVector();
  float bn=v2.norm();
  if (bn==0.0f) return;
  FVector b=v2.mkUnitVector();
  FVector half=a; half+=b; half/=2.0f;
//  float n=half.norm();
//  if (n<0.00001f) {
if (0) {
    *this=Quaternion(0.0f, a.anyOrthogonal().mkUnitVector());
  } else {
    *this=Quaternion(0.0f, a)*Quaternion(0.0f, half.mkUnitVector());
    r=-r;
    // from definition: Quaternion(dot(u, half), cross(u, half));
  }
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
