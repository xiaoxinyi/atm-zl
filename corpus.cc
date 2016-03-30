
#include <assert.h>
#include <gsl/gsl_permutation.h>
#include <math.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <set>

#include "corpus.h"
#include "author.h"
#include "document.h"

#define BUF_SIZE 10000

namespace atm {

// =======================================================================
// Corpus
// =======================================================================

Corpus::Corpus()
    : word_no_(0),
      word_total_(0),
      author_no_(0) {
}

// =======================================================================
// CorpusUtils
// =======================================================================

void CorpusUtils::ReadCorpus(
    const string& docs_filename,
    const string& authors_filename,
    Corpus* corpus,
    int topic_no) {

  ifstream infile(docs_filename.c_str());
  char buf[BUF_SIZE];

  ifstream authors_infile(authors_filename.c_str());
  char authors_buf[BUF_SIZE];

  int author_no = 0;
  int doc_no = 0;
  int word_no = 0;
  int total_word_count = 0;
  int words;

  AllWords& all_words = AllWords::GetInstance();

  while (infile.getline(buf, BUF_SIZE) && 
  			 authors_infile.getline(authors_buf, BUF_SIZE)) {
  	
  	istringstream s_line_author(authors_buf);
    std::vector<int> author_ids;
  	while (s_line_author.getline(authors_buf, BUF_SIZE, ' ')) {
  		int author_id = atoi(authors_buf);
  		if (author_id >= author_no) {
  			author_no = author_id + 1;
  		}
  		author_ids.push_back(author_id);
  	}

  	if (author_ids.empty()) {
  		continue;
  	}

    istringstream s_line(buf);
    // Consider each line at a time.
    int word_count_pos = 0;
    Document document(doc_no);
    // Set author ids
    document.setAuthorIds(author_ids);
    while (s_line.getline(buf, BUF_SIZE, ' ')) {
      if (word_count_pos == 0) {
        words = atoi(buf);
      } else {
        int word_id, word_count;
        istringstream s_word_count(buf);
        string str;
        getline(s_word_count, str, ':');
        word_id = atoi(str.c_str());
        getline(s_word_count, str, ':');
        word_count = atoi(str.c_str());
        total_word_count += word_count;
       
        for (int i = 0; i < word_count; i++) {
          all_words.addWord(word_id);
          document.addWord(all_words.getWordNo() - 1);
        }
       
        if (word_id >= word_no) {
          word_no = word_id + 1;
        }
      }
      word_count_pos++;
    }
    corpus->addDocument(move(document));
    doc_no += 1;
  }

  infile.close();
  authors_infile.close();

  AllAuthors& all_authors = AllAuthors::GetInstance();
  all_authors.clearAllAuthors();
  
  for (int i = 0; i < author_no; i++) {
    all_authors.addAuthor(i, topic_no);
  }

  corpus->setWordNo(word_no);
  corpus->setWordTotal(total_word_count);
  corpus->setAuthorNo(author_no);

  cout << "Number of documents in corpus: " << doc_no << endl;
  cout << "Number of authors in corpus: " << author_no << endl;
  cout << "Number of distinct words in corpus: " << word_no << endl;
  cout << "Number of words in corpus: " << total_word_count << " = " 
       << all_words.getWordNo() << endl;
}

void CorpusUtils::SaveTrainCorpus(const string& filename_corpus,
                              const string& filename_authors,
                              const string& filename_save,
                              const string& filename_authors_save,
                              Corpus* corpus,
                              int doc_no) {
  ifstream infile(filename_corpus.c_str());
  char buf[BUF_SIZE];

  ifstream authors_infile(filename_authors.c_str());
  char authors_buf[BUF_SIZE];

  ofstream ofs_corpus(filename_save);
  ofstream ofs_authors(filename_authors_save);

  set<int> s;
  for (int i = 0; i < doc_no; i++) {
    s.insert(corpus->getMutableDocument(i)->getId());
  }

  int cur_doc_no = 0;

  while (infile.getline(buf, BUF_SIZE) && 
         authors_infile.getline(authors_buf, BUF_SIZE)) {
    auto it = s.find(cur_doc_no++);
    if (it != end(s)) {
      ofs_corpus << buf << endl;
      ofs_authors << authors_buf << endl;
    }
  }

  infile.close();
  authors_infile.close();

  ofs_corpus.close();
  ofs_authors.close();

}

void CorpusUtils::PermuteDocuments(Corpus* corpus) {
  int size = corpus->getDocuments();
  vector<Document> permuted_documents;

  // Permute the values in perm.
  // These values correspond to the indices of the documents in the
  // document vector of the corpus.
  gsl_permutation* perm = gsl_permutation_calloc(size);
  Utils::Shuffle(perm, size);
  int perm_size = perm->size;
  assert(size == perm_size);

  for (int i = 0; i < perm_size; i++) {
    Document* document = corpus->getMutableDocument(perm->data[i]);
    permuted_documents.emplace_back(move(*document));
  }

  corpus->setDocuments(move(permuted_documents));

  gsl_permutation_free(perm);
}

double CorpusUtils::ComputePerplexity(Corpus* corpus, 
                                      AllTopics* all_topics,
                                      double alpha) {
  int doc_no = corpus->getDocuments();
  double perplexity = 0.0;
  int total_words = 0;
  for (int i = 0; i < doc_no; i++) {
    Document* document = corpus->getMutableDocument(i);
    perplexity +=  DocumentUtils::ComputePerplexity(document, all_topics, alpha);
    total_words += document->getWords();
  }

  return exp(-perplexity / total_words);
}

}  // namespace atm

