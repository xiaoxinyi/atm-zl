#include <math.h>
#include <assert.h>

#include <gsl/gsl_sf.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>


#include "topic.h"

const int BUF_SIZE = 10000;


namespace atm {

// =======================================================================
// Topic
// =======================================================================
Topic::Topic(int corpus_word_no, double eta)
    : topic_word_no_(0),
      corpus_word_no_(corpus_word_no),
      word_counts_(corpus_word_no, 0),
      eta_(eta) {
}

void Topic::updateWordCount(int word_id, int update) {
  // Find the word counts for the word with word_id, and update the counts.
  word_counts_[word_id] += update;
  topic_word_no_ += update;
}

// =======================================================================
// TopicUtils
// =======================================================================

void TopicUtils::SaveTopic(
        Topic* topic, 
        ofstream& ofs, 
        ofstream& ofs_count) {
  ofs.precision(12);
  ofs << std::right;
  for (int i = 0; i < topic->getCorpusWordNo(); i++) {
    ofs << exp(topic->getLogPrWord(i)) << " ";
    ofs_count << topic->getWordCount(i) << " ";
  }
  ofs << endl;
  ofs_count << endl;
}

double TopicUtils::EtaScore(Topic* topic) {
  double score = 0.0;
  int word_count_size = topic->getCorpusWordNo();
  
  double eta = topic->getEta();
  double lgam_eta = gsl_sf_lngamma(eta);

  // The current eta score.
  score = gsl_sf_lngamma(word_count_size * eta);

  // Update the score based on the pre-computed Gamma (word count + eta)
  for (int i = 0; i < word_count_size; i++) {
  	if (topic->getWordCount(i) > 0) {
  		score += topic->getLgamWordCountEta(i) - lgam_eta;	
  	}
  }

  score -= gsl_sf_lngamma(topic->getTopicWordNo() + word_count_size * eta);


  return score;
}



// =======================================================================
// AllTopicsUtils 
// =======================================================================

double AllTopicsUtils::EtaScores(AllTopics* all_topics) {
	int topics = all_topics->getTopics();
	double score = 0.0;
	for (int i = 0; i < topics; i++) {
		Topic* topic = all_topics->getMutableTopic(i);
		score += TopicUtils::EtaScore(topic);
	}
	return score;
}


void AllTopicsUtils::SaveTopics(AllTopics* all_topics,
																const string& filename,
                                const string& filename_count) {
	ofstream ofs(filename);
	ofstream ofs_count(filename_count);

	int topics = all_topics->getTopics();
	for (int i = 0; i < topics; i++) {
		Topic* topic = all_topics->getMutableTopic(i);
		TopicUtils::SaveTopic(topic, ofs, ofs_count);
	}
	ofs.close();
  ofs_count.close();
}

void AllTopicsUtils::LoadTopics(AllTopics* all_topics,
                const string& filename_topics,
                const string& filename_other) {
  ifstream ifs(filename_other);
 
  char buf[BUF_SIZE];
  int topic_no = 0;
  int term_no = 0;
  double eta = 0;

  while(ifs.getline(buf, BUF_SIZE)) {
    istringstream iss(buf);
    string str;
    getline(iss, str, ' ');
    string value;
    getline(iss, value, ' ');

    if (str.compare("topic_no") == 0) {
      topic_no = atoi(value.c_str());
    } else if (str.compare("term_no") == 0) {
      term_no = atoi(value.c_str());
    } else if (str.compare("eta") == 0) {
      eta = atof(value.c_str());
    }
  }
  ifs.close();

  assert(topic_no > 0);
  assert(term_no > 0);

  cout << "loading " << filename_other << " successfully." << endl;
  cout << "topic_no : " << topic_no << endl;
  cout << "term_no : " << term_no << endl;
  cout << "eta : " << eta << endl;

  ifs = ifstream(filename_topics);
  int topic_word_no = 0;

  for (int i = 0; i < topic_no; i++) {
    all_topics->addTopic(term_no, eta);
    Topic* topic = all_topics->getMutableTopic(i);

    ifs.getline(buf, BUF_SIZE);
    istringstream iss(buf);

    for (int w = 0; w < term_no; w++) {
      string str;
      iss >> str;
      topic_word_no += atoi(str.c_str());
      topic->setWordCount(w, atoi(str.c_str()));
    }

    topic->setTopicWordNo(topic_word_no);
  }

  ifs.close();
  
}

vector<double> AllTopicsUtils::WordProbabilities(AllTopics* all_topics, int word_id) {
  int topic_no = all_topics->getTopics();
  vector<double> log_pr(topic_no, 0.0);

  for (int i = 0; i < topic_no; i++) {
    Topic* topic = all_topics->getMutableTopic(i);
    log_pr[i] = topic->getLogPrWord(word_id);
  }

  return log_pr;
}



}  // namespace atm