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

#ifndef OPENHTMM_FAST_RESTRICTED_HMM_H__
#define OPENHTMM_FAST_RESTRICTED_HMM_H__

#include <vector>

// The FastRestrictedHMM class is an implementation of an efficient algorithm
// for inference (computing marginal probabilities and likelihood) in a special
// case of HMM.
// It is designed for HMMs with matrix transition of a special form as
// described in the paper "Hidden Topic Markov Models" available at:
// http://www.cs.huji.ac.il/~amitg/aistats07.pdf
// Throughout this code we use the standard alpha and beta recursions:
// alpha_t(i) = Pr(y_1,...,y_t,q_t=i ; param).
// beta_t(i)  = Pr(y_{t+1},...,y_T | q_t=i; param).
// T is the length of the markov chain,
// y_t are the observations at time i, q_t is the (discrete) state at time t,
// param is the set of parmeters of the markov model.
// We assume the reader is familiar with inference in HMMs and with the
// above mentioned paper.


// Assumptions:
// (1) The transition matrix is of the special form described in the HTMM paper,
// i.e. consists of entries that depend on theta and epsilon only
// (2) sprobs is a vector of T vectors, each of which has states=2*topics
// entries and is already allocated when Forward Backward is called.
// (3) Theta is a probability vector of length topics.
// (4) Local is a vector of T probability vectors, each of which is of length
// topics and contains the emissions probabilities.
// (5) pi is a probability vector of length states=2*topics and specifies the
// prior distribution on the first hidden state in the chain.

// Design Issues:
// We might want to have a base HMM class and derive FastRestrictedHMM from it.
// pros:
// we can easily switch between different implementations of HMM.
// cons:
// this is not a general HMM implementation, it is not suitable whereever HMM is
// suitable.
// the interface of the initialization is different from standard HMM.


using namespace std;

namespace htmm {

class FastRestrictedHMM {
 public:
  FastRestrictedHMM() {}

  ~FastRestrictedHMM();

  // This method computes effciently the marginal state probabilities and
  // likelihood in the HMM specified by the method's arguments. See the comment
  // in the beginning of this file for more details regarding this computations
  // and the assumptions being made.
  // This function is not thread safe (should not be called on the same object
  // more than once simultaneously).
  // Parameters:
  //   epsilon is a probability defined in the HTMM paper referenced above;
  //   sprobs, theta, local, pi are as defined in assumptions (2)-(5) above;
  //   *loglik will contain the log likelihood computed by ForwardBackward.
  // sprobs is assumed to be already allocated to the appropriate dimensions
  // as described in assumption (2) above when ForwardBackward is called.
  void ForwardBackward(double epsilon, const vector<double> &theta,
                       const vector<vector<double> > &local,
                       const vector<double> &pi,
                       vector<vector<double> > *sprobs_, double *loglik);

 private:

  // This method normalizes the vector vec to according to the factor norm.
  void Normalize(double factor, vector<double> *vec);

  // This method initializes alpha[0] to be Pr(y_0 | q_0; param) * Pr(q_0)
  // norm is the normalizing factor used.
  // The other parameters are as defined above.
  void InitAlpha(const vector<double> &pi, const vector<double> &local0,
                 double *norm, vector<double> *alpha0);

  // This method initializes beta[T-1] (the argument beta_T_1) to be all ones
  // and then normalizes it according to the normalization of alpha[T-1].
  // norm is the normalizing factor used.
  // The other parameters are as defined above.
  void InitBeta(double norm, vector<double> *beta_T_1);

  // This method computes alpha[i] for i>=1 (after alpha[0] has been
  // initialized).
  // norm_factor is the vector of normalizing factors used along the chain.
  // The other parameters are defined above.
  void ComputeAllAlphas(const vector<vector<double> > &local,
                        const vector<double> &theta, double epsilon,
                        vector<double> *norm_factor,
                        vector<vector<double> > *alpha);

  // This method computes the alphas for a single level after alpha has been
  // computed for the previous level.
  // alpha_t_1 is alpha[t-1], norm is the normalizing factor.
  // The other parameters are as defined above.
  void ComputeSingleAlpha(const vector<double> &local_t,
                          const vector<double> &theta, double epsilon,
                          double *norm, const vector<double> &alpha_t_1,
                          vector<double> *alpha_t);

  // Computes beta[i] for i<=T-2 (beta[T-1] has been initialized). Use same
  // normalizing factors as in the alpha computation.
  // norm_factor is the vector of normalizing factors used along the chain.
  // The other parameters are as defined above.
  void ComputeAllBetas(const vector<vector<double> > &local,
                       const vector<double> &theta,
                       double epsilon, const vector<double> &norm_factor,
                       vector<vector<double> > *beta);

  // This method computes the betas for a single level after beta has been
  // computed for the next level.
  // local_t_1 is local[t-1], beta_t1 is beta[t+1], norm is the normalizing
  // factor.
  // The other parameters are as defined above.
  void ComputeSingleBeta(const vector <double> &local_t_1,
                         const vector<double> &theta,
                         double epsilon, double norm,
                         const vector<double> &beta_t1,
                         vector<double> *beta_t);

  // This method combines the previously computed alpha and beta to get the
  // posterior state probabilities.
  // The parameters are as defined above.
  void CombineAllProbs(const vector<vector<double> > &alpha,
                       const vector<vector<double> > &beta,
                       vector<vector<double> > *sprobs);

  // This method combines the alpha and beta to get probabilities for a
  // single level.
  // The parameters are as defined above.
  void CombineSingleProb(const vector<double> &alpha,
                         const vector<double> &beta,
                         vector<double> *sprobs);

  // This method computes the log likelihood for the given observations sequence
  // while averaging over all possible states.
  // Alpha and beta must have been computed before calling this method.
  // norm_factor is the normalizing factor that was used when computing the
  // alphas and the betas.
  void ComputeLoglik(const vector<double> &norm_factor,
                     double *loglik);


  // Data members:

  int topics_;                          // The number of topics.
  int states_;                          // The number of possible states.
};

}  // namespace

#endif  // OPENHTMM_FAST_RESTRICTED_HMM_H__
