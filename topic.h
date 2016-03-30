#ifndef TOPIC_H_
#define TOPIC_H_

#include <math.h>

#include <map>
#include <vector>
#include <gsl/gsl_sf.h>
#include <iostream>

using namespace std;

namespace atm {

// The topic in the atm implementation.
// Each topic contains word statistics,
// the number of authors it is assigned to,
// the topic id, the level in the tree, a scaling factor,
// pointers to the parent and children topics,
// a pointer to the tree this topic belongs to,
// and a probability for sampling the path.
class Topic {
public:
	Topic(int corpus_word_no, double eta);
	
  double getLogPrWord(int word_id) const {
  	return log(eta_ + word_counts_[word_id]) -
  				 log(eta_ * corpus_word_no_ + topic_word_no_); }

  int getWordCount(int word_id) const { return word_counts_[word_id]; }
  void setWordCount(int word_id, int count) { word_counts_[word_id] = count; }
	// Update the count of a word in a given topic.
  void updateWordCount(int word_id, int update);

  void setTopicWordNo(int topic_word_no) { topic_word_no_ = topic_word_no; }
  int getTopicWordNo() const { return topic_word_no_; }

  double getLgamWordCountEta(int word_id) const {
    return gsl_sf_lngamma(word_counts_[word_id] + eta_);
  }

  int getCorpusWordNo() const { return corpus_word_no_; }

  double getEta() const { return eta_; }
  void setEta(double eta) { eta_ = eta; }

private:
	// Total number of words assigned to this topic.
	int topic_word_no_;

	// Total number of words in the corpus.
	int corpus_word_no_;

	// Word counts.
	vector<int> word_counts_;

	// Eta
	double eta_;

};

// This class provides functionality for calculating Eta.
class TopicUtils {
 public:
  // Compute the Eta score which is a topic score.
  // The Eta parameter represents the expected variance of the
  // underlying topics.
  static double EtaScore(Topic* topic);

 
  static void SaveTopic(Topic* topic, 
  											ofstream& ofs,
  											ofstream& ofs_count);

};



// AllTopics store all the topics globally.
// This class provides functionality of 
// adding new topic.
class AllTopics {
public:
	vector<Topic>& getMutableTopics() { return topics_; }
	int getTopics() const { return topics_.size(); }
	void addTopic(int corpus_word_no, double eta) {
		topics_.emplace_back(Topic(corpus_word_no, eta));
	}
	Topic* getMutableTopic(int i) {
		return &topics_[i];
	}

private:
	// All topics.
	vector<Topic> topics_;
	
};

// This class provides functionality for computing
// Eta scores.
class AllTopicsUtils {
public:
	// Compute eta score.
	// eta - dirichlet distribution parameter of each topic.
	static double EtaScores(AllTopics* all_topics);

	static void SaveTopics(AllTopics* all_topics, 
												 const string& filename,
												 const string& filename_count);

	static void LoadTopics(AllTopics* all_topics,
												const string& filename_topics,
												const string& filename_other);

	static vector<double> WordProbabilities(AllTopics* all_topics, int word_id);

};

}  // namespace hatm

#endif  // TOPIC_H_