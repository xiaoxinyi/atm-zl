#include <assert.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_sf.h>
#include <math.h>

#include "document.h"
#include "utils.h"
#include "author.h"
#include "topic.h"


namespace atm {

// =======================================================================
// Word
// =======================================================================

Word::Word(int id, int author_id, int topic_id)
		: id_(id),
		  author_id_(author_id),
		  topic_id_(topic_id) {
}

Word::Word(int id) 
		: id_(id),
		  author_id_(-1),
		  topic_id_(-1) {

}

bool Word::operator==(const Word& word) {
	return (id_ == word.id_) and
				 (author_id_ == word.author_id_) and
				 (topic_id_ == word.topic_id_);
}

// =======================================================================
// WordUtils
// =======================================================================

void WordUtils::UpdateAuthorFromWord(
			int word_idx,
			int update,
			AllTopics* all_topics) {
	AllWords& all_words = AllWords::GetInstance();
	Word* word = all_words.getMutableWord(word_idx);

	if (word->getAuthorId() == -1 && update == -1) {
			return;
	}

	AllAuthors& all_authors = AllAuthors::GetInstance();
	Author* author = all_authors.getMutableAuthor(word->getAuthorId());
	if (update == -1) {	
		int topic_id = word->getTopicId();
		if (topic_id != -1) {
			// Update topic_id count.
			author->updateTopicCounts(topic_id, update);	

			// Update topic statistics.
			Topic* topic = all_topics->getMutableTopic(topic_id);
			topic->updateWordCount(word->getId(), update);
		}
		
		// Remove word from author.
		author->removeWord(word_idx);

		// Reset author id and topic_id.
		word->setAuthorId(-1);
		word->setTopicId(-1);
		return;
	}

	if (update == 1) {
		author->addWord(word_idx);
		word->setTopicId(-1);
	}
}

// =======================================================================
// AllWords
// =======================================================================

AllWords& AllWords::GetInstance() {
	static AllWords instance;
	return instance;
}


// =======================================================================
// Document
// =======================================================================
Document::Document(int id)
		: id_(id){
}



// =======================================================================
// DocumentUtils
// =======================================================================

void DocumentUtils::PermuteWords(Document* document) {
  int size = document->getWords();
  vector<int> permuted_words;

  // Permute the values in perm.
  // These values correspond to the indices of the words in the
  // word vector of the document.
  gsl_permutation* perm = gsl_permutation_calloc(size);
  Utils::Shuffle(perm, size);
  int perm_size = perm->size;
  assert(size == perm_size);

  for (int i = 0; i < perm_size; i++) {
  	int word = document->getWord(perm->data[i]);
    permuted_words.push_back(word);
  }

  document->setWords(move(permuted_words));

  gsl_permutation_free(perm);
}


void DocumentUtils::SampleAuthors(Document* document, 
																	AllTopics* all_topics) {
	int authors = document->getAuthors();
	std::vector<double> log_pr(authors, log(1.0 / authors));
	
	AllWords& all_words = AllWords::GetInstance();

	for (int i = 0; i < document->getWords(); i++) {
		int word_idx = document->getWord(i);
		Word* word = all_words.getMutableWord(word_idx);


		// Sample author id uniformly.
		int author_id = Utils::SampleFromLogPr(log_pr);
		if (author_id != word->getAuthorId()) {
			WordUtils::UpdateAuthorFromWord(word_idx, -1, all_topics);
			word->setAuthorId(author_id);
			WordUtils::UpdateAuthorFromWord(word_idx, 1, all_topics);	
		}
	}
}

}  // namespace atm