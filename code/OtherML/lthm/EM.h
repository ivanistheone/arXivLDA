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


#ifndef OPENHTMM_EM_H__
#define OPENHTMM_EM_H__

#include <vector>
#include "htmm.h"


using namespace std;

namespace htmm {

class Sentence;

class EM : public HTMM {
 public:
  EM() {}

  virtual ~EM() { }
  // Initialization: The documents are read, hyper parameters and number of
  // iterations are set and the parameters are set to random variables.
  // If we want to repeat a specific run, the argument seed is used to specify
  // the random seed. Otherwise we specify seed=0 and the random generator is
  // initialized according to the clock.

  void init(int topics, int words, double alpha, double beta, int iters,
            const char *fname, const char *data_dir, unsigned int seed,
	    const char *train_map_file);

  // EM inference for HTMM
  virtual void Infer();

  // Saves all parameters and the distribution on hidden states.
  virtual void SaveAll(const char *base_name);

 protected:
  // Methods:
  // E-step of the algorithm
  virtual void EStep();

  // Computes expectations and contribution to the likelihood for a single
  // document.
  virtual void EStepSingleDoc(int d, double *ll);

  // M-step of the algorithm
  virtual void MStep();

  // Finds the MAP estimator for all thetas.
  void FindTheta();

  // Finds the MAP estimator for theta_d.
  virtual void FindSingleTheta(int d);

  // Finds the MAP estimator for epsilon.
  void FindEpsilon();

  // Finds the MAP estimator for all phi.
  void FindPhi();

  // Counts (the expectation of) how many times a topic was drawn from
  // theta in a certain document.
  void CountTopicsInDoc(int d, vector<double> *Cdz);

  // Counts (the expectation of) how many times the each pair (topic, word)
  // appears in a certain sentence.
  void CountTopicWordInSentence(const Sentence &sen,
                                const vector<double> &topic_probs,
                                vector<vector<double> > *Czw);

  // Counts (the expectation of) how many times the each pair (topic, word)
  // appears in the whole corpus.
  void CountTopicWord(vector<vector<double> > *Czw);

  // Compute the emission (local) probabilities for a certain document.
  void ComputeLocalProbsForDoc(int d, vector<vector<double> > *probs,
                               double *ll);

  // Compute the emission (local) probabilities for a certain sentence.
  void ComputeLocalProbsForItem(const Sentence &sen,
                                vector<double>  *local, double *ll);

  // Saves theta in a file with extension .theta
  virtual void SaveTheta(const char *fname);

  // Saves phi in a file with extension .phi.
  virtual void SavePhi(const char *fname);

  // Saves epsilon in a file with extension .eps.
  virtual void SaveEpsilon(const char *fname);

  // Saves the latent states probabilities.
  virtual void SaveTopicTransProbs(const char *fname);

  // Saves the log likelihood.
  virtual void SaveLogLikelihood(const char *fname);

  // Randomly initializes the parameters (phi, theta and epsilon) and allocates
  // space for the state probabilities.
  // See comments above (for init) regarding the random seed.
  void RandInitParams(unsigned int seed);

  // Draws a random probability vector (uniformly distriubted).
  void RandProb(vector<double> *vec);

  // Given the parameters, finds the most probable sequence of hidden states.
  void MAPTopicEstimate(int d, vector<int> *best_path);

  // Adds the Dirichlet priors to the likelihood computation.
  virtual void IncorporatePriorsIntoLikelihood();

  // I'm not sure if that's the place for this method!
  void Normalize(double norm, vector<double> *v);

  //Incorporates the prior on theta, assuming a symmetric Dirichlet distirubiton
  void IncorporatePriorOnTheta();

  // Incorporates the prior on phi, assuming a symmetric Dirichlet distirubiton
  void IncorporatePriorOnPhi();

  // For debug only
  void ReadPhi(const char *fname);
  void ReadLatentVars(const char *fname);

  // Data members:
  int iters_;                           // number of EM iterations
  vector<vector<double> > theta_;       // estimated theta
  vector<vector<double> > phi_;         // estimated phi
  double epsilon_;                      // estimated epsilon
  vector<vector<vector<double> > > p_dwzpsi_;  // The state probabilities,
           // that is Pr(z,psi | d,w). The weird name reflects the arrangements
           // of these probabilities in this 3D array.
  double loglik_;                       // loglikelihood

 private:
  // Copy Constructor - never use it for this class.
  EM(const EM &);
};

}  // namespace

#endif  // OPENHTMM_EM_H__
