#include "ec_fft.hpp"

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace waffle {
namespace g1_fft {

using namespace barretenberg;

inline bool is_power_of_two(uint64_t x)
{
    return x && !(x & (x - 1));
}

inline uint32_t reverse_bits(uint32_t x, uint32_t bit_length)
{
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return (((x >> 16) | (x << 16))) >> (32 - bit_length);
}

inline void ec_fft_inner(g1::element* g1_elements, const size_t n, const std::vector<fr*>& root_table)
{
    is_power_of_two(n);
    ASSERT(!root_table.empty());

    // Exercise: implement the butterfly structure to perform ec-fft
}

void ec_fft(g1::element* g1_elements, const evaluation_domain& domain)
{
    ec_fft_inner(g1_elements, domain.size, domain.get_round_roots());
}

void ec_ifft(g1::element* g1_elements, const evaluation_domain& domain)
{
    ec_fft_inner(g1_elements, domain.size, domain.get_inverse_round_roots());
    for (size_t i = 0; i < domain.size; i++) {
        g1_elements[i] *= domain.domain_inverse;
    }
}

void convert_srs(g1::affine_element* monomial_srs, g1::affine_element* lagrange_srs, const evaluation_domain& domain)
{
    const size_t n = domain.size;
    is_power_of_two(n);

    // Exercise: implement the conversion of monomial to Lagrange SRS
    // Note that you can convert from g1::affine_element form to g1::element by just doing:
    // g1::affine_element x_affine = g1::affine_one;
    // auto x_elem = g1::element(x_affine);
    // Conversion from g1::element to g1::affine_element can be done similarly.
}

} // namespace g1_fft
} // namespace waffle