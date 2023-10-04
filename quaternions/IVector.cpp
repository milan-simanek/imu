#include <iostream>
#include "IVector.h"

void IVector::write(ostream& os) const {
  os << '[' << x << ',' << y << ',' << z << ']';
}

