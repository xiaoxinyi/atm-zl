#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <string>
#include <vector>

using namespace std;

namespace atm {

// A word having an id, a word count, author id,
// and the topic_id the word is assigned to.
class Word {
public:
	Word(int id, int author_id, int topic_id);
	Word(int id);

	bool operator==(const Word& word);

	void setId(int id) { id_ = id; }
	int getId() const { return id_; }

	void setAuthorId(int author_id) { author_id_ = author_id; }
	int getAuthorId() const { return author_id_; }

	void setTopicId(int topic_id) { topic_id_ = topic_id; }
	int getTopicId() const { return topic_id_; }

private:
	// Word id.
	int id_;

	// Author id the word is assigned to.
	int author_id_;

	// Corresonding topic id.
	int topic_id_;
};

class AllTopics;

class WordUtils {
public:
	static void UpdateAuthorFromWord(
			int word_idx,
			int update,
			AllTopics* all_topics,
			bool inf = false);
};

// AllWords contains all the words in the corpus,
// each word has unique index in the corpus.
class AllWords {
public:
	static AllWords& GetInstance();
public:
	AllWords(const AllWords& from) = delete;
	AllWords& operator=(const AllWords& from) = delete;

	int getWordNo() const { return word_no_; }
	void setWordNo(const int& word_no) { word_no_ = word_no; }
	void updateWordNo(int update) { word_no_ += update; }

	void addWord(int word_id, int author_id = -1, int topic_id = -1) {
		words_.emplace_back(Word(word_id, author_id, topic_id));
		++word_no_;
	}

	Word* getMutableWord(int i) { return &words_[i]; }

private:
	// Number of words.
	int word_no_;

	// All the words.
	vector<Word> words_;

	AllWords() {}
};

// The document containing a number of words and authors.
// A document has an id.
class Document {
public:
	Document(int id);
	Document(const Document& from) = default;
	Document& operator=(const Document& from) = default;

	Document(Document&& from) = default;
	Document& operator=(Document&& from) = default;

	int getWords() const { return words_.size(); }
	int getAuthors() const { return author_ids_.size(); }

	void addWord(int word) { words_.push_back(word); }
	int getWord(int i) { return words_.at(i); }
	void setWords(vector<int>&& words) { words_ = move(words); }

	int getAuthorId(int i) const { return author_ids_.at(i); }
	void addAuthorId(const int author_id) { author_ids_.push_back(author_id); } 

	void setAuthorIds(const std::vector<int>& author_ids) { author_ids_ = author_ids; }
private:
	// Document id.
	int id_;

	// The words in the documnet
	vector<int> words_;

	// Author ids of the document.
	vector<int> author_ids_;
};

// The class provides functionality for permuting words
// and author ids.
class DocumentUtils {
public:
	// Permute the words in a document.
	static void PermuteWords(Document* document);

	// Sample author id
	static void SampleAuthors(Document* document, 
														AllTopics* all_topics,
														bool inf=false);

};

}  // namespace atm

#endif