#pragma once
#include <polynomials/polynomial.hpp>
#include "kzg_proof.hpp"

namespace waffle {

using namespace barretenberg;

class FeistKhovratovichTrick {
  public:
    FeistKhovratovichTrick();

  private:
    size_t n;
    barretenberg::polynomial source_poly;
    std::vector<waffle::KZGProof> kzg_opening_proofs;
};

} // namespace waffle