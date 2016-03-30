#include <iostream>

#include "gibbs.h"

using atm::GibbsSampler;
using atm::GibbsState;


int main(int argc, char** argv) {
  if (argc == 3) {
    // The random number generator seed.
    // For testing an example seed is: t = 1147530551;
    long rng_seed = 458312327;
    (void) time(&rng_seed);

    std::string filename_corpus = argv[1];
    std::string filename_authors = argv[2];
    std::string filename_topics_count = "result/train-topics-counts-final.dat";
    string filename_other = "result/train.other";
    string filename_author_counts = "result/train-author-counts-final.dat";
    string doc_no = argv[4];
    
    GibbsSampler::InferATM(filename_corpus, filename_authors,
                              filename_topics_count, filename_other,
                              filename_author_counts, rng_seed);
  } else {
    cout << "Arguments: "
        "(1) corpus filename "
        "(2) author filename "
        "(3) settings filename " 
        "(4) part of train doc number" << endl;
  }
  return 0;
}

