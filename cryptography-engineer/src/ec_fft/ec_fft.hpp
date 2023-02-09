#pragma once
#include <polynomials/evaluation_domain.hpp>
#include <ecc/curves/bn254/g1.hpp>
#include <numeric/bitop/get_msb.hpp>

namespace waffle {
namespace g1_fft {

using namespace barretenberg;

/**
 * Computes FFT (butterfly-structure) on EC (elliptic curve) points `g1_elements`.
 *
 * @param g1_elements: Given set of curve points on the BN254 curve
 * @param n: Number of curve points (assumed to be a power of two)
 * @param root_table: Contains roots of unity required in each round of the butterfly structure
 * @details:
 * root_table[0]: [1 ω₁]
 * root_table[1]: [1 ω₂ (ω₂)² (ω₂)³]
 * .
 * .
 * .
 * root_table[m]: [1 ωᵣ (ωᵣ)² (ωᵣ)³ ... (ωᵣ)ᴿ⁻² (ωᵣ)ᴿ⁻¹]
 *
 * where r = log2(n) - 1, R = 2^r and ωᵢ = i-th root of unity.
 */
void ec_fft_inner(g1::element* g1_elements, const size_t n, const std::vector<fr*>& root_table);

/**
 * Computes EC-FFT of `g1_elements` given the evaluation domain `domain`.
 *
 * @details:
 * The domain contains the following:
 * n = domain.size (number of `g1_elements`, assumed to be a power of two)
 * ω = domain.root (n-th root of unity)
 * 1/ω = domain.root_inverse (multiplicative inverse of ω)
 */
void ec_fft(g1::element* g1_elements, const evaluation_domain& domain);

/**
 * Computes inverse EC-FFT of `g1_elements` given the evaluation domain `domain`.
 */
void ec_ifft(g1::element* g1_elements, const evaluation_domain& domain);

/**
 * Using `ec_fft`, computes the Lagrange form of the SRS given the monomial form SRS `monomial_srs`.
 *
 * @param monomial_srs: Monomial SRS of the form: ([1]₁, [x]₁, [x²]₁, [x³]₁, ..., [xⁿ⁻¹]₁)
 * @param lagrange_srs: Result must be stored in this, it should be of the form: ([L₀(x)]₁, [L₁(x)]₁, ..., [Lⁿ⁻¹(x)]₁)
 * @param domain: contains the information about n-th roots of unity
 */
void convert_srs(g1::affine_element* monomial_srs, g1::affine_element* lagrange_srs, const evaluation_domain& domain);

} // namespace g1_fft
} // namespace waffle