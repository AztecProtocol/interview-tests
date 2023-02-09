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

TEST(ec_fft, test_convert_srs)
{
    constexpr size_t n = 512;
    std::vector<g1::element> monomial_points;
    std::vector<g1::element> lagrange_points;
    std::vector<fr> poly_monomial;
    std::vector<fr> poly_lagrange;

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
        multiplicand *= x;
    }

    auto domain = evaluation_domain(n);
    domain.compute_lookup_table();

    std::vector<g1::affine_element> lagrange_srs;
    std::vector<g1::affine_element> monomial_srs;
    lagrange_srs.resize(2 * n);
    monomial_srs.resize(2 * n);

    // Copy over the monomial points in monomial_srs
    for (size_t i = 0; i < n; i++) {
        monomial_srs[i] = g1::affine_element(monomial_points[i]);
    }

    // Convert from monomial to lagrange srs
    waffle::g1_fft::convert_srs(&monomial_srs[0], &lagrange_srs[0], domain);
    polynomial_arithmetic::fft(&poly_lagrange[0], domain);

    scalar_multiplication::pippenger_runtime_state state(n);
    scalar_multiplication::generate_pippenger_point_table(&monomial_srs[0], &monomial_srs[0], n);
    scalar_multiplication::generate_pippenger_point_table(&lagrange_srs[0], &lagrange_srs[0], n);

    // Check <monomial_form , monomial_srs> == <lagrange_form , lagrange_srs>
    g1::element expected = scalar_multiplication::pippenger(&poly_monomial[0], &monomial_srs[0], n, state);
    g1::element result = scalar_multiplication::pippenger(&poly_lagrange[0], &lagrange_srs[0], n, state);
    expected = expected.normalize();
    result = result.normalize();

    EXPECT_EQ(result, expected);
}