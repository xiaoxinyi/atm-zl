#include <assert.h>
#include <math.h>
#include <algorithm>
#include <gsl/gsl_sf.h>

#include "utils.h"
#include "author.h"
#include "topic.h"

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
																			 int word_idx, 
																			 int update,
																			 AllTopics* all_topics) {
	AllWords& all_words = AllWords::GetInstance();
	Word* word = all_words.getMutableWord(word_idx);
	int topic_id = word->getTopicId();
	if (topic_id == -1) {
		return;
	}

	author->updateTopicCounts(topic_id, update);

	Topic* topic = all_topics->getMutableTopic(topic_id);
	topic->updateWordCount(word->getId(), update);

}
void AuthorUtils::SampleTopic(
			Author* author,
			int word_idx,
      bool remove,
      double alpha,
      double eta,
      AllTopics* all_topics) {
	if (remove) {
		UpdateTopicFromWord()
	}


}

void AuthorUtils::SampleTopics(
			Author* author,
      int permute_words,
      bool remove,
      double alpha,
      double eta) {
	int depth = author->getMutablePathTopic(0)->getMutableTree()->getDepth();
	vector<double> log_pr(depth);

	// Permute the words in the author.
	if (permute_words == 1) {
		PermuteWords(author);
	}

	AllWords& all_words = AllWords::GetInstance();

	for (int i = 0; i < author->getWords(); i++) {
		int word_idx = author->getWord(i);
		Word* word = all_words.getMutableWord(word_idx);

		if (remove) {
			int level = word->getLevel();
			if (level != -1) {
				// Update the word level.
				author->updateLevelCounts(level, -1);
			
				// Decrease the word count.
				author->getMutablePathTopic(level)->updateWordCount(word->getId(), -1);
			}
		}

		// Compute probabilities.
		// Compute log prbabilities for all levels.
		// Use the corpus GEM mean and scale.
		author->computeLogPrLevel(gem_mean, gem_scale, depth);

		for (int j = 0; j < depth; j++) {
			double log_pr_level = author->getLogPrLevel(j);
			double log_pr_word = 
					author->getMutablePathTopic(j)->getLogPrWord(word->getId());

			double log_value = log_pr_level + log_pr_word;
			// Keep for each level the log probability of the word +
      // log probability of the level.
      // Use these values to sample the new level.
      log_pr.at(j) = log_value;
		}

		// Sample the new level and update.
    int new_level = Utils::SampleFromLogPr(log_pr);
    author->getMutablePathTopic(new_level)->updateWordCount(word->getId(), 1);
    word->setLevel(new_level);
    author->updateLevelCounts(new_level, 1);
 	}
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
}

}  // namespace atm