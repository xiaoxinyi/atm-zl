#include <math.h>

#include <gsl/gsl_sf.h>
#include <fstream>
#include <iomanip>


#include "topic.h"


namespace atm {

// =======================================================================
// Topic
// =======================================================================
Topic::Topic(int corpus_word_no, double eta)
    : topic_word_no_(0),
      corpus_word_no_(corpus_word_no),
      word_counts_(corpus_word_no),
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

void TopicUtils::SaveTopic(Topic* topic, ofstream& ofs) {
  ofs.precision(10);
  ofs << setw(12);
  ofs << std::right;
  for (int i = 0; i < topic->getCorpusWordNo(); i++) {
    ofs << exp(topic->getLogPrWord(i)) << " ";
  }
  ofs << endl;
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
																const string& filename) {
	ofstream ofs(filename);
	
	int topics = all_topics->getTopics();
	for (int i = 0; i < topics; i++) {
		Topic* topic = all_topics->getMutableTopic(i);
		TopicUtils::SaveTopic(topic, ofs);
	}
	ofs.close();
}



}  // namespace atm