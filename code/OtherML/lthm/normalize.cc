#include <vector>
#include "normalize.h"

// Divides each entry of the vector v by the constant norm.
void Normalize(double norm, vector<double> *v) {
  double invnorm = 1.0 / norm;
  double* vp = &(*v)[0];
  for (int i = 0; i < v->size(); i++) {
    *vp++ *= invnorm;
  }
}
