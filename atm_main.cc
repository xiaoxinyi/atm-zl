#include <iostream>

#include "gibbs.h"

using atm::GibbsSampler;
using atm::GibbsState;
using atm::AllTopicsUtils;


#define MAX_ITERATIONS 10000

int main(int argc, char** argv) {
  if (argc == 5) {
    // The random number generator seed.
    // For testing an example seed is: t = 1147530551;
    long rng_seed = 458312327;
    (void) time(&rng_seed);

    std::string filename_corpus = argv[1];
    std::string filename_authors = argv[2];
    std::string filename_settings = argv[3];
    string doc_no = argv[4];
    
    GibbsSampler::TrainByPart(filename_corpus, filename_authors,
                              filename_settings, rng_seed, atoi(doc_no.c_str()));
  } else {
    cout << "Arguments: "
        "(1) corpus filename "
        "(2) author filename "
        "(3) settings filename " 
        "(4) part of train doc number" << endl;
  }
  return 0;
}

