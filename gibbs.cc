#include <assert.h>
#include <stdio.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "gibbs.h"
#include "author.h"

#define REP_NO 300
#define DEFAULT_HYPER_LAG 0
#define DEFAULT_SHUFFLE_LAG 100
#define DEFAULT_SAMPLE_ALPHA 0
#define DEFAULT_SAMPLE_ETA 0
#define BUF_SIZE 100

const int MAX_ITER_INF = 1000;

namespace atm {

// =======================================================================
// GibbsState
// =======================================================================

GibbsState::GibbsState()
    : alpha_(1.1),
    	score_(0.0),
      alpha_score_(0.0),
      eta_score_(0.0),
      max_score_(0.0),
      iteration_(0),
      shuffle_lag_(DEFAULT_SHUFFLE_LAG),
      hyper_lag_(DEFAULT_HYPER_LAG),
      sample_eta_(0),
      sample_alpha_(0) {
}


double GibbsState::computeGibbsScore() {
  // Compute the alpha, Eta scores.
  alpha_score_ = AllAuthorsUtils::AlphaScores(alpha_);
  eta_score_ = AllTopicsUtils::EtaScores(&all_topics_);

  score_ = alpha_score_ + eta_score_;
  cout << "Alpha_score: " << alpha_score_ << endl;
  cout << "Eta_score: " << eta_score_ << endl;
  cout << "Score: " << score_ << endl;

  // Update the maximum score if necessary.
  if (score_ > max_score_ || iteration_ == 0) {
    max_score_ = score_;
  }

  return score_;
}

// =======================================================================
// GibbsUtils
// =======================================================================

void GibbsSampler::ReadGibbsInput(
    GibbsState* gibbs_state,
    const std::string& filename_corpus,
    const std::string& filename_authors,
    const std::string& filename_settings) {
  // Read hyperparameters from file
  ifstream infile(filename_settings.c_str());
  char buf[BUF_SIZE];

  int sample_eta = 0, sample_alpha = 0, topic_no = 0;
  double alpha =  1.0, eta = 1.0;

  while (infile.getline(buf, BUF_SIZE)) {
    istringstream s_line(buf);
    // Consider each line at a time.
    std::string str;
    getline(s_line, str, ' ');
    std::string value;
    getline(s_line, value, ' ');
    if (str.compare("ETA") == 0) {
     	eta = atof(value.c_str());
    } else if (str.compare("ALPHA") == 0) {
    	alpha = atof(value.c_str());
    } else if (str.compare("SAMPLE_ETA") == 0) {
      sample_eta = atoi(value.c_str());
    } else if (str.compare("SAMPLE_ALPHA") == 0) {
      sample_alpha = atoi(value.c_str());
    } else if (str.compare("TOPIC_NO") == 0) {
    	topic_no = atoi(value.c_str());
    }
  }

  infile.close();

  // Create corpus.
  Corpus* corpus = gibbs_state->getMutableCorpus();
  CorpusUtils::ReadCorpus(filename_corpus, filename_authors, corpus, topic_no);

  // Create all topics.
  AllTopics* all_topics = gibbs_state->getMutableAllTopics();
  for (int i = 0; i < topic_no; i++) {
  	all_topics->addTopic(corpus->getWordNo(), eta);
  }

  gibbs_state->setSampleEta(sample_eta);
  gibbs_state->setSampleAlpha(sample_alpha);
  gibbs_state->setAlpha(alpha);

}

void GibbsSampler::InitGibbsState(
    GibbsState* gibbs_state) {

  Corpus* corpus = gibbs_state->getMutableCorpus();
  AllTopics* all_topics = gibbs_state->getMutableAllTopics();
  double alpha = gibbs_state->getAlpha();

  // Permute Authors in the corpus.
  CorpusUtils::PermuteDocuments(corpus);

  for (int i = 0; i < corpus->getDocuments(); i++) {
    Document* document = corpus->getMutableDocument(i);
    DocumentUtils::SampleAuthors(document, all_topics);
  }

  AllAuthors& all_authors = AllAuthors::GetInstance();

  for (int i = 0; i < all_authors.getAuthors(); i++) {
    Author* author = all_authors.getMutableAuthor(i);

   

    // Permute the words in the current author.
    AuthorUtils::PermuteWords(author);

    

    // Sample topics for this author, without permuting the words
    // in the author and without removing words from topics.
    AuthorUtils::SampleTopics(author,
                                0,
                                false,
                                alpha,
                                all_topics);
  }

  // Compute the Gibbs score.
  double gibbs_score = gibbs_state->computeGibbsScore();

  cout << "Gibbs score = " << gibbs_score << endl;
}

GibbsState* GibbsSampler::InitGibbsStateRep(
    const string& filename_corpus,
    const string& filename_authors,
    const string& filename_settings,
    long random_seed) {
  double best_score = 0.0;
  GibbsState* best_gibbs_state = nullptr;

  for (int i = 0; i < REP_NO; i++) {
    // Initialize the random number generator.
    Utils::InitRandomNumberGen(random_seed);

    GibbsState* gibbs_state = new GibbsState();
    ReadGibbsInput(gibbs_state, filename_corpus, filename_authors, filename_settings);

    // Initialize the Gibbs state.
    InitGibbsState(gibbs_state);

    // Update Gibbs best state if necessary.
    if (gibbs_state->getScore()  > best_score || i == 0) {
      if (best_gibbs_state != nullptr) {
        delete best_gibbs_state;
      }
      best_gibbs_state = gibbs_state;
      best_score = gibbs_state->getScore();
      cout << "Best initial state at iteration: " <<
          i << " score " << best_score << endl;
    } else {
      delete gibbs_state;
    }
  }

  return best_gibbs_state;
}

void GibbsSampler::IterateGibbsState(GibbsState* gibbs_state) {
  assert(gibbs_state != nullptr);

  
  Corpus* corpus = gibbs_state->getMutableCorpus();
  AllTopics* all_topics = gibbs_state->getMutableAllTopics();
  double alpha = gibbs_state->getAlpha();

  gibbs_state->incIteration(1);
  int current_iteration = gibbs_state->getIteration();

  cout << "Start iteration..." << gibbs_state->getIteration() << endl;

 

  // Determine value for permute.
  int permute = 0;
  int shuffle_lag = gibbs_state->getShuffleLag();
  if (shuffle_lag > 0) {
    permute = 1 - (current_iteration % shuffle_lag);
  }

  // Permute documents in corpus.
  if (permute == 1) {
    CorpusUtils::PermuteDocuments(corpus);
  }

  for (int i = 0; i < corpus->getDocuments(); i++) {
    Document* document = corpus->getMutableDocument(i);
    DocumentUtils::SampleAuthors(document, all_topics);
  }

  AllAuthors& all_authors = AllAuthors::GetInstance();

  for (int i = 0; i < all_authors.getAuthors(); i++) {
    Author* author = all_authors.getMutableAuthor(i);
    AuthorUtils::SampleTopics(author,
                              permute,
                              true,
                              alpha,
                              all_topics);
  }

  // Sample hyper-parameters.
  if (gibbs_state->getHyperLag() > 0 &&
      (current_iteration % gibbs_state->getHyperLag() == 0)) {
   
    if (gibbs_state->getSampleAlpha() == 1) {
    	// TODO
    }
    if (gibbs_state->getSampleEta() == 1) {
    	// TODO
    }
  }

  // Compute the Gibbs score with the new parameter values.
  double gibbs_score = gibbs_state->computeGibbsScore();

  cout << "Gibbs score at iteration "
       << gibbs_state->getIteration() << " = " << gibbs_score << endl;
}

void GibbsSampler::InferATM(
          const string& filename_corpus,
          const string& filename_authors,
          const string& filename_topics,
          const string& filename_other,
          long random_seed) {
  Utils::InitRandomNumberGen(random_seed);

  GibbsState* gibbs_state = new GibbsState();

  LoadState(gibbs_state, filename_topics, filename_other);

  AllTopics* all_topics = gibbs_state->getMutableAllTopics();
  int topic_no = all_topics->getTopics();
  double alpha = gibbs_state->getAlpha();
  bool inf = true;

  Corpus* corpus = gibbs_state->getMutableCorpus();
  CorpusUtils::ReadCorpus(filename_corpus, filename_authors, corpus, topic_no);

  for (int i = 0; i < corpus->getDocuments(); i++) {
    Document* document = corpus->getMutableDocument(i);
    DocumentUtils::SampleAuthors(document, all_topics, inf);
  }

  AllAuthors& all_authors = AllAuthors::GetInstance();

  for (int i = 0; i < all_authors.getAuthors(); i++) {
    Author* author = all_authors.getMutableAuthor(i);


    // Sample topics for this author, without permuting the words
    // in the author and without removing words from topics.
    AuthorUtils::SampleTopics(author,
                                0,
                                false,
                                alpha,
                                all_topics,
                                inf);
  }

  char filename[1000];
  sprintf(filename, "result/inf-likelihood-%d.dat", topic_no);
  ofstream ofs(filename);
  for (int i = 0; i < MAX_ITER_INF; i++) {
    IterateGibbsState(gibbs_state);
  }

  // Compute the Gibbs score with the new parameter values.
  double gibbs_score = gibbs_state->computeGibbsScore();

  ofs << gibbs_score << endl;

}

void GibbsSampler::SaveState(
          GibbsState* gibbs_state,
          const string& filename_other) {
  Corpus* corpus = gibbs_state->getMutableCorpus();
  AllTopics* all_topics = gibbs_state->getMutableAllTopics();
  int topic_no = all_topics->getTopics();
  assert(topic_no > 0);
  double eta = all_topics->getMutableTopic(0)->getEta();
  int term_no = corpus->getWordNo();
  double alpha = gibbs_state->getAlpha();

  ofstream ofs(filename_other);
  ofs << topic_no << endl;
  ofs << term_no << endl;
  ofs << eta << endl;
  ofs << alpha << endl;
  ofs.close();
}

void GibbsSampler::LoadState(
          GibbsState* gibbs_state,
          const string& filename_topics,
          const string& filename_other) {
  AllTopics* all_topics = gibbs_state->getMutableAllTopics();
  ifstream ifs(filename_other);
 
  char buf[BUF_SIZE];
  int topic_no = 0;
  int term_no = 0;
  double eta = 0;
  double alpha = 0.0;

  while(ifs.getline(buf, BUF_SIZE)) {
    istringstream iss(buf);
    string str;
    getline(iss, str, ' ');
    string value;
    getline(iss, value, ' ');

    if (value.compare("topic_no") == 0) {
      topic_no = atoi(value.c_str());
    } else if (value.compare("term_no") == 0) {
      term_no = atoi(value.c_str());
    } else if (value.compare("eta") == 0) {
      eta = atof(value.c_str());
    } else if (value.compare("alpha") == 0) {
      alpha = atof(value.c_str());
    }
  }
  ifs.close();

  gibbs_state->setAlpha(alpha);

  assert(topic_no > 0);
  assert(term_no > 0);

  cout << "loading " << filename_other << " successfully." << endl;
  cout << "topic_no : " << topic_no << endl;
  cout << "term_no : " << term_no << endl;
  cout << "eta : " << eta << endl;
  cout << "alpha : " << alpha << endl;

  ifs = ifstream(filename_topics);
  for (int i = 0; i < topic_no; i++) {
    all_topics->addTopic(term_no, eta);
    Topic* topic = all_topics->getMutableTopic(i);

    ifs.getline(buf, BUF_SIZE);
    istringstream iss(buf);

    for (int w = 0; w < term_no; w++) {
      string str;
      iss >> str;
      topic->setWordCount(w, atoi(str.c_str()));
    }
  }

  ifs.close();
}

}  // namespace atm


