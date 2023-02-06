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