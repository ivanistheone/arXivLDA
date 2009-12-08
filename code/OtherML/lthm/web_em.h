#ifndef WEBHTMM_WEBEM_H__
#define WEBHTMM_WEBEM_H__

#include <map>
#include <vector>
#include "EM.h"

namespace htmm {

class WebEM : public EM {
 public:
  WebEM() {}

  virtual ~WebEM();

  void init(int topics, int words, double alpha, double beta, int iters,
            const char *fname, const char *data_dir, unsigned int seed,
            const char *links_file, const char *gamma_file,
	    const char *train_map_file);

  // Saves all the results 
  void SaveAll(const char *base_name);

 protected:

  //-------------
  // Data Members
  //-------------  

  // contains all links from (document,sentence) -> document
  // links_from_[from_doc][from_sen] = to_doc
  vector<map<int,int> > links_from_; // we'll need to look for links

  // contains all links incoming.
  // links_to_[to_doc][from_doc][from_sen]
  vector<vector<pair<int,int> > > links_to_;

  // "Posterior" probabilities that do not include link information.
  //  vector<vector<vector<double> > > semi_posterior_;

  // The expected occurances of each topic in the entire corpus.
  vector<double> semi_topic_in_corpus_;

  // The expected occurances of each topic associated with a link
  // (Z_L) in a certain document.
  vector<vector<double> > links_to_topic_counts_;

  // vector of length d+1, lambda[i] = Pr(tau=i), the last entry
  // denotes no link.
  vector<double> lambda_;

  // The Dirichlet prior on lambda.
  vector<double> gamma_;

  // Total number of sentences in corpus.
  int total_sentences_in_corpus_;

  // The probability for no-link given each topic.
  vector<double> pr_no_link_given_topic_;

  // only for debug
  //  vector<vector<int> > zl_;
  //  vector<vector<int> >  tau_;

  // The probability for no-link (topic is not given nor specified)
  //  vector<vector<double> > pr_no_link_;

  //---------
  // Methods:
  //---------

  // Reads a list of links from a file. Keep only the links where both
  // ends belong to the train set.
  void ReadTrainLinks(const char *fname);

  // Prints all links outgoing from a cretain document.
  void PrintLinksFrom(int doc);
  
  // Print all links pointing to a certain document.
  void PrintLinksTo(int doc);

  // The M-step of the EM algorithm for WEB HTMM.
  void MStep();

  // The E-step of the EM algorithm for WEB HTMM.
  void EStep();
  
  // Performs the Estep for a single document, namely computes the posterior
  // state probabilities using the efficient forward backward algorithm (the
  // parameters are given from the Mstep).
  void EStepSingleDoc(int d, double *ll);

  // Overrides the method in EM
  // Finds the MAP estimator for theta_d.
  virtual void FindSingleTheta(int d, const vector<double> &Udz,
			       const vector<double> &Vdz);
  
  // Computes Vdz - the expected number of topics Z_L corresponding to
  // an existing link pointing to document d with topic z.
  void ComputeVdz(vector<vector<double> > *Vdz);

  // Another version of the above method for debug purpose. Both
  // methods should give the same result.
  //  void ComputeVdz2(vector<vector<double> > *Vdz);
  
  // Compute Udz - the expected number of topics Z_L when there is no
  // link when tau was d (a link was considered) but Z_L was different
  // from the topic of the sentence from which the link would have
  // come out.
  void NewComputeUdz(vector<vector<double> > *Udz);
  void OldComputeUdz(vector<vector<double> > *Udz);

  void ReadTheta(const char *fname);

  // Read Phi from a file - only for debug
  void ReadPhi(const char *fname);

  // Counts the expected number of occurances of each topic in the corpus based
  // on the semi-posterior probabilities. It considers only topics associated 
  // with a sentence not with a link.
  void CountSemiTopicInCorpus();

  // Counts the number of occurances of each topic associated with a sentence
  // (not links) based on semi-posterior probabilities.
  void CountSemiTopicLinksInDocument(int d);

