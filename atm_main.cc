#include <iostream>

#include "gibbs.h"

using atm::GibbsSampler;
using atm::GibbsState;
using atm::AllTopicsUtils;

static const string FILENAME_TOPICS = "topics_final.dat";

#define MAX_ITERATIONS 10000

int main(int argc, char** argv) {
  if (argc == 4) {
    // The random number generator seed.
    // For testing an example seed is: t = 1147530551;
    long rng_seed = 458312327;
    (void) time(&rng_seed);

    std::string filename_corpus = argv[1];
    std::string filename_authors = argv[2];
    std::string filename_settings = argv[3];
    atm::GibbsState* gibbs_state = atm::GibbsSampler::InitGibbsStateRep(
        filename_corpus, filename_authors,
        filename_settings, rng_seed);

    for (int i = 0; i < MAX_ITERATIONS; i++) {
      atm::GibbsSampler::IterateGibbsState(gibbs_state);
    }

    AllTopicsUtils::SaveTopics(gibbs_state->getMutableAllTopics(),
    													 FILENAME_TOPICS);
    delete gibbs_state;
  } else {
    cout << "Arguments: "
        "(1) corpus filename "
        "(2) author filename "
        "(3) settings filename" << endl;
  }
  return 0;
}

