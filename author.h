#ifndef AUTHOR_H_
#define AUTHOR_H_

#include <vector>
#include <string>
#include <unordered_map>


#include "document.h"



namespace atm {


class Topic;
class Tree;
class AllTopics;

// An author has an id, a author score, a topic path
// from the root of the tree to the leaf and
// statistics for words assigned to different levels in
// the tree - level counts and log probabilities for the
// levels.
// The class records map from id to author.
class Author {
public:
	Author();
	Author(int id, int topic_no);

	int getId() const { return id_; }

	int getTopicCounts(int topic_id) const { return topic_counts_.at(topic_id); }
	void setTopicCounts(int topic_id, int count) { topic_counts_[topic_id] = count; }
	int getSumTopicCounts(int topic_no) const;
	void updateTopicCounts(int topic_id, int value) {
		topic_counts_.at(topic_id) += value;
	}

	int getTopicNo() const { return topic_no_; }
	void setTopicNo(int topic_no) { topic_no_ = topic_no; }

	double getScore() const { return score_; }
	void setScore(double score) { score_ = score; }

	int getWords() const { return words_.size(); }
	void setWords(vector<int>&& words) { words_ = move(words); }

	int getWord(int i) { return words_.at(i); }
	void setWord(int i, const int& word) { words_.at(i) = word; }
	void addWord(int word) { words_.push_back(word); }
	void removeWord(int word);

private:
	// Author id;
	int id_;

	// Topic number.
	int topic_no_;

	vector<int> words_;
	// Word counts.
	// std::vector<int> word_counts_;

	// Topic counts.
	vector<int> topic_counts_;

	// Author score.
	double score_;
};


// The class provides functionality for sampling topics
// for an given author.
class AuthorUtils {
public:
	// Sample the word topics for a given author,
  // Sampling can be with (permute = 1) or without (permute != 1)
  // permuting the words in the document.
  // Words can or cannot be removed from topics (set/unset
  // the bool remove variable).
  // alpha control default topic proportion.
  // eta smooth topic-word probability.
	static void SampleTopics(
			Author* author,
      int permute_words,
      bool remove,
      double eta,
      AllTopics* all_topics,
      bool inf=false);

	static void SampleTopic(
			Author* author,
			int word_idx,
			bool remove,
			double alpha,
			AllTopics* all_topics,
			bool inf=false);

	static void UpdateTopicFromWord(Author* author, 
																	 Word* word,
																	 int update,
																	 AllTopics* all_topics,
																	 bool inf=false);

	static void PermuteWords(Author* author);

	static double AlphaScore(Author* author, double alpha);

	static vector<double> TopicProportion(Author* author, double alpha);

	static void SaveAuthor(Author* author, ofstream& ofs);
};



// AllAuthors contains all the authors in corpus.
class AllAuthors {

public:
	static AllAuthors& GetInstance();

public:
	AllAuthors(const AllAuthors& from) = delete;
	AllAuthors& operator=(const AllAuthors& from) = delete;

	int getAuthors() const { return authors_.size(); }

	Author* getMutableAuthor(int author_id) { return &authors_[author_id]; }

	void addAuthor(int id, int depth) { authors_.emplace_back(Author(id, depth)); }

	void clearAllAuthors() { authors_.resize(0); }

private:
	// All authors.
	vector<Author> authors_;

	// Private constructor.
	AllAuthors() {}
};

class AllAuthorsUtils {
public:
	static double AlphaScores(double alpha);

	static void SaveAuthors(const string& filename_authors);

	static void LoadAuthors(const string& filename_authors);
};

}  // namespace atm
#endif // AUTHOR_H_