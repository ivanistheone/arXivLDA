#ifndef WEBHTMM_CHECK_PROB_H__
#define WEBHTMM_CHECK_PROB_H__

#include <vector>

using namespace std;

namespace htmm {

  class CheckProb {
  public:
    static void CheckProbVector(const vector<double> &vec, const char *msg);
  };

}  // namespace

#endif
