#include "ec_fft.hpp"
#include <gtest/gtest.h>

#include <ecc/curves/bn254/g1.hpp>
#include <ecc/curves/bn254/g2.hpp>
#include <ecc/curves/bn254/fq12.hpp>
#include <ecc/curves/bn254/pairing.hpp>
#include <ecc/curves/bn254/scalar_multiplication/pippenger.hpp>
#include <ecc/curves/bn254/scalar_multiplication/scalar_multiplication.hpp>
#include <polynomials/polynomial_arithmetic.hpp>

using namespace barretenberg;

TEST(ec_fft, test_fft_ifft)
{
    constexpr size_t n = 256;
    std::vector<g1::element> monomial_points;
    std::vector<g1::element> lagrange_points;

    for (size_t i = 0; i < n; i++) {
        fr multiplicand = fr::random_element();
        g1::element monomial_term = g1::one * multiplicand;
        monomial_points.push_back(monomial_term);
        lagrange_points.push_back(monomial_term);
    }

    auto domain = evaluation_domain(n);
    domain.compute_lookup_table();

    // Do EC-FFT and then EC-iFFT
    waffle::g1_fft::ec_fft(&lagrange_points[0], domain);
    waffle::g1_fft::ec_ifft(&lagrange_points[0], domain);

    // Compare the results
    for (size_t i = 0; i < n; i++) {
        EXPECT_EQ(monomial_points[i].normalize(), lagrange_points[i].normalize());
    }
}

TEST(ec_fft, test_compare_ffts)
{
    constexpr size_t n = 256;
    std::vector<g1::element> monomial_points;
    std::vector<g1::element> lagrange_points;
    std::vector<fr> poly_monomial;
    std::vector<fr> poly_lagrange;

    for (size_t i = 0; i < n; i++) {
        fr multiplicand = fr::random_element();
        poly_monomial.push_back(multiplicand);
        poly_lagrange.push_back(multiplicand);

        g1::element monomial_term = g1::one * multiplicand;
        monomial_points.push_back(monomial_term);
        lagrange_points.push_back(monomial_term);
    }

    auto domain = evaluation_domain(n);
    domain.compute_lookup_table();

    // Do EC FFT
    waffle::g1_fft::ec_fft(&lagrange_points[0], domain);

    // Do fr FFT
    polynomial_arithmetic::fft(&poly_lagrange[0], domain);

    // Compare the results
    for (size_t i = 0; i < n; i++) {
        fr scalar = poly_lagrange[i];
        g1::element expected = g1::one * scalar;
        EXPECT_EQ(expected.normalize(), lagrange_points[i].normalize());
    }
}

TEST(ec_fft, test_compare_iffts)
{
    constexpr size_t n = 256;
    std::vector<g1::element> monomial_points;
    std::vector<g1::element> lagrange_points;
    std::vector<fr> poly_monomial;
    std::vector<fr> poly_lagrange;

    for (size_t i = 0; i < n; i++) {
        fr multiplicand = fr::random_element();
        poly_monomial.push_back(multiplicand);
        poly_lagrange.push_back(multiplicand);

        g1::element monomial_term = g1::one * multiplicand;
        monomial_points.push_back(monomial_term);
        lagrange_points.push_back(monomial_term);
    }

    auto domain = evaluation_domain(n);
    domain.compute_lookup_table();

    // Do EC iFFT
    waffle::g1_fft::ec_ifft(&monomial_points[0], domain);

    // Do fr iFFT
    polynomial_arithmetic::ifft(&poly_monomial[0], domain);

    // Compare the results
    for (size_t i = 0; i < n; i++) {
        fr scalar = poly_monomial[i];
        g1::element expected = g1::one * scalar;
        EXPECT_EQ(expected.normalize(), monomial_points[i].normalize());
    }
}

TEST(ec_fft, test_srs)
{
    constexpr size_t n = 256;
    std::vector<g1::element> monomial_points;
    std::vector<g1::element> lagrange_points;
    std::vector<fr> poly_monomial;
    std::vector<fr> poly_lagrange;
    std::vector<fr> srs_poly_monomial;
    std::vector<fr> srs_poly_lagrange;

    const fr x = fr::random_element();
    fr multiplicand = 1;
    for (size_t i = 0; i < n; i++) {
        // Fill the polynomials with random coefficients
        fr coefficient = fr::random_element();
        poly_monomial.push_back(coefficient);
        poly_lagrange.push_back(coefficient);

        // Create a fake srs with secret x
        g1::element monomial_term = g1::one * multiplicand;
        monomial_points.push_back(monomial_term);
        lagrange_points.push_back(monomial_term);
        srs_poly_monomial.push_back(multiplicand);
        srs_poly_lagrange.push_back(multiplicand);
        multiplicand *= x;
    }

    auto domain = evaluation_domain(n);
    domain.compute_lookup_table();

    // Do EC FFT to get Lagrange SRS (shifting yet to be done)
    waffle::g1_fft::ec_fft(&lagrange_points[0], domain);
    polynomial_arithmetic::fft(&poly_lagrange[0], domain);
    polynomial_arithmetic::fft(&srs_poly_lagrange[0], domain);

    // Check evals of SRS
    multiplicand = 1;
    for (size_t i = 0; i < n; i++) {
        fr scalar = srs_poly_lagrange[i];
        g1::element expected = g1::one * scalar;
        EXPECT_EQ(expected.normalize(), lagrange_points[i].normalize());

        g1::element expected_monomial = g1::one * multiplicand;
        EXPECT_EQ(expected_monomial.normalize(), monomial_points[i].normalize());
        multiplicand *= x;
    }

    // Check scalar eval of polynomial
    fr monomial_sum = 0;
    fr lagrange_sum = 0;
    size_t n_by_2 = (n >> 1);
    for (size_t i = 0; i < n; i++) {
        monomial_sum += poly_monomial[i] * srs_poly_monomial[i];
        if (i == 0 || i == n_by_2) {
            lagrange_sum += poly_lagrange[i] * srs_poly_lagrange[i] * domain.domain_inverse;
        } else {
            lagrange_sum += poly_lagrange[i] * srs_poly_lagrange[n - i] * domain.domain_inverse;
        }
    }
    EXPECT_EQ(monomial_sum, lagrange_sum);

    g1::affine_element lagrange_srs[2 * n];
    g1::affine_element monomial_srs[2 * n];
    for (size_t i = 0; i < n; i++) {
        monomial_srs[i] = g1::affine_element(monomial_points[i]);
        if (i == 0 || i == n_by_2) {
            lagrange_srs[i] = g1::affine_element(lagrange_points[i] * domain.domain_inverse);
        } else {
            lagrange_srs[i] = g1::affine_element(lagrange_points[n - i] * domain.domain_inverse);
        }
    }

    scalar_multiplication::pippenger_runtime_state state(n);
    scalar_multiplication::generate_pippenger_point_table(&monomial_srs[0], &monomial_srs[0], n);
    scalar_multiplication::generate_pippenger_point_table(&lagrange_srs[0], &lagrange_srs[0], n);

    g1::element expected = scalar_multiplication::pippenger(&poly_monomial[0], &monomial_srs[0], n, state);
    g1::element result = scalar_multiplication::pippenger(&poly_lagrange[0], &lagrange_srs[0], n, state);
    expected = expected.normalize();
    result = result.normalize();

    EXPECT_EQ(result, expected);
}