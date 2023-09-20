#include "quaternion.h"
#include <iostream>

FVector::FVector(istream& is) {
  char c;
  while (is.good()) {
    is >> c; if (std::isspace(c)) continue;
    if (c!='(') break;
    if (is >> x) if (is >> y) if (is >> z) return;
    break;
  }
  cerr << "Error reading FVector";
}
FVector FVector::crossProduct(const FVector& v) const {
  return FVector(     
      (y*v.z) - (z*v.y),
      (z*v.x) - (x*v.z),
      (x*v.y) - (y*v.x));
}
void FVector::write(ostream& os) const {
  os << '[' << x << ',' << y << ',' << z << ']';
}


FVector FVector::rotateBy(const QRotation &Rot) const {
  const quaternion r=Rot;
  return r*quaternion(0.0f, *this)*(Rot.inverse());
}
FVector FVector::rotateBackBy(const QRotation &Rot) const {
  return (Rot.inverse())*(quaternion(0.0f, *this))*Rot;
}
FVector FVector::anyOrthogonal() const {
  float xx = abs(x);
  float yy = abs(y);
  float zz = abs(z);
  FVector other = xx < zz ? (xx < yy ? FVector(1.0f, 0.0f, 0.0f) 
                                     : FVector(0.0f, 1.0f, 0.0f))
                          : (yy < zz ? FVector(0.0f, 1.0f, 0.0f)
                                     : FVector(0.0f, 0.0f, 1.0f));
  return crossProduct(other);
}

