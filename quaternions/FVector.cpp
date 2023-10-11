#include <iostream>
#include "FVector.h"
#include "Quaternion.h"
#include "QRotation.h"

FVector::FVector(istream& is) {
  while (is.good()) {
    is.ignore(256, '[');
    if (is >> x)
    if (is.ignore(256, ',') >> y)
    if (is.ignore(256, ',') >> z) {
      is.ignore(256, ']');
      return;
    }
    break;
  }
  cerr << "Error reading FVector:" << x << "," << y << "," << z << endl;;
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
  const Quaternion r=Rot;
  return r*Quaternion(0.0f, *this)*(Rot.inverse());
}
FVector FVector::rotateBackBy(const QRotation &Rot) const {
  return (Rot.inverse())*(Quaternion(0.0f, *this))*Rot;
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

