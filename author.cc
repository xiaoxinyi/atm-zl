#include <assert.h>
#include <math.h>
#include <algorithm>
#include <gsl/gsl_sf.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "utils.h"
#include "author.h"
#include "topic.h"

const int BUF_SIZE = 1000;

namespace atm {

// =======================================================================
// Author
// =======================================================================

Author::Author() {	
}

Author::Author(int id, int topic_no) 
		: id_(id),
		  topic_no_(topic_no),
		  topic_counts_(topic_no, 0) {

}

void Author::removeWord(int word) {
	auto found = find(begin(words_), end(words_), word);
	if (found == end(words_)) {

	} else {
		words_.erase(found);
	}
}

int Author::getSumTopicCounts(int topic_no) const {
	int sum = 0;
	for (int i = 0; i < topic_no; i++) {
		sum += topic_counts_[i];
	}
	return sum;
}




// =======================================================================
// AllAuthors
// =======================================================================

AllAuthors& AllAuthors::GetInstance() {
	static AllAuthors instance;
	return instance;
}



// =======================================================================
// AuthorUtils
// =======================================================================

void AuthorUtils::PermuteWords(Author* author) {
  int size = author->getWords();
	if (size == 0) return;
  vector<int> permuted_words;

  // Permute the values in perm.
  // These values correspond to the indices of the words in the
  // word vector of the author.
  gsl_permutation* perm = gsl_permutation_calloc(size);
  Utils::Shuffle(perm, size);
  int perm_size = perm->size;
  assert(size == perm_size);

  for (int i = 0; i < perm_size; i++) {
    permuted_words.push_back(author->getWord(perm->data[i]));
  }

  author->setWords(move(permuted_words));

  gsl_permutation_free(perm);
}


void AuthorUtils::UpdateTopicFromWord(Author* author,
																			 Word* word,
																			 int update,
																			 AllTopics* all_topics,
																			 bool inf) {
	int topic_id = word->getTopicId();
	if (topic_id == -1) {
		return;
	}

	author->updateTopicCounts(topic_id, update);
	if (not inf) {
		Topic* topic = all_topics->getMutableTopic(topic_id);
		topic->updateWordCount(word->getId(), update);
	}

}

void AuthorUtils::SampleTopic(
			Author* author,
			int word_idx,
      bool remove,
      double alpha,
      AllTopics* all_topics,
      bool inf) {

	AllWords& all_words = AllWords::GetInstance();
	Word* word = all_words.getMutableWord(word_idx);
	if (remove) {
		UpdateTopicFromWord(author, word, -1, all_topics, inf);
	}

	int topics = all_topics->getTopics();
	vector<double> log_pr(topics, 0.0);
	for (int i = 0; i < topics; i++) {
		Topic* topic = all_topics->getMutableTopic(i);
		int topic_count = author->getTopicCounts(i);
		log_pr[i] = log(topic_count + alpha) + topic->getLogPrWord(word->getId());
	}

	int sample_topic_id = Utils::SampleFromLogPr(log_pr);

	word->setTopicId(sample_topic_id);
	UpdateTopicFromWord(author, word, 1, all_topics, inf);
}

void AuthorUtils::SampleTopics(
			Author* author,
      int permute_words,
      bool remove,
      double alpha,
      AllTopics* all_topics,
      bool inf) {

	int author_word_count = author->getWords();
	if (author_word_count == 0) return;
	assert(author_word_count > 0);

	// Permute the words in the author.
	if (permute_words == 1) {
		PermuteWords(author);
	}
	
	for (int i = 0; i < author_word_count; i++) {
		int word_idx = author->getWord(i);
		SampleTopic(author, word_idx, remove, alpha, all_topics, inf);
	}
}

double AuthorUtils::AlphaScore(Author* author, double alpha) {
	double score = 0.0;
	double lgam_alpha = gsl_sf_lngamma(alpha);
	int word_count = author->getWords();
	int topic_no = author->getTopicNo();

	score += gsl_sf_lngamma(topic_no * alpha);
	for (int i = 0; i < topic_no; i++) {
		int topic_count = author->getTopicCounts(i);
		if (topic_count > 0) {
			score += gsl_sf_lngamma(topic_count + alpha) - lgam_alpha;
		}
	}

	score -= gsl_sf_lngamma(word_count + topic_no * alpha);

	return score;
}

vector<double> AuthorUtils::TopicProportion(Author* author, 
																						double alpha) {

	int topic_no = author->getTopicNo();
	vector<double> log_pr(topic_no, 0.0);
	int sum_topic_count = author->getSumTopicCounts(topic_no);

	for (int i = 0; i < topic_no; i++) {
		log_pr[i] = log(author->getTopicCounts(i) + alpha) - 
								log(sum_topic_count + topic_no * alpha);
	}

	return log_pr;
}

void AuthorUtils::SaveAuthor(Author* author, ofstream& ofs) {
	int topic_no = author->getTopicNo();
	for (int i = 0; i < topic_no; i++) {
		int topic_count = author->getTopicCounts(i);
		ofs << topic_count << " ";
	}
	ofs << endl;
}

// =======================================================================
// AllAuthorsUtils
// =======================================================================

double AllAuthorsUtils::AlphaScores(double alpha) {
	double score = 0.0;
	AllAuthors& all_authors = AllAuthors::GetInstance();
	int authors = all_authors.getAuthors();
	for (int i = 0; i < authors; i++) {
		Author* author = all_authors.getMutableAuthor(i);
		score += AuthorUtils::AlphaScore(author, alpha);
	}	
	return score;
}

void AllAuthorsUtils::SaveAuthors(const string& filename_authors) {
	AllAuthors& all_authors = AllAuthors::GetInstance();
	int author_no = all_authors.getAuthors();
	ofstream ofs(filename_authors);

	for (int i = 0; i < author_no; i++) {
		Author* author = all_authors.getMutableAuthor(i);
		AuthorUtils::SaveAuthor(author, ofs);
	}
	ofs.close();
}

void AllAuthorsUtils::LoadAuthors(const string& filename_authors) {
	ifstream ifs(filename_authors);
	AllAuthors& all_authors = AllAuthors::GetInstance();
	int topic_no = all_authors.getMutableAuthor(0)->getTopicNo();
	int authors = all_authors.getAuthors();

	char buf[BUF_SIZE];
	for (int i = 0; i < authors; i++) {
		ifs.getline(buf, BUF_SIZE);
		istringstream iss(buf);

		Author* author = all_authors.getMutableAuthor(i);
		for (int j = 0; j < topic_no; j++) {
			string str;
			getline(iss, str, ' ');
			author->setTopicCounts(j, atoi(str.c_str()));
		}	
	}
	ifs.close();	
}

}  // namespace atm