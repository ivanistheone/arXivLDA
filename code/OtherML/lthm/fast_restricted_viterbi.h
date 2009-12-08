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

#ifndef OPENHTMM_FAST_RESTRICTED_VITERBI_H__
#define OPENHTMM_FAST_RESTRICTED_VITERBI_H__

#include <vector>

// Find the most probable path in a Hidden Markov Model
// with a special form transition matrix.
// The run time is O(T*N)

// Assumptions:
// (1) The transition matrix is of the special form described in the HTMM
// paper, i.e. consists of entries that depend on theta and epsilon only
// (2) best_path is a pre allocated vector of length T
// (3) theta is a probability vector of length topics.
// (4) local is a vector of T probability vectors, each of which is of length
// topics and contains the emissions probabilities.
// (5) pi is a probability vector of length states=2*topics that specifies the
// prior distribution on the first hidden state in the chain.


using namespace std;

namespace htmm {

class FastRestrictedViterbi {
 public:
  FastRestrictedViterbi() {}
  ~FastRestrictedViterbi() {}
  // Run the Viterbi algorithm on the HMM specified by the arguments of the
  // method (see details above) and return the MAP estimator in the vector
  // best_path
  void Viterbi(double epsilon, const vector<double> &theta,
               const vector<vector<double> > &local,
               const vector<double> &pi,
               vector<int> *best_path);
 private:
  // The length of the Markov chain
  int T_;
  // Number of topics
  int topics_;
  // Number of states which is always 2*topics_ (for psi=0/1)
  int states_;
  // Find the state at which the most probable path up to a certain
  // level ends.
  void FindBestInLevel(const vector<double> &delta_t_1, int *prev_best);
  // Compute the probability of the most probable path ending in each node
  // at a certain level t. For each node at level t store the previous
  // node in the most probable path (the node at level t-1).
  void ComputeSingleDelta(const vector<double> &local_t,
                          const vector<double> &theta, double epsilon,
                          const vector<double> &delta_t_1,
                          vector<double> *delta_t, vector<int> *best);
  // Perofrm a single forward path of the Viterbi algorithm on the HMM
  void ComputeAllDeltas(const vector<double> &pi,
                        const vector<vector<double> > &local,
                        const vector<double> &theta, double epsilon,
                        vector<vector<double> > *delta,
                        vector<vector<int> > *best);
  // Initialize the Viterbi algorithm for level 0.
  void InitDelta(const vector<double> &pi, const vector<double> &local0,
                 vector<double> * delta0);
  // After the forward pass has finished we track back to get all the nodes
  // of the most probable series of states in the HMM
  void BackTrackBestPath(const vector<double> &delta_T_1,
                         const vector<vector<int> > &best,
                         vector<int> *best_path);
  // Divide all the numbers in the vector vec (scaled probabilities) by
  // the factor norm (to sum to 1).
  void Normalize(double norm, vector<double> *vec);
};

}  // namespace

#endif  // OPENHTMM_FAST_RESTRICTED_VITERBI_H__
