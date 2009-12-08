// Copyright 2007 Google Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: agdevhtmm@gmail.com (Amit Gruber)

#include <vector>
#include "fast_restricted_viterbi.h"


using namespace std;

namespace htmm {

void FastRestrictedViterbi::Viterbi(double epsilon,
                                        const vector<double> &theta,
                                        const vector<vector<double> > &local,
                                        const vector<double> &pi,
                                        vector<int> *best_path) {
  T_ = local.size();
  topics_ = theta.size();
  states_ = 2*topics_;
  vector<double> dummy_double(states_);
  vector<vector<double> > delta(T_, dummy_double);
  vector<int> dummy_int(states_);
  vector<vector<int> > best(T_, dummy_int);
  ComputeAllDeltas(pi, local, theta, epsilon, &delta, &best);
  BackTrackBestPath(delta[T_-1], best, best_path);
}

void FastRestrictedViterbi::InitDelta(const vector<double> &pi,
                                      const vector<double> &local0,
                                      vector<double> *delta0) {
  double norm = 0;
  for (int i = 0;i < topics_;i++) {
    // this is the beginning of the path, no need to initialize best
    (*delta0)[i] = pi[i]*local0[i];
    (*delta0)[i+topics_] = pi[i+topics_]*local0[i];
    norm += (*delta0)[i] + (*delta0)[i+topics_];
  }
  Normalize(norm, delta0);
}

// compute delta[i] for i>=1 (delta[0] has been initialized)
// NOTICE that this is not the standard delta computation, we are taking
// advantage of the special structure of our problem!
void FastRestrictedViterbi::ComputeAllDeltas(const vector<double> &pi,
                                             const vector<vector<double> >
                                             &local,
                                             const vector<double> &theta,
                                             double epsilon,
                                             vector<vector<double> > *delta,
                                             vector<vector<int> > *best) {
  InitDelta(pi, local[0], &((*delta)[0]));
  for (int i = 1;i < T_;i++) {
    ComputeSingleDelta(local[i], theta, epsilon, (*delta)[i-1],
                       &((*delta)[i]), &((*best)[i]));
  }
}



// delta[t][s] corresponds to a new lottery when picking the topic s at time t.
// delta[t][s+topics] corresponds to setting the topic at time t by copying the
// topic s from time t-1.
void FastRestrictedViterbi::ComputeSingleDelta(const vector<double> &local_t,
                                               const vector<double> &theta,
                                               double epsilon,
                                               const vector<double> &delta_t_1,
                                               vector<double> *delta_t,
                                               vector<int> *best) {
  int prev_best = -1;
  FindBestInLevel(delta_t_1, &prev_best);
  double norm = 0;
  for (int s = 0;s < topics_;s++) {
    // If there was no topic transition:
    (*delta_t)[s] = delta_t_1[prev_best]*epsilon*theta[s]*local_t[s];
    (*best)[s] = prev_best;
    // In case of topic transition
    if (delta_t_1[s] > delta_t_1[s+topics_]) {
      (*delta_t)[s+topics_] = delta_t_1[s]*(1-epsilon)*local_t[s];
      (*best)[s+topics_] = s;
    } else {
      (*delta_t)[s+topics_] = delta_t_1[s+topics_]*(1-epsilon)*local_t[s];
      (*best)[s+topics_] = s+topics_;
    }
    norm += (*delta_t)[s]+(*delta_t)[s+topics_];
  }
  Normalize(norm, delta_t);
}

void FastRestrictedViterbi::FindBestInLevel(const vector<double> &delta_t_1,
                                            int *prev_best) {
  double best_score = -1;
  for (int i = 0;i < states_;i++) {
    if (delta_t_1[i] > best_score) {
      best_score = delta_t_1[i];
      *prev_best = i;
    }
  }
}

// After we've found the best path ending at each state at each level, we back
// track to find the best path amnong them all.
void FastRestrictedViterbi::BackTrackBestPath(const vector<double> &delta_T_1,
                                              const vector<vector<int> > &best,
                                              vector<int> *best_path) {
  FindBestInLevel(delta_T_1, &((*best_path)[T_-1]));
  for (int i = T_-2;i >= 0;i--) {
    (*best_path)[i] = best[i+1][(*best_path)[i+1]];
  }
}

void FastRestrictedViterbi::Normalize(double norm, vector<double> *vec) {
  for (int i = 0;i < vec->size();i++) {
    (*vec)[i] /= norm;
  }
}

}  // namespace
