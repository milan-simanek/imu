#include "Quaternion.h"

using namespace std;

Quaternion Quaternion::operator*(const Quaternion& q) const { // Hamilton product
    return Quaternion(
      (q.r * r) - (q.x * x) - (q.y * y) - (q.z * z),
      (q.r * x) + (q.x * r) + (q.y * z) - (q.z * y),
      (q.r * y) + (q.y * r) + (q.z * x) - (q.x * z),
      (q.r * z) + (q.z * r) + (q.x * y) - (q.y * x));
}
// normalizes the Quaternion
Quaternion& Quaternion::normalize() {
    const float n = norm2();
    if (n == 1.0f) return *this;
    return (*this /= sqrtf(n));
}

float Quaternion::dotProduct(const Quaternion& q) const {
    return (r*q.r) + FVector::dotProduct(q);
}
