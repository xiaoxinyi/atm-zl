#ifndef CORPUS_H_
#define CORPUS_H_

#include <string>

#include "document.h"
#include "utils.h"
#include "topic.h"

namespace atm {

// A corpus containing a number of documents.
// Local dirichlet paramter of each author - alpha.
class Corpus {
 public:
  Corpus();

  void setWordNo(int word_no) { word_no_ = word_no; }
  int getWordNo() const { return word_no_; }

  void addDocument(Document&& document) {
    documents_.emplace_back(move(document));
  }
  int getDocuments() const { return documents_.size(); }
  Document* getMutableDocument(int i) { return &documents_.at(i); }
  void setDocuments(vector<Document>&& documents) {
    documents_ = move(documents);
  }

  int getWordTotal() const { return word_total_; }
  void setWordTotal(int word_total) { word_total_ = word_total; }

  int getAuthorNo() const { return author_no_; }
  void setAuthorNo(const int& author_no) { author_no_ = author_no; }

private:
  // The number of distinct words in the corpus.
  int word_no_;

  // The number of total words in the corpus.
  int word_total_;

  // The number of authors.
  int author_no_;

  // The documents in this corpus.
  vector<Document> documents_;

};


// This class provides functionality for reading a corpus from a file.
class CorpusUtils {
 public:
  // Read corpus from file.
  static void ReadCorpus(
      const string& filename,
      const string& authors_filename,
      Corpus* corpus,
      int topic_no);

  static void SaveTrainCorpus(const string& filename_corpus,
                              const string& filename_authors,
                              const string& filename_save,
                              const string& filename_authors_save,
                              Corpus* corpus,
                              int doc_no);


  // Permute the documents in the corpus.
  static void PermuteDocuments(Corpus* corpus);
};

} // atm

#endif  // CORPUS_H_