#ifndef GIBBS_H_
#define GIBBS_H_

#include <string>

#include "topic.h"
#include "utils.h"
#include "corpus.h"

namespace atm {

// The Gibbs state of the HLDA implementation.
// Each Gibbs state has a corpus and all topics, and
// keeps current scores, the current iteration and
// the sampling parameters.
class GibbsState {
 public:
 	GibbsState();

  // Computes the Gibbs score, which is a perplexity score for
  // the model.
  double computeGibbsScore();

  void setScore(double score) { score_ = score; }
  double getScore() const { return score_; }

  void setAlphaScore(double alpha_score) { alpha_score_ = alpha_score; }
  double getAlphaScore() const { return alpha_score_; }

  void setEtaScore(double eta_score) { eta_score_ = eta_score; }
  double getEtaScore() const { return eta_score_; }


  double getMaxScore() const { return max_score_; }
  void setMaxScore(double max_score) { max_score_ = max_score; }

  void setSampleEta(int sample_eta) { sample_eta_ = sample_eta; }
  void setSampleAlpha(int sample_alpha) { sample_alpha_ = sample_alpha; }
  
  void setCorpus(const Corpus& corpus) { corpus_ = corpus; }
  Corpus* getMutableCorpus() { return &corpus_; }

  int getIteration() const { return iteration_; }
  void setIteration(int iteration) { iteration_ = iteration; }
  void incIteration(int val) { iteration_ += val; }

  
  int getShuffleLag() const { return shuffle_lag_; }
  int getHyperLag() const { return hyper_lag_; }
  int getSampleEta() const { return sample_eta_; }
  int getSampleAlpha() const { return sample_alpha_; }
  

  void setAllTopics(const AllTopics& all_topics) { all_topics_ = all_topics; }
  AllTopics* getMutableAllTopics() { return &all_topics_; }

  void setAlpha(double alpha) { alpha_ = alpha; }
  double getAlpha() const { return alpha_; }
 private:
  Corpus corpus_;
  AllTopics all_topics_;
  double alpha_;

  // The current score obtained by summing the Eta, Gamma and
  // alpha scores.
  double score_;

  // Corresponds to the local distribution.
  double alpha_score_;

  // Corresponds to the topic prior Eta parameter.
  double eta_score_;


  // The current maximum score over several iterations.
  double max_score_;

  // Current iteration.
  int iteration_;

  // Sampling parameters.
  int shuffle_lag_;
  int hyper_lag_;
  int sample_eta_;
  int sample_alpha_;

};

// This class provides functionality for reading input for the
// Gibbs state, initializing the Gibbs state,
// and performing iterations of the Gibbs state.
class GibbsSampler {
 public:
  // Read input corpus and state parameters from file.
  static void ReadGibbsInput(
      GibbsState* gibbs_state,
      const std::string& filename_corpus,
      const std::string& filename_authors,
      const std::string& filename_settings);

  // Initialize Gibbs state.
  static void InitGibbsState(
      GibbsState* gibbs_state);

  // Initialize Gibbs state - repeat the initialization REP_NO,
  // by calling InitGibbsState.
  // Keep the Gibbs state with the best score.
  // rng_seed is the random number generator seed.
  static GibbsState* InitGibbsStateRep(
      const std::string& filename_corpus,
      const std::string& filename_authors,
      const std::string& filename_settings,
      long rng_seed);

  // Iterations of the Gibbs state.
  // Sample the document path and the word levels in the tree.
  // Sample hyperparameters: Eta, GEM mean and scale.
  static void IterateGibbsState(GibbsState* gibbs_state);
};

}  // namespace atm

#endif  // GIBBS_H_
