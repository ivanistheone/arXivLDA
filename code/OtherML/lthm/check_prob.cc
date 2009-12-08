#include <math.h>
#include <iostream>
#include "check_prob.h"
#include "error.h"

using namespace std;

namespace htmm {

// Checks that the vector vec is a probability vector
void CheckProb::CheckProbVector(const vector<double> &vec, const char *msg) {
  double sum = 0;
  for (int i = 0; i < vec.size(); i++) {
    if ((vec[i] < 0) || (vec[i] > 1) || (!isfinite(vec[i]))) {
      ERROR(msg << endl << "CheckProbVector: vec[" << i << "] = " << vec[i]);
    }
    sum += vec[i];
  }
  if (fabs(sum-1) > 1e-4) {
    ERROR(msg << endl << "CheckProbVector: vector doesn't sum to 1, sum = "
	  << sum);
  }
}

}  // namespace
