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


#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "EM.h"
#include "fast_restricted_hmm.h"
#include "document.h"
#include "sentence.h"
#include "error.h"
#include "fast_restricted_viterbi.h"


using namespace std;

namespace htmm {

// Initialization: The documents are read throught the HTMM init class,
// the parameters are set to random variables.
// If we want to repeat a specific run the argument seed is used to specify
// the random seed. Otherwise we specify seed=0 and the random generator is
// initialized according to the clock.
void EM::init(int topics, int words, double alpha, double beta, int iters,
              const char *fname, const char *data_dir, unsigned int seed,
	      const char *train_map_file) {
  HTMM::init(topics, words, alpha, beta, fname, data_dir, train_map_file);
  iters_ = iters;
  // allocate and initialize phi, theta, state probs:
  RandInitParams(seed);
  //  ReadPhi("/cs/vis/amitg/webLDA/data/ldaK10/d_D_100_K_10_eps_1.0.true_phi");
}

// Randomly initializes the parameters (phi, theta and epsilon) and allocates
// space for the state probabilities.
void EM::RandInitParams(unsigned int seed) {
  // If seed is 0 use the clock to initialize the random generator.
  if (seed == 0) {
    seed = time(0);
  }
  cout << "seed = " << seed <<endl;
  srand48(seed);
  epsilon_ = drand48();
  theta_.resize(docs_.size());
  for (int i = 0; i < docs_.size(); i++) {
    theta_[i].resize(topics_);
    RandProb(&(theta_[i]));
  }
  phi_.resize(topics_);
  for (int i = 0; i < topics_; i++) {
    phi_[i].resize(words_);
    RandProb(&(phi_[i]));
  }
  p_dwzpsi_.resize(docs_.size());
  for (int d = 0; d < docs_.size(); d++) {
    p_dwzpsi_[d].resize(docs_[d]->size());
    for (int i = 0; i < docs_[d]->size(); i++) {
      p_dwzpsi_[d][i].resize(2*topics_);
      // This one is not initialized because we begin with the E-step
    }
  }
}

// Draws a random probability vector (uniformly distriubted).
void EM::RandProb(vector<double> *vec) {
  double norm = 0;
  for (int i = 0; i < vec->size(); i++) {
    (*vec)[i] = drand48();
    norm += (*vec)[i];
  }
  Normalize(norm, vec);
}

// EM inference
void EM::Infer() {
  double old_loglik = 0;
  for (int i = 0; i < iters_; i++) {
    EStep();
    MStep();
    cout << "iteration " << i << ", loglikelihood = " << loglik_ << endl;
    if ((i > 0) && (loglik_ - old_loglik < -1e-7)) {
      cerr << setprecision(20);
      cerr << "old likelihood = " << old_loglik << endl
	   << "likelihood = " << loglik_ << endl;
      ERROR("likelihood is not monotone\n");
    }
    old_loglik = loglik_;
  }
}

// Prepares things for the Forward Backward and use it.
void EM::EStep() {
  //    ReadLatentVars("/cs/vis/amitg/webLDA/data/ZL3/params_D_8283_K_5_eps_1.0.lat");
  loglik_ = 0;
  for (int d = 0; d < docs_.size(); d++) {
    double ll = 0;
    EStepSingleDoc(d, &ll);
    loglik_ += ll;
  }
  // Here we take into account the priors of the parameters when computing
  // the likelihood:
  IncorporatePriorsIntoLikelihood();
}

// Adds the Dirichlet priors to the likelihood computation.
// *loglik contains the observations likelihood computed using the forward
// backward algorithm. We now consider also the Dirichlet priors of the
// parameters.
void EM::IncorporatePriorsIntoLikelihood() {
  IncorporatePriorOnTheta();
  IncorporatePriorOnPhi();
}

// Incorporates the prior on theta, assuming a symmetric Dirichlet distirubiton
void EM::IncorporatePriorOnTheta() {
  for (int d = 0; d < docs_.size(); d++) {
    for (int z = 0; z < topics_; z++) {
      loglik_ += (alpha_-1)*log(theta_[d][z]);
    }
  } 
}

// Incorporates the prior on phi, assuming a symmetric Dirichlet distirubiton
void EM::IncorporatePriorOnPhi() {
  for (int z = 0; z < topics_; z++) {
    for (int w = 0; w < words_; w++) {
      loglik_ += (beta_-1)*log(phi_[z][w]);
    }
  }
}


// Given the parameters, find the most probable sequence of hidden states
void EM::MAPTopicEstimate(int d, vector<int> *best_path) {
  FastRestrictedViterbi f;
  vector<double> dummy(topics_);
  vector<vector<double> > local(docs_[d]->size(), dummy);
  double dummy_ll;        // we don't care about the likelihood in this case.
  ComputeLocalProbsForDoc(d, &local, &dummy_ll);
  vector<double> init_probs(topics_*2);
  for (int i = 0; i < topics_; i++) {
    init_probs[i] = theta_[d][i];
    init_probs[i+topics_] = 0;  // document must begin with a topic transition
  }
  f.Viterbi(epsilon_, theta_[d], local, init_probs, best_path);
}

// Performs the Estep for a single document, namely computes the posterior
// state probabilities using the efficient forward backward algorithm (the
// parameters are given from the Mstep).
void EM::EStepSingleDoc(int d, double *ll) {
  vector<double> dummy(topics_);
  vector<vector<double> > local(docs_[d]->size(), dummy);
  double local_ll = 0;
  ComputeLocalProbsForDoc(d, &local, &local_ll);
  vector<double> init_probs(topics_*2);
  for (int i = 0; i < topics_; i++) {
    init_probs[i] = theta_[d][i];
    init_probs[i+topics_] = 0;  // Document must begin with a topic transition.
  }
  FastRestrictedHMM f;
  f.ForwardBackward(epsilon_, theta_[d], local, init_probs,
                    &(p_dwzpsi_[d]), ll);
  *ll = *ll + local_ll;
}

// Computes the local probabilites for all the sentences of a particular
// document.
void EM::ComputeLocalProbsForDoc(int d, vector<vector<double> > *locals,
                                 double *ll) {
  for (int i = 0; i < docs_[d]->size(); i++) {
    ComputeLocalProbsForItem(docs_[d]->GetSentence(i), &((*locals)[i]), ll);
  }
}

// This method is used to compute local probabilities for a word or for a
// sentece.
// Actually, we compute potentials rather than emission probabilities
// because we have to normalize them
void EM::ComputeLocalProbsForItem(const Sentence &sen,
                                  vector<double>  *local,
                                  double *ll) {
  for (int z = 0; z < topics_; z++) {
    (*local)[z] = 1;
  }
  Normalize(topics_, local);
  *ll += log(topics_);
  for (int i = 0; i < sen.size(); i++) {
    double norm = 0;
    int word = sen.GetWord(i);
    for (int z = 0; z < topics_; z++) {
      (*local)[z] *= phi_[z][word];
      norm += (*local)[z];
    }
    Normalize(norm, local);  // to prevent underflow
    *ll += log(norm);
  }
}

// The M-step of the EM algorithm for HTMM.
void EM::MStep() {
  //FindEpsilon();
  epsilon_ = 1;
  FindPhi();
  FindTheta();
}

// Read Phi from a file - only for debug
void EM::ReadPhi(const char *fname) {
  ifstream in(fname);
  if (!in || in.fail()) {
    ERROR("can't open file "<< fname << "for reading");
  }
  int t, w;
  in >> t >> w;
  for (int i = 0; i < t; i++) {
    for (int j = 0; j < w; j++) {
      in >> phi_[i][j];
    }
  }
  in.close();
}
// Finds the theta for all documents in the train set.
void EM::FindTheta() {
  for (int d = 0; d < docs_.size(); d++) {
    FindSingleTheta(d);
  }
}

// Finds the MAP estimator for theta_d
void EM::FindSingleTheta(int d) {
  double norm = 0;
  vector<double> Cdz(topics_);
  CountTopicsInDoc(d, &Cdz);
  for (int z = 0; z < topics_; z++) {
    theta_[d][z] = Cdz[z] + alpha_ - 1;
    norm += theta_[d][z];
  }
  Normalize(norm, &(theta_[d]));
}

// We count only the number of times when a new topic was drawn according to
// theta, i.e. when psi=1 (this includes the beginning of a document).
void EM::CountTopicsInDoc(int d, vector<double> *Cdz) {
  for (int z = 0; z < topics_; z++) {
    (*Cdz)[z] = 0;
  }
  for (int i = 0; i < docs_[d]->size(); i++) {
    for (int z = 0; z < topics_; z++) {
      // only psi=1
      (*Cdz)[z] += p_dwzpsi_[d][i][z];
    }
  }
}

// This is code duplication!
// There's already a normalize method in the FastHMM class
void EM::Normalize(double norm, vector<double> *v) {
  double invnorm = 1.0 / norm;
  double* vp = &(*v)[0];
  for (int i = 0; i < v->size(); i++) {
    *vp++ *= invnorm;
  }
}

// Finds the MAP estimator for epsilon.
void EM::FindEpsilon() {
  int total = 0;
  double lot = 0;
  for (int d = 0; d < docs_.size(); d++) {
    //  we start counting from the second item in the document
    for (int i = 1; i < docs_[d]->size(); i++) {
      for (int z = 0; z < topics_; z++) {
        // only psi=1
        lot += p_dwzpsi_[d][i][z];
      }
    }
    total += docs_[d]->size()-1;      // Psi is always 1 for the first
                                      // word/sentence
  }
  epsilon_ = lot/total;
}

// Czw is already allocated and all its entries are initialized to 0.
void EM::CountTopicWord(vector<vector<double> > *Czw) {
  // iterate over all sentences in corpus
  for (int d = 0; d < docs_.size(); d++) {
    for (int i = 0; i < docs_[d]->size(); i++) {
      CountTopicWordInSentence(docs_[d]->GetSentence(i), p_dwzpsi_[d][i], Czw);
    }
  }
}

// Counts how many times the pair (z,w) for a certain topic z and a certain
// word w appears in a certain sentence,
void EM::CountTopicWordInSentence(const Sentence &sen,
                                  const vector<double> &topic_probs,
                                  vector<vector<double> > *Czw) {
  // Iterate over all the words in a sentence
  for (int n = 0; n < sen.size(); n++) {
    int w = sen.GetWord(n);
    for (int z = 0; z < topics_; z++) {
      // both psi=1 and psi=0
      (*Czw)[z][w] += topic_probs[z]+topic_probs[z+topics_];
    }
  }
}

// Finds the MAP estimator for phi
void EM::FindPhi() {
  vector<double> dummy(words_, 0);
  vector<vector<double> > Czw(topics_, dummy);
  CountTopicWord(&Czw);   // Czw is allocated and initialized to 0
  for (int z = 0; z < topics_; z++) {
    double norm = 0;
    for (int w = 0; w < words_; w++) {
      phi_[z][w] = Czw[z][w] + beta_ - 1;
      norm += phi_[z][w];
    }
    Normalize(norm, &(phi_[z]));
  }
}

// Saves theta in a file with extension .theta.
void EM::SaveTheta(const char *fname) {
  std::string str(std::string(fname)+std::string(".theta"));
  ofstream out(str.c_str());
  if (!out || out.fail()) {
    ERROR("can't open file " << str << " for writing");
  }
  out << docs_.size() << "\t" << topics_ << endl;
  for (int d = 0; d < docs_.size(); d++) {
    for (int z = 0; z < topics_; z++) {
      out << theta_[d][z] << "\t";
    }
    out << endl;
  }
  out.close();
}

// Saves phi in a file with extension .phi.
void EM::SavePhi(const char *fname) {
  std::string str(std::string(fname)+std::string(".phi"));
  ofstream out(str.c_str());
  if (!out || out.fail()) {
    ERROR("can't open file " << str << " for writing");
  }
  out << topics_ << "\t" << words_ << endl;
  for (int i = 0; i < topics_; i++) {
    for (int j = 0; j <words_; j++) {
      out << phi_[i][j] << "\t";
    }
    out << endl;
  }
  out.close();
}

// Saves epsilon in a file with extension .eps.
void EM::SaveEpsilon(const char *fname) {
  std::string str(std::string(fname)+std::string(".eps"));
  ofstream out(str.c_str());
  if (!out || out.fail()) {
    ERROR("can't open file " << str << " for writing");
  }
  out << epsilon_ << endl;
  out.close();
}

// Saves the log likelihood in a file with extension .ll.
void EM::SaveLogLikelihood(const char *fname) {
  std::string str(std::string(fname)+std::string(".ll"));
  ofstream out(str.c_str());
  if (!out || out.fail()) {
    ERROR("can't open file " << str << " for writing");
  }
  out << loglik_ << endl;
  out.close();
}

// Saves all parameters and the distribution on hidden states.
void EM::SaveAll(const char *base_name) {
  SaveTheta(base_name);
  SavePhi(base_name);
  SaveEpsilon(base_name);
  SaveTopicTransProbs(base_name);
  SaveLogLikelihood(base_name);
}

// Save the latent states probabilities.
void EM::SaveTopicTransProbs(const char *fname) {
  std::string str(std::string(fname)+std::string(".pdwz"));
  ofstream out(str.c_str());
  if (!out || out.fail()) {
    ERROR("can't open file " << str << " for writing");
  }
  out << docs_.size() << "\t" << topics_ << endl;
  for (int d = 0; d < docs_.size(); d++) {
    //    out << "d = " << d << endl;
    out << docs_[d]->size() << endl;
    for (int i = 0; i < docs_[d]->size(); i++) {
      //      out << "i = " << i << endl;
      for (int z = 0; z < topics_*2; z++) {
	/*        if (z >= topics_) {
          out << "*";
	  }*/
        out << p_dwzpsi_[d][i][z] << "\t";
      }
      out << endl;
    }
    out << endl;
  }
  out.close();
}

void EM::ReadLatentVars(const char *fname) {
  cerr << "in read latent vars\n";
  ifstream in(fname);
  if (!in || in.fail()) {
    ERROR("can't open file " << fname << " for reading");
  }
  int D, L, psi, t;
  in >> D;
  if (D != docs_.size()) {
    ERROR("number of documents mismatch");
  }
  for (int d = 0; d < D; d++) {
    in >> L;
    if (L < docs_[d]->size()) {
      ERROR("length of document " << d << " doesn't match, L = " << L
	    << ", size = " << docs_[d]->size());
    }
    // if it's a test doc we see only the beginning of it.
    for (int i = 0; i < docs_[d]->size(); i++) {
      in >> psi >> t;
      for (int z = 0; z < 2*topics_; z++) {
	p_dwzpsi_[d][i][z] = 0;
      }
      p_dwzpsi_[d][i][topics_*(1-psi) + t - 1] = 1;
    }
    for (int i = 0; i < L - docs_[d]->size(); i++) {
      in >> psi >> t;
    }
  }
}

}  // namespace
