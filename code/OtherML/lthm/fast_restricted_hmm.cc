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

#include <math.h>
#include <vector>
#include "fast_restricted_hmm.h"
#include <assert.h>
#define ASSERT_EQ(a,b) assert(a==b)



using namespace std;

namespace htmm {

// This method computes posterior probabilities according to the
// Forward Backward algorithm for HMM.
// Notice that due to the special structure of our problem we have an effective
// number of states which is twice the number of topics.
// See http://www.cs.huji.ac.il/~amitg/aistats07.pdf
// We assume the reader is familiar with inference in HMMs and with the
// above mentioned paper.
// See the comments at the start of fast_restricted_hmm.h for definitions of
// variables and notations used throughout this file.

void FastRestrictedHMM::ForwardBackward(double epsilon,
                                        const vector<double> &theta,
                                        const vector<vector<double> > &local,
                                        const vector<double> &pi,
                                        vector<vector<double> > *sprobs,
                                        double *loglik) {
  topics_ = theta.size();
  states_ = 2*topics_;
  vector<double> norm_factor(local.size());
  vector<double> dummy(states_);
  ASSERT_EQ(pi.size(), states_);
  ASSERT_EQ(sprobs->size(), local.size());
  for (int i = 0; i < local.size(); i++) {
    ASSERT_EQ(((*sprobs)[i]).size(), states_);
    ASSERT_EQ(local[i].size(), topics_);
  }
  vector<vector<double> > alpha(local.size(), dummy);
  vector<vector<double> > beta(local.size(), dummy);
  InitAlpha(pi, local[0], &(norm_factor[0]), &(alpha[0]));
  ComputeAllAlphas(local, theta, epsilon, &norm_factor, &alpha);
  InitBeta(norm_factor[local.size()-1], &(beta[local.size()-1]));
  ComputeAllBetas(local, theta, epsilon, norm_factor, &beta);
  CombineAllProbs(alpha, beta, sprobs);
  ComputeLoglik(norm_factor, loglik);
}

// This method initializes alpha[0] to be Pr(y_0 | q_0; param) * Pr(q_0).
void FastRestrictedHMM::InitAlpha(const vector<double> &pi,
                                  const vector<double> &local0,
                                  double *norm, vector<double> *alpha0) {
  *norm = 0;
  for (int i = 0; i < topics_; i++) {
    (*alpha0)[i] = local0[i]*pi[i];
    (*alpha0)[i+topics_] = local0[i]*pi[i+topics_];
    *norm += (*alpha0)[i] + (*alpha0)[i+topics_];
  }

  Normalize(*norm, alpha0);
}

// This method initializes beta[T-1] to be all ones.
void FastRestrictedHMM::InitBeta(double norm, vector<double> *beta_T_1) {
  for (int i = 0; i < states_; i++) {
    (*beta_T_1)[i] = 1;
  }

  Normalize(norm, beta_T_1);
}

// This method computes alpha[i] for i>=1 (after alpha[0] has been initialized).
// Notice that this is not the standard alpha computation, we are taking
// advantage of the special structure of our problem!
void FastRestrictedHMM::ComputeAllAlphas(const vector<vector<double> > &local,
                                         const vector<double> &theta,
                                         double epsilon,
                                         vector<double> *norm_factor,
                                         vector<vector<double> > *alpha) {
  for (int i = 1; i < local.size(); i++) {
    ComputeSingleAlpha(local[i], theta, epsilon, &((*norm_factor)[i]),
                       (*alpha)[i-1], &((*alpha)[i]));
  }
}

// This method computes the alphas for a single level after alpha has
// been computed for the previous level.  (See the comments at the
// start of fast_restricted_hmm.h for a description of alpha and
// beta.)
// alpha[t][s] corresponds to a new lottery when picking the topic s at time t.
// alpha[t][s+topics] corresponds to setting the topic at time t by copying the
// topic s from time t-1.
void FastRestrictedHMM::ComputeSingleAlpha(const vector<double> &local_t,
                                           const vector<double> &theta,
                                           double epsilon, double *norm,
                                           const vector<double> &alpha_t_1,
                                           vector<double> *alpha_t) {
  *norm = 0;
  for (int s = 0; s < topics_; s++) {
    (*alpha_t)[s] = epsilon*theta[s]*local_t[s];  // regardless of the previous
       // topic - remember that sum_k alpha[t-1][k] is 1 (because of the norm).
    (*alpha_t)[s+topics_] = (1-epsilon)*(alpha_t_1[s] +
                                         alpha_t_1[s+topics_])*local_t[s];
    *norm += (*alpha_t)[s]+(*alpha_t)[s+topics_];
  }

  Normalize(*norm, alpha_t);
}

// Computes beta[i] for i<=T-2 (beta[T-1] has been initialized). Use same
// normalizing factors as in the alpha computation.
// Notice that this is not the standard beta computation, we are taking
// advantage of the special structure of our problem!
void FastRestrictedHMM::ComputeAllBetas(const vector<vector<double> > &local,
                                        const vector<double> &theta,
                                        double epsilon,
                                        const vector<double> &norm_factor,
                                        vector<vector<double> > *beta) {
  for (int i = local.size()-2; i >= 0; i--) {
    ComputeSingleBeta(local[i+1], theta, epsilon, norm_factor[i],
                      (*beta)[i+1], &((*beta)[i]));
  }
}

// This method computes the betas for a single level after beta has been
// computed for the next level.
void FastRestrictedHMM::ComputeSingleBeta(const vector <double> &local_t_1,
                                          const vector<double> &theta,
                                          double epsilon, double norm,
                                          const vector<double> &beta_t1,
                                          vector<double> *beta_t) {
  double trans_sum = 0;
  for (int i = 0; i < topics_; i++) {
    trans_sum += epsilon*theta[i]*local_t_1[i]*beta_t1[i];
  }

  for (int s = 0; s < topics_; s++) {
    // Recall that beta_t1[s] == beta_t1[s+topics_]
    (*beta_t)[s] = trans_sum + (1-epsilon)*local_t_1[s]*beta_t1[s];
    (*beta_t)[s] /= norm;
    (*beta_t)[s+topics_] = (*beta_t)[s];
  }
  // we've already normalized the betas!
}

// This method combines the previously computed alpha and beta to get the
// posterior probabilities.
void FastRestrictedHMM::CombineAllProbs(const vector<vector<double> > &alpha,
                                        const vector<vector<double> > &beta,
                                        vector<vector<double> > *sprobs) {
  for (int i = 0; i < alpha.size(); i++) {
    CombineSingleProb(alpha[i], beta[i], &((*sprobs)[i]));
  }
}

// This method combines the alpha and beta to get probabilities for a
// single level.
void FastRestrictedHMM::CombineSingleProb(const vector<double> &alpha,
                                          const vector<double> &beta,
                                          vector<double> *sprobs) {
  double norm = 0;
  for (int s = 0; s < states_; s++) {
    (*sprobs)[s] = alpha[s]*beta[s];
    norm += (*sprobs)[s];
  }
  Normalize(norm, sprobs);
}

// This method normalizes the vector vec to according to the factor norm.
void FastRestrictedHMM::Normalize(double norm, vector<double> *vec) {
  double *vp = &((*vec)[0]);
  for (int i = 0; i < vec->size(); i++) {
    *vp++ /= norm;
  }
}

// This method computes the log likelihood for the given observations sequence
// while averaging over all possible states.
// Alpha and beta must have been computed before calling this method.
// Summming the posterior state probs is useless, as they are normalized to 1.
// The alphas are normalized, hence we need to consider the normalizing
// factors.
void FastRestrictedHMM::ComputeLoglik(const vector<double> &norm_factor,
                                      double *loglik) {
  *loglik = 0;
  for (int t = 0; t < norm_factor.size(); t++) {
    *loglik += log(norm_factor[t]);
  }
}

FastRestrictedHMM::~FastRestrictedHMM() {
  // No pointers in my class!
}

}  // namespace
