#include <math.h>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include "web_em.h"
#include "fast_restricted_hmm.h"
#include "error.h"
#include "check_prob.h"

#define REAL_DATA

const int k_link_weight = 1;

namespace htmm { 

void WebEM::init(int topics, int words, double alpha, double beta, int iters,
          const char *fname, const char *data_dir, unsigned int seed,
		 const char *links_file, const char *gamma_file,
		 const char *train_map_file) {
  cerr << "before EM init\n";
  EM::init(topics, words, alpha, beta, iters, fname, data_dir, seed,
	   train_map_file);
  cerr << "after em init, before count sentences\n";
  CountSentences();
  cerr<< " before read gamma\n";
  InitGamma(gamma_file);
  //  cerr << "before init lambda\n";
  RandInitLambda();
  AllocateLinksVars();
  cerr<< "before read links\n";
  ReadTrainLinks(links_file);
  //  cerr << "before read zl\n";
  //  ReadZL("/cs/vis/amitg/webLDA/data/ZL/params_D_8283_K_5_eps_1.0.zl");
  //  cerr << "after read zl\n";
  //  ReadTau("/cs/vis/amitg/webLDA/data/ZL/params_D_8283_K_5_eps_1.0.tau");
  //  cerr << "after read tau\n";
  //  cerr << "end of init\n";
  // ONLY FOR DEBUG!!!
  //  cerr << "WARNING: reading lambda " << lambda_file << "\n";
  //  ReadLambda("/cs/vis/amitg/webLDA/data/new_short/lambda.init");
  //  cerr << "WARNING reading phi " << phi_file << "\n";
  //  ReadPhi(phi_file);
}

// Allocate memory for the new data members: semi_posteriors_,
// topic_in_corpus_, links_to_topic_counts_, lambda_.
// When this method is invoked all documents should have been read already.
void WebEM::AllocateLinksVars() {
  //  semi_posterior_.resize(docs_.size());
  links_to_topic_counts_.resize(docs_.size());
  cerr << "before for\n";
  for (int d = 0; d < docs_.size(); d++) {
    //    cerr << "docs["<<d<<"].size = " << docs_[d]->size() << endl;
    //    semi_posterior_[d].resize(docs_[d]->size());
    links_to_topic_counts_[d].resize(topics_); 
    /*    for (int n = 0; n < docs_[d]->size(); n++) {
      semi_posterior_[d][n].resize(2*topics_);
      }*/
  }
  cerr << "after for\n";
  semi_topic_in_corpus_.resize(topics_);
  cerr << "topics_ = " << topics_ << endl;
  lambda_.resize(docs_.size()+1);
  cerr << "docs_.size = " << docs_.size() << endl;
  pr_no_link_given_topic_.resize(topics_);
  cerr << "at the end\n";
  //  pr_no_link_.resize(docs_.size());
}

// Counts the total number of sentences in corpus.
void WebEM::CountSentences() {
  total_sentences_in_corpus_ = 0;
  for (int d = 0; d < docs_.size(); d++) {
    total_sentences_in_corpus_ += docs_[d]->size();
  }
  cerr << "docs_.size() = " << docs_.size() << ", sentences = " 
       << total_sentences_in_corpus_ << endl;
}

// Initialize Lambda randomly
void WebEM::RandInitLambda() {
  lambda_.resize(docs_.size()+1);
  // Maybe it is worth somehow to increase the last lambda 
  // (corresponding to no link) ?
  RandProb(&lambda_);
}

// Initialize Gamma, the Dirichlet prior on lambda (assymetric).
void WebEM::InitGamma(const char *gamma_file) {
  ifstream in(gamma_file);
  if (!in || in.fail()) {
    ERROR("can't open gamma file " << gamma_file << " for reading");
  }
  //  int N;
  //  in >> N;
  //  if (N != docs_.size()+1) {
  //    ERROR("size of gamma " << N <<" doesn't match number of documents " 
  //	  << docs_.size());
  //  }
  gamma_.resize(docs_.size()+1);
  double g;
  for (int d = 0; d < docs_.size(); d++) {
    in >> g;
    if (g < 1) {
      ERROR("Dirichlet prior is less than 1");
    }
    if (train_map_.find(d) != train_map_.end()) {
      gamma_[train_map_[d]] = g;
    }
  }
  in >> gamma_[docs_.size()];
  if (gamma_[docs_.size()] < 1) {
    ERROR("gamma(no_link) = " << gamma_[docs_.size()]);
  }
  in.close();
}

void WebEM::ReadTrainLinks(const char *fname) {
  //  ERROR("need to figure out what happens with train vs. test");

  //  cerr << "going to read links from file " << fname << endl;
  ifstream in(fname);
  if (!in || in.fail()) {
    ERROR("can't open file " << fname << " for reading");
  }
  links_from_.resize(docs_.size());
  links_to_.resize(docs_.size());
  int N, from, sent, to;
  //  in >> N;
  //  cerr << "docs_.size() = " << docs_.size() << ", N = " << N << endl;
  //  for (int i = 0; i < N; i++) {
  //    if (in.eof()) {
  //      ERROR("something's wrong with the links file.");
  //    }
  for (in >> from >> sent >> to; !in.eof(); in >> from >> sent >> to) {
#ifdef SYNTHETIC    
    to = to - 1;//    to = train_map[to];
    from = from - 1;//    from = train_map[from];
    sent = sent - 1;
#endif
    if ((from < 0) || (from >= docs_.size())) {
      ERROR("illegal link from doc = " << from);
    }
    if ((sent < 0)) { // || (sent >= docs_[from]->size())) {
      ERROR("illegal link from doc = " << from <<", sentence " << sent);
    }
    if (sent >= docs_[from]->size()) {
      cerr << "skipping link = " << from << " " << sent << " " << to << endl;
    }
    if ((to < 0) || (to >= docs_.size())) {
      ERROR("illegal link to doc = " << to);
    }
    // Keep only the links where both ends belong to the train set.
    //    if ((train_map_.find(from) != train_map_.end()) && 
    //	(train_map_.find(to) != train_map_.end())) {
    //      from = train_map_[from];
    //      to = train_map_[to];
      if (sent < docs_[from]->size()) {
	// keep only first occurence of link.
	if (links_from_[from].find(sent) == links_from_[from].end()) {
	  links_from_[from][sent] = to;
	  pair<int, int> p(from, sent);
	  links_to_[to].push_back(p);
	} else {
	  // do something in the future - some error message once this problem
	  // is fixed.
	}
      }
      //    }
  }
  in.close();

  int lf = 0, lt = 0;
  for (int d = 0; d < docs_.size(); d++) {
    lf += links_from_[d].size();
    lt += links_to_[d].size();
    //    cerr << "d = " << d << ", links from = " << links_from_[d].size()
    //	 << ", links to  = " << links_to_[d].size() << endl;
  }
  cerr << "total links from = " << lf << ", total links to = " << lt <<endl;
}

// Print all links originating from a certain document.
void WebEM::PrintLinksFrom(int doc) {
  map<int, int>::const_iterator iter;
  map<int, int> &m(links_from_[doc]);
  cerr << "doc = " << doc << ", size = " << links_from_[doc].size() << endl;
  for (iter = m.begin(); iter != m.end(); iter++) {
    pair<int,int> p = *iter;
    int sent = p.first;
    int to = p.second;
    cerr << "link from " << doc << " sent = " << sent << " to " << to << endl;
  }
}

// Print all links pointing to a certain document.
void WebEM::PrintLinksTo(int doc) {
  vector<pair<int, int> >::const_iterator iter;
  cerr << "doc = " << doc << ", size = " << links_to_[doc].size() << endl;
  for (iter = links_to_[doc].begin(); iter != links_to_[doc].end(); iter++) {
    int from = iter->first;
    int sent = iter->second;
    cerr << "link from " << from << " sent = " << sent << " to " << doc << endl;
  }
}

// Finds the theta for all documents in the train set.
void WebEM::FindTheta(const vector<vector<double> > &Udz,
		      const vector<vector<double> > &Vdz) {
  for (int d = 0; d < docs_.size(); d++) {
    FindSingleTheta(d, Udz[d], Vdz[d]);
    CheckProb::CheckProbVector(theta_[d], "find single theta");
  }
}

// Finds the MAP estimator for theta_d
  void WebEM::FindSingleTheta(int d, const vector<double> &Udz,
			      const vector<double> &Vdz) {
  double norm = 0;
  vector<double> Cdz(topics_);
  CountTopicsInDoc(d, &Cdz);
  //  cerr << "size(d) = " << docs_[d]->size() << ", epsilon = " << epsilon_ <<endl;
  //  cerr << "links to = " << links_to_[d].size() << endl;
  for (int z = 0; z < topics_; z++) {
    //        cerr << "d = " << d << ", z = " << z <<" Cdz = " << Cdz[z] 
	  //    	 << " Vdz = " << Vdz[z] << ", Udz = " << Udz[z] <<endl;
    theta_[d][z] = Cdz[z] + Vdz[z] + Udz[z] + alpha_ - 1;
    norm += theta_[d][z];
  }
  if (norm == 0) {
    ERROR("d = " << d << ", norm = 0");
  }
  Normalize(norm, &(theta_[d]));
}

// See definition of Vdz in h file.
// We need to iterate over all existing incoming links and sum the probabilities
// for a given topic, that is the posterior of z_i in the Markov chain
// (we considered the link existence in the inference and Z_L must be equal to
// z_i if there's a link).
void WebEM::ComputeVdz(vector<vector<double> > *Vdz) {
  for (int d = 0; d < docs_.size(); d++) {
    vector<pair<int, int> >::const_iterator iter;
    for (iter = links_to_[d].begin(); iter != links_to_[d].end(); iter++) {
      int from_doc = iter->first;
      int from_sent = iter->second;
      for (int z = 0; z < topics_; z++) {
	(*Vdz)[d][z] += (p_dwzpsi_[from_doc][from_sent][z] + 
			 p_dwzpsi_[from_doc][from_sent][z+topics_])*k_link_weight;
      }
    }
  }
}

// The following method is for debug - see that two different ways to compute
// Vdz give the same result.
// The second way to compute Vdz would be to use the semi-posterior
// probabilities (the ones that did not consider the link info) and multiply
// them by theta_d.
/*void WebEM::ComputeVdz2(vector<vector<double> > *Vdz) {
  for (int d = 0; d < docs_.size(); d++) {
    map<int, int>::const_iterator iter;
    for (iter = links_to_[d].begin(); iter != links_to_[d].end(); iter++) {
      int from_doc = iter->first;
      int from_sent = iter->second;
      for (int z = 0; z < topics_; z++) {
	(*Vdz)[d][z] += theta_[d][z]*(semi_posterior_[from_doc][from_sent][z] + 
			 semi_posterior_[from_doc][from_sent][z+topics_]);
      }
    }
  }
  }*/

// Computes n! 
int WebEM::Factorial(int n) {
  if (n < 0) {
    ERROR("factorial of " << n);
  }
  int res = 1;
  for (int i = 1; i <= n; i++) {
    res *= i;
  }
  return res;
}

// Returns n choose k
int WebEM::NChooseK(int n, int k) {
  return Factorial(n)/(Factorial(k)*Factorial(n-k));
}

// See definition of Udz in the h file.
// This computation is a bit tricky:
// need to take total topic count in the corpus, then subtract those for which
// links exist, then average it over tau (i.e. multiply by the relevant lambda).
// Must be called before any theta or lambda is modified.
void WebEM::NewComputeUdz(vector<vector<double> > *Udz) {
  //  CountSemiTopicInCorpus();  
  //  double norm_factor = ComputeUdzNormFactor();
  //  cerr << "norm_factor = " << norm_factor << endl;
  //  if ((norm_factor =< 0) || (norm_factor >= 1)) {
  //    ERROR("wrong Pr(no-link|O',P)");
  //  }
  vector<double> no_links_term(topics_, 0);
  vector<double> second_term(topics_, 0);
  ComputeNoLinksTerm(&no_links_term, &second_term);
  for (int d = 0; d < docs_.size(); d++) {
    for (int z = 0; z < topics_; z++) {
      // notice that lambda and theta here are from the previous iteration.
      if (k_link_weight == 1) {
	(*Udz)[d][z] = lambda_[d]*theta_[d][z]*no_links_term[z];
      } else {
	(*Udz)[d][z] = 0;
	for (int i = 1; i < k_link_weight; i++) {
	  (*Udz)[d][z] += i*lambda_[d]*NChooseK(k_link_weight, i)*
	    pow(theta_[d][z], i) * pow(1-theta_[d][z], k_link_weight-i)*
	    (no_links_term[z] + second_term[z]);
	}
	(*Udz)[d][z] += k_link_weight*lambda_[d]*
	  pow(theta_[d][z], k_link_weight) * no_links_term[z];
      }
    }
  }
}

// Computes the term that sums over all the sentences from which there are no
// outgoing links in the corpus
double WebEM::ComputeNoLinksTerm(vector<double> *no_links_term,
				 vector<double> *second_term) {
  vector<vector<double> > pr_no_link(docs_.size());
  for (int d = 0; d < docs_.size(); d++) {
    pr_no_link[d].resize(docs_[d]->size());
  }
  ComputeNoLinkProb(&pr_no_link);
  for (int z = 0; z < topics_; z++) {
    (*no_links_term)[z] = 0;
    (*second_term)[z] = 0;
    for (int d = 0; d < docs_.size(); d++) {
      for (int i = 0; i < docs_[d]->size(); i++) {
	vector<double> semi(2*topics_);
	RecomputeSemiPosterior(d, i, &semi);
	// sum only over no outgoing links.
	if (links_from_[d].find(i) == links_from_[d].end()) {
	  double pr = 
	    //	    semi_posterior_[d][i][z] + semi_posterior_[d][i][z+topics_];
	    semi[z] + semi[z+topics_];
	  (*no_links_term)[z] += (1 - pr) / pr_no_link[d][i];
	  (*second_term)[z] += pr / pr_no_link[d][i];
	}
      }
    }
  }
}

// Computes the probability for no link when the topic is not given.
// (conditioned on observed words, ignoring the links information, of course).
void WebEM::ComputeNoLinkProb(vector<vector<double> > *pr_no_link) {
  vector<double> incoming_term(topics_, 0);
  for (int z = 0; z < topics_; z++) {
    for (int to = 0; to < docs_.size(); to++) {
      incoming_term[z] += lambda_[to]*pow(theta_[to][z], k_link_weight);
    }
  }
  for (int from = 0; from < docs_.size(); from++) {
    for (int i = 0; i < docs_[from]->size(); i++) {
      (*pr_no_link)[from][i] = 1;  // need to initialize every M-step iteration
      vector<double> semi(2*topics_);
      RecomputeSemiPosterior(from, i, &semi);
      for (int z = 0; z < topics_; z++) {
	(*pr_no_link)[from][i] -= incoming_term[z] *
	  //	  (semi_posterior_[from][i][z] + semi_posterior_[from][i][z+topics_]);
	  (semi[z] + semi[z+topics_]);
      }
    }
  }
}

// Compute the normalization factor in Udz.
double WebEM::ComputeUdzNormFactor() {
  double res = 0;
  for (int z = 0; z < topics_; z++) {
    double lambda_theta_term = 0;
    for (int d = 0; d < docs_.size(); d++) {
      lambda_theta_term += lambda_[d]*pow(theta_[d][z], k_link_weight);
    }
    res += lambda_theta_term * semi_topic_in_corpus_[z];
  }
  return res;
}

// See definition of Udz in the h file.
// This computation is a bit tricky:
// need to take total topic count in the corpus, then subtract those for which
// links exist, then average it over tau (i.e. multiply by the relevant lambda).
void WebEM::OldComputeUdz(vector<vector<double> > *Udz) {
  CountSemiTopicInCorpus();  
  for (int d = 0; d < docs_.size(); d++) {
    CountSemiTopicLinksInDocument(d);
    for (int z = 0; z < topics_; z++) {
      double no_link_count = semi_topic_in_corpus_[z]*lambda_[d] - 
	                     links_to_topic_counts_[d][z];
      (*Udz)[d][z] = no_link_count;//*lambda_[d];
    }
  }
}

// Counts the expected number of occurances of each topic in the corpus based
// on the semi-posterior probabilities. It considers only topics associated 
// with a sentence not with a link.
void WebEM::CountSemiTopicInCorpus() {
  for (int z = 0; z < topics_; z++) {
    semi_topic_in_corpus_[z] = 0;
  }
  for (int d = 0; d < docs_.size(); d++) {
    for (int n = 0; n < docs_[d]->size(); n++) {
      vector<double> semi(2*topics_);
      RecomputeSemiPosterior(d, n, &semi);
      for (int z = 0; z < topics_; z++) {
	semi_topic_in_corpus_[z] += semi[z] + semi[z+topics_];
//semi_posterior_[d][n][z] + 
   	                            //semi_posterior_[d][n][z+topics_];
      }
    }
  }
}

// Counts the number of occurances of each topic associated with a sentence
// (not links) based on semi-posterior probabilities.
void WebEM::CountSemiTopicLinksInDocument(int d) {
  for (int z = 0; z < topics_; z++) {
    links_to_topic_counts_[d][z] = 0;
  }
  // Iterate over all the links pointing to document d:
  // TODO - check this one - if that's link to or link from
  //  map<int, int> &m(links_to_[d]);
  vector<pair<int, int> >::const_iterator iter;
  for (iter = links_to_[d].begin(); iter != links_to_[d].end(); iter++) {
    int from = iter->first;
    int sent = iter->second;
    vector<double> semi(2*topics_);
    RecomputeSemiPosterior(from, sent, &semi);
    for (int z = 0; z < topics_; z++) {
      links_to_topic_counts_[d][z] += semi[z] + semi[z+topics_]; 
      //semi_posterior_[from][sent][z] + semi_posterior_[from][sent][z+topics_];
    }
  }
}

// Find the MAP estimator for lambda.
// lambda[d] is proportional to the number of times tau = d. This is the number
// of links (tau = 1 and topic match) + number of times tau = 1 but topcis 
// don't match
void WebEM::FindLambda(const vector<vector<double> > &Udz,
		       const vector<vector<double> > &Vdz) {
  double norm = 0;
  double sum_etd = 0;
  double sum_v = 0, sum_u = 0;
  for (int d = 0; d < docs_.size(); d++) {
    double ETd = 0;
    for (int z = 0; z < topics_; z++) {
      ETd += Udz[d][z] + Vdz[d][z];
      sum_v += Vdz[d][z];
      sum_u += Udz[d][z];
    }
    sum_etd += ETd;
    lambda_[d] = ETd + gamma_[d] - 1;
    norm += lambda_[d];
  }
  cerr << "sum_etd = " <<  sum_etd << endl;
  lambda_[docs_.size()] = total_sentences_in_corpus_*k_link_weight - sum_etd + 
                          gamma_[docs_.size()] - 1;
  norm += lambda_[docs_.size()];
  cerr << "last gamma = " << gamma_[docs_.size()] << ", norm = " << norm
       << ", last lambda = " << lambda_[docs_.size()] << endl;
  cerr << "sum v = " <<sum_v << ", sum_u = " << sum_u <<endl;
  Normalize(norm, &lambda_);

  CheckProb::CheckProbVector(lambda_, "find lambda");
}

// The M-step of the EM algorithm for WEB HTMM.
void WebEM::MStep() {
  vector<double> dummy(topics_);
  vector<vector<double> > Udz(docs_.size(), dummy);
  vector<vector<double> > Vdz(docs_.size(), dummy);
  // Udz must be computed before any theta or lambda is modified.
  cerr << "before compute udz\n";
  NewComputeUdz(&Udz);
  cerr << "after compute udz\n";
  ComputeVdz(&Vdz);

  /*  ReadUdzVdz("/cs/vis/amitg/webLDA/data/ZL3/params_D_8283_K_5_eps_1.0.zl",
      "/cs/vis/amitg/webLDA/data/ZL3/params_D_8283_K_5_eps_1.0.tau",
      &Udz, &Vdz);
  */

  cerr << "after compute vdz\n";
  //  cerr << "before find epsilon\n";
  epsilon_ = 1;
  //      FindEpsilon();
  //  ReadPhi("/cs/vis/amitg/webLDA/data/very_short/d_D_100_K_5_eps_1.0.true_phi");
  cerr << "before find phi\n";
  FindPhi();
  
  //  ReadTheta("/cs/vis/amitg/webLDA/data/very_short/d_D_100_K_5_eps_1.0.true_theta");
  cerr << "before find theta\n";
  FindTheta(Udz, Vdz);

  cerr << "before find lambda\n";
  FindLambda(Udz, Vdz);
  //  ReadLambda("/cs/vis/amitg/webLDA/data/ZL3/lambda.init");
  cerr << "end Mstep\n";
}

void WebEM::ReadTheta(const char *fname) {
  ifstream in(fname);
  if (!in || in.fail()) {
	ERROR("can't open file "<< fname << "for reading");
  }
  int d, t;
  in >> d >> t;
  for (int i = 0; i < docs_.size(); i++) {
    for (int j = 0; j < topics_; j++) {
      in >> theta_[i][j];
    }
  }
  in.close();
}

// Read Phi from a file - only for debug
void WebEM::ReadPhi(const char *fname) {
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

// Prepares things for the Forward Backward and use it.
void WebEM::EStep() {
  loglik_ = 0;
  ComputeNoLinkProbGivenTopic();
  //  ReadLatentVars("/cs/vis/amitg/webLDA/data/ZL3/params_D_8283_K_5_eps_1.0.lat");
  for (int d = 0; d < docs_.size(); d++) {
    double ll = 0;
    EStepSingleDoc(d, &ll);
    loglik_ += ll;
  }
  // Here we take into account the priors of the parameters when computing
  // the likelihood:
  //  cerr << "incorporating priors\n";
  IncorporatePriorsIntoLikelihood();
  //  cerr << "after incorporate\n";
}

// Performs the Estep for a single document, namely computes the posterior
// state probabilities using the efficient forward backward algorithm (the
// parameters are given from the Mstep).
void WebEM::EStepSingleDoc(int d, double *ll) {
  // likelihood is computed using the forward backward pass taking into
  // account all observations.
  //  cerr << "starting Estep sinel d = " << d << "\n";
  ComputeSentenceTopicPosterior(d, ll);
  /*  NewComputeSentenceTopicSemiPosterior(d);*/
  //    cerr << "emd Estep single \n";
}

#if 0 
// OBSOLETE -- seems to be the wrong thing to compute
// Compute "semi posterior" probabilities - 
// i.e. compute topic posterior like in HTMM, ignoring link information.
void WebEM::ComputeSentenceTopicSemiPosterior(int d) {
  ERROR("deprecated version - probably it's not right mathematically");
  double dummy_ll_var = 0, *dummy_ll;
  dummy_ll = &dummy_ll_var;
  vector<double> dummy(topics_);
  vector<vector<double> > local(docs_[d]->size(), dummy);
  double local_ll = 0;
  EM::ComputeLocalProbsForDoc(d, &local, &local_ll);
  
  for (int i = 0; i < docs_[d]->size(); i++) {
    CheckProb::CheckProbVector(local[i], "semi posterior: local");
  }
  vector<double> init_probs(topics_*2);
  for (int i = 0; i < topics_; i++) {
    init_probs[i] = theta_[d][i];
    init_probs[i+topics_] = 0;  // Document must begin with a topic transition.
  }
  CheckProb::CheckProbVector(init_probs, "semi posterior: init_probs");
  FastRestrictedHMM f;
  f.ForwardBackward(epsilon_, theta_[d], local, init_probs,
                    &(semi_posterior_[d]), dummy_ll);
  for (int i = 0; i < docs_[d]->size(); i++) {
    CheckProb::CheckProbVector(semi_posterior_[d][i],
			       "semi posterior: semi_posterior");
  }
  *dummy_ll = *dummy_ll + local_ll;
}
#endif

// Compute "semi posterior" probabilities - 
// i.e. compute topic posterior like in HTMM, ignoring the link information
// for each hidden variable in its turn (considering the other links).
// NOTICE - this is not equivalent to the previous version
/*void WebEM::NewComputeSentenceTopicSemiPosterior(int d) {
  for (int i = 0; i < docs_[d]->size(); i++) {
    RecomputeSemiPosterior(d,i);
  }
  }*/

// Recompute the semi posterior for sentence i in document d.
  void WebEM::RecomputeSemiPosterior(int d, int i, vector<double> *semi) {
  vector<double> links_probs(topics_);
  double dummy_ll;
  // if there is an outgoing link
  if (links_from_[d].find(i) != links_from_[d].end()) {
    ComputeLinkProbs(links_from_[d][i], &links_probs, &dummy_ll);
  } else {  // if there is no outgoing link
    ComputeNonLinkProbs(&links_probs, &dummy_ll);
  }
  double norm = 0;
  for (int z = 0; z < topics_; z++) {
    // assuming links_probs != 0, otherwise we have a problem...
    // it shouldn't be 0 cause estimators for theta and lambda are never 0
    // (because of the dirichlet prior).
    if (links_probs[z] == 0) {
      ERROR("links probs = 0");
    }
    (*semi)[z] /*_posterior_[d][i][z]*/ = p_dwzpsi_[d][i][z] / links_probs[z];
    (*semi)[z+topics_] = //_posterior_[d][i][z+topics_] = 
      p_dwzpsi_[d][i][z+topics_] / links_probs[z];
    norm += (*semi)[z] + (*semi)[z+topics_]; // semi_posterior_[d][i][z] + semi_posterior_[d][i][z+topics_];
  }
  Normalize(norm, semi); //&(semi_posterior_[d][i]));
  CheckProb::CheckProbVector(*semi, "recompute semi posterior");
}

// Compute posterior probabilities for topics of sentences considering 
// link information.
void WebEM::ComputeSentenceTopicPosterior(int d, double *ll) {
  vector<double> dummy(topics_);
  vector<vector<double> > local(docs_[d]->size(), dummy);
  double local_ll = 0;
  ComputeLocalProbsForDocWithLink(d, &local, &local_ll);
  for (int i = 0; i < docs_[d]->size(); i++) {
    CheckProb::CheckProbVector(local[i], "posterior: local");
  }  
  vector<double> init_probs(topics_*2);
  for (int i = 0; i < topics_; i++) {
    init_probs[i] = theta_[d][i];
    init_probs[i+topics_] = 0;  // Document must begin with a topic transition.
  }
  FastRestrictedHMM f;
  f.ForwardBackward(epsilon_, theta_[d], local, init_probs,
                    &(p_dwzpsi_[d]), ll);
  for (int i = 0; i < docs_[d]->size(); i++) {
    CheckProb::CheckProbVector(p_dwzpsi_[d][i], "posterior: p_dwzpsi");
  }  
  *ll = *ll + local_ll;  
}

// Computes the local probabilities ("emission probabilities") considering
// the existence and non-existence of links.
void WebEM::ComputeLocalProbsForDocWithLink(int d,
					    vector<vector<double> > *local,
					    double *local_ll) {
  for (int i = 0; i < docs_[d]->size(); i++) {
    int link = -1;
    if (links_from_[d].find(i) != links_from_[d].end()) {
      link = links_from_[d][i];
    }
    ComputeLocalProbsForSentenceWithLink(link, docs_[d]->GetSentence(i),
					 &((*local)[i]), local_ll);
  }
}

// Computes local probabilities considering link / no link for a sentence.
  void WebEM::ComputeLocalProbsForSentenceWithLink(int link,
						   const Sentence &sen,
						   vector<double> *local,
						   double *local_ll) {
  vector<double> words_probs(topics_);
  double words_ll = 0;
  EM::ComputeLocalProbsForItem(sen, &words_probs, &words_ll);
  CheckProb::CheckProbVector(words_probs,
			     "local for sentence with link: words_probs");
  vector<double> links_probs(topics_);
  double link_ll = 0;
  if (link != -1) {
    // no norlalization in this case, so ll is not affected.
    ComputeLinkProbs(link, &links_probs, &link_ll);
  } else {
    ComputeNonLinkProbs(&links_probs, &link_ll);
  }
  CheckProb::CheckProbVector(links_probs,
			     "local for sentence with link: link_probs");
  double norm = 0;
  for (int z = 0; z < topics_; z++) {
    (*local)[z] = words_probs[z] * links_probs[z];
    norm += (*local)[z];
  }
  Normalize(norm, local);
  *local_ll += log(norm) + link_ll + words_ll;
}

// Computes the local probability induced (only) by the existence of link to
// document 'link_to'.
void WebEM::ComputeLinkProbs(int link_to, vector<double> *local,
			     double *link_ll) {
  double norm = 0;
  for (int z = 0; z < topics_; z++) {
    if ((link_to < 0) || (link_to >= docs_.size())) {
      ERROR("illegal link " << link_to);
    }
    // it should have been times lambda_[link_to_], but that's the same for all 
    // topics and canecles when normalizing.
    (*local)[z] = pow(theta_[link_to][z], k_link_weight);
    norm += (*local)[z];
  }
  Normalize(norm, local);
  // likelihood should take into account the normalization - lambda_[d].
  *link_ll += log(lambda_[link_to]) + log(norm);
}

// Computes the local probability induced by (only) the non existence of a link
// to document 'link_to'.
  void WebEM::ComputeNonLinkProbs(vector<double> *local,
				  double *link_ll) {
  double norm = 0;
  for (int z = 0; z < topics_; z++) {
    (*local)[z] = pr_no_link_given_topic_[z];
    norm += (*local)[z];
  }
  // TODO - the normalization can be done once and for all in
  // the ComputeSumProbLinkGivenTopic method.
  Normalize(norm, local);
  *link_ll += log(norm);
}

// Computes the probability for no link given each topic.
// This probability is equal to 1 - sum_d Pr(link=d | z) =
// 1 - sum_d [ Pr(tau = d) * Pr(theta_d = z) = 
// 1 - sum_d [ lambda_d * theta_d(z)
void WebEM::ComputeNoLinkProbGivenTopic() {
  for (int z = 0; z < topics_; z++) {
    pr_no_link_given_topic_[z] = 1;
    for (int d = 0; d < docs_.size(); d++) {
      pr_no_link_given_topic_[z] -= lambda_[d]*pow(theta_[d][z], k_link_weight);
    }
  }
}

// Saves all of the learned parameters, distribution over hidden variables
// (in this case only the (topic, psi) corresponding to a sentence), the 
// log likelihood and command line parameters.
void WebEM::SaveAll(const char *base_name) {
  EM::SaveAll(base_name);
  SaveLambda(base_name);
  //  SaveCmdLineArgs(base_name);
  //  SaveSemiPosteriors(base_name);
}

// Saves the semi posterior probabilities
/*void WebEM::SaveSemiPosteriors(const char *base_name) const {
  string fname(string(base_name)+".semi");
  ofstream out(fname.c_str());
  out << docs_.size() << "\t" << topics_ << endl;
  for (int d = 0; d < docs_.size(); d++) {
    out << docs_[d]->size() << endl;
    for (int i = 0; i < docs_[d]->size(); i++) {
      for (int z = 0; z < 2*topics_; z++) {
	out << semi_posterior_[d][i][z] << "\t";
      }
      out << endl;
    }
    out << endl;
  }
  out.close();
  }*/

// Saves Lambda
void WebEM::SaveLambda(const char *base_name) {
  std::string str(std::string(base_name)+std::string(".lambda"));
  ofstream out(str.c_str());
  if (!out || out.fail()) {
    ERROR("can't open output file " << str.c_str() << " for writing");
  }
  out << docs_.size() << endl;
  for (int d = 0; d < docs_.size(); d++) {
    out << lambda_[d] << "\n";
  }
  out << lambda_[docs_.size()] << endl;
  out.close();
}

// Saves the command line arguments.
void WebEM::SaveCmdLineArgs(const char *base_name) {
  string str(string(base_name) + string(".cmd"));
  ofstream out(str.c_str());
  if (!out || out.fail()) {
    ERROR("can't open output file " << str.c_str() << " for writing");
  }
  out << "topics = " << topics_ << endl;
  out << "alpha = " << alpha_ << endl;
  out << "beta = " << beta_ << endl;
  out << "iters = " << iters_ << endl;
  out << "and more to come ... " << endl;
  out.close();
}

// Reads lambda from a file, for debug purpose.
void WebEM::ReadLambda(const char *lambda_file) {
  cerr << "WARNING: reading lambda\n";
  ifstream in(lambda_file);
  if (!in || in.fail()) {
    ERROR("can't open file " << lambda_file << " for reading");
  }
  int dummy;
  in >> dummy;
  if (dummy != docs_.size()) {
    ERROR("wrong number of docs in lambda file, dummy = " << dummy
	  << ", size(docs) = " << docs_.size());
  }
  for (int d = 0; d < docs_.size()+1; d++) {
    in >> lambda_[d];
  }
  CheckProb::CheckProbVector(lambda_, "read lambda: lambda_");
  in.close();
}

// Adds the Dirichlet priors to the likelihood computation.
void WebEM::IncorporatePriorsIntoLikelihood() {
  IncorporatePriorOnTheta();
  IncorporatePriorOnPhi();
  IncorporatePriorOnLambda();
}

// Incorporates the prior in Lambda into the likelihood.
void WebEM::IncorporatePriorOnLambda() {
  // the +1 is for the no-link option.
  for (int d = 0; d < docs_.size() + 1; d++) {
    loglik_ += (gamma_[d] - 1)*log(lambda_[d]);
  }
}

#if 0
// Reads the latent varaibles ZL
void WebEM::ReadZL(char *fname) {
  ifstream in(fname);
  if (!in || in.fail()) {
    ERROR("can't open file " << fname << " for reading");
  }
  int D, L;
  in >> D;
  //  cerr << "D = " << D << endl;
  zl_.resize(D);
  //  cerr << "after resize\n";
  for (int d = 0; d < D; d++) {
    in >> L;
    //    cerr << "L = " << L << endl;
    zl_[d].resize(L);
    //    cerr << "after second resize\n";
    for (int i = 0; i < L; i++) {
      in >> zl_[d][i];
      zl_[d][i]--;
    }
    //    cerr << "end of loop\n";
  }
  in.close();
}

void WebEM::ReadTau(char *fname) {
  ifstream in(fname);
  if (!in || in.fail()) {
    ERROR("can't open file " << fname << " for reading");
  }
  int D, L;
  in >> D;
  tau_.resize(D);
  //  cerr << "after resize\n";
  for (int d = 0; d < D; d++) {
    in >> L;
    tau_[d].resize(L);
    for (int i = 0; i < L; i++) {
      in >> tau_[d][i];
      if (tau_[d][i] == 0) {
	tau_[d][i] = docs_.size();
      } else {
	tau_[d][i]--;
      }
    }
  }
  in.close();  
}

// go over the entire corpus, in the places where there's no link and tau
// is not null, increase by 1 the udz of document tau according to its zl.
void WebEM::ReadUdz(vector<vector<double> > *Udz) {
  for (int d = 0; d < docs_.size(); d++) {
    for (int i = 0; i < docs_[d]->size(); i++) {
      // if there's no outgoing link
      if (links_from_[d].find(i) != links_from_[d].end()) {
	int doc_to = tau_[d][i];
	int topic = zl_[d][i];
	(*Udz)[doc_to][topic]++;
      }
    }
  }
}

void WebEM::ReadVdz(vector<vector<double> > *Vdz) {
  for (int d = 0; d < docs_.size(); d++) {
    vector<pair<int, int> >::const_iterator iter;
    for (iter = links_to_[d].begin(); iter != links_to_[d].end(); iter++) {
      int z = zl_[iter->first][iter->second];
      (*Vdz)[d][z]++;
    }
  }
}

#endif

  void WebEM::ReadUdzVdz(const char *zfile, const char *tfile,
			 vector<vector<double> > *Udz,
			 vector<vector<double> > *Vdz) {
    ifstream in_zl(zfile);
    ifstream in_tau(tfile);
    if (!in_zl || in_zl.fail() || !in_tau || in_tau.fail()) {
      ERROR("can't open files " << zfile << ", " << tfile << " for reading");
    }
    int zl, tau, D, L;
    in_zl >> D;
    in_tau >> D;
    for (int d = 0; d < D; d++) {
      in_zl >> L;
      in_tau >> L;
      for (int i = 0; i < L; i++) {
	in_zl >> zl;
	zl--;
	in_tau >> tau;
	if (tau == 0) {
	  tau = docs_.size()+1;
	}
	tau--;
	if (i < docs_[d]->size()) {
	  if (links_from_[d].find(i) != links_from_[d].end()) {
	    int doc_to = links_from_[d][i];
	    if (doc_to != tau) {
	      ERROR("doc_to = " << doc_to << " , tau = " << tau);
	    }
	    (*Vdz)[doc_to][zl] += 1;
	  } else {
	    if (tau != docs_.size()) {
	      (*Udz)[tau][zl] += 1;
	    }
	  }
	}
      }
    }
    in_zl.close();
    in_tau.close();
  }

  WebEM::~WebEM() {
  }

}  // namespace