  // Find the MAP estimator for lambda.
  void FindLambda(const vector<vector<double> > &Udz,
		  const vector<vector<double> > &Vdz);

  // Initialize Lambda randomly
  void RandInitLambda();

  // Initialize Gamma, the Dirichlet prior on glambda (assymetric).
  void InitGamma(const char *gamma_file);

  // Computes the local probabilities ("emission probabilities") considering
  // the existence and non-existence of links.
  void ComputeLocalProbsForDocWithLink(int d, vector<vector<double> > *local,
				       double *local_ll);

  // Computes local probabilities considering link / no link for a sentence.
  void ComputeLocalProbsForSentenceWithLink(int link, const Sentence &sen,
					    vector<double> *local,
					    double *local_ll);

#if 0
  // Compute "semi posterior" probabilities - 
  // i.e. compute topic posterior like in HTMM, ignoring link information.
  void ComputeSentenceTopicSemiPosterior(int d);
#endif

  // Compute posterior probabilities for topics of sentences considering 
  // link information.
  void ComputeSentenceTopicPosterior(int d, double *ll);

  // Finds the theta for all documents in the train set.
  void FindTheta(const vector<vector<double> > &Udz,
		 const vector<vector<double> > &Vdz);

  // Counts the total number of sentences in corpus.
  void CountSentences();

  // compute the local probability induced (only) by the existence of link to
  // document 'link_to'
  void ComputeLinkProbs(int link_to, vector<double> *local, double *link_ll);

  // Compute the local probability induced by (only) the non existence of a link
  // to document 'link_to'.
  void ComputeNonLinkProbs(vector<double> *local, double *link_ll);
  
  // Saves Lambda
  void SaveLambda(const char *base_name);
    
  // Saves the command line arguments.
  void SaveCmdLineArgs(const char *base_name);

  // Allocate memory for the new data members: semi_posteriors_,
  // topic_in_corpus_, links_to_topic_counts_, lambda_.
  void AllocateLinksVars();

  // Computes the probability for no link given each topic.
  void ComputeNoLinkProbGivenTopic();

  // Computes the probability for no link when the topic is not given.
  void ComputeNoLinkProb(vector<vector<double> > *pr_no_link);

  // Reads lambda from a file, for debug purpose only.
  void ReadLambda(const char *lambda_file);

  // Compute the normalization factor in Udz.
  double ComputeUdzNormFactor();

  // Computes the term that sums over all the sentences from which there are no
  // outgoing links in the corpus
  double ComputeNoLinksTerm(vector<double> *no_links_term,
			    vector<double> *second_term);

  // Adds the Dirichlet priors to the likelihood computation.
  virtual void IncorporatePriorsIntoLikelihood();

  // Incorporates the prior in Lambda into the likelihood.
  void IncorporatePriorOnLambda();

  // Saves the semi posteriors to a file.
  //  void SaveSemiPosteriors(const char *base_name) const;

  // Recompute the semi posterior for sentence i in document d.
  void RecomputeSemiPosterior(int d, int i, vector<double> *semi);

  // Compute "semi posterior" probabilities - 
  // i.e. compute topic posterior like in HTMM, ignoring the link information
  // for each hidden variable in its turn (considering the other links).
  // NOTICE - this is not equivalent to the previous version
  void NewComputeSentenceTopicSemiPosterior(int d);

  // Reads the latent varaibles ZL
#if 0
  void ReadZL(char *fname);

  void ReadTau(char *fname);
  void ReadUdz(vector<vector<double> > *Udz);
  void ReadVdz(vector<vector<double> > *Vdz);
#endif

  void ReadUdzVdz(const char *zfile, const char *tfile,
			 vector<vector<double> > *Udz,
			 vector<vector<double> > *Vdz);

  int Factorial(int n);

  int NChooseK(int n, int k);

 private:

  // Disallow evil copy constructor
  WebEM(const WebEM &);

};

}  // namespace

#endif  //  WEBHTMM_WEBEM_H__
