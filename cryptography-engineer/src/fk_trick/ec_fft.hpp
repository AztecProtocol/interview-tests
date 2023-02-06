#pragma once
#include <polynomials/evaluation_domain.hpp>
#include <ecc/curves/bn254/g1.hpp>
#include <numeric/bitop/get_msb.hpp>

namespace waffle {
namespace g1_fft {

using namespace barretenberg;

void ec_fft_inner(g1::element* g1_elements, const size_t n, const std::vector<fr*>& root_table);

void ec_fft(g1::element* g1_elements, const evaluation_domain& domain);

void ec_ifft(g1::element* g1_elements, const evaluation_domain& domain);

} // namespace g1_fft
} // namespace waffle