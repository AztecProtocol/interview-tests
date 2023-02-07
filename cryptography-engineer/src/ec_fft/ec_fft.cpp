#include "ec_fft.hpp"

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

    g1::element temp;
    fr temp_scalar;
    size_t log2_size = (size_t)numeric::get_msb(n);

    // efficiently separate odd and even indices - (An introduction to algorithms, section 30.3)
    for (size_t i = 0; i <= n; ++i) {
        uint32_t swap_index = (uint32_t)reverse_bits((uint32_t)i, (uint32_t)log2_size);
        if (i < swap_index) {
            temp = g1_elements[swap_index];
            g1_elements[swap_index] = g1_elements[i];
            g1_elements[i] = temp;
        }
    }

    // perform first butterfly iteration explicitly: x0 = x0 + x1, x1 = x0 - x1
    for (size_t k = 0; k < n; k += 2) {
        temp = g1_elements[k + 1];
        g1_elements[k + 1] = g1_elements[k] - g1_elements[k + 1];
        g1_elements[k] += temp;
    }

    for (size_t m = 2; m < n; m *= 2) {
        const size_t i = (size_t)numeric::get_msb(m);
        for (size_t k = 0; k < n; k += (2 * m)) {
            for (size_t j = 0; j < m; ++j) {
                const size_t even_elem_idx = (k + j) & (n - 1);
                const size_t odd_elem_idx = (k + j + m) & (n - 1);

                temp = g1_elements[odd_elem_idx] * root_table[i - 1][j];
                g1_elements[odd_elem_idx] = g1_elements[even_elem_idx] - temp;
                g1_elements[even_elem_idx] += temp;
            }
        }
    }
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
    std::vector<g1::element> monomial_srs_jac;
    for (size_t i = 0; i < n; i++) {
        monomial_srs_jac.push_back(monomial_srs[i]);
    }

    // Compute [ω⁰], [ω¹], [ω²], ..., [ωⁿ⁻¹] using ec-fft
    ec_fft(&monomial_srs_jac[0], domain);

    // Compute Lagrange srs from the result of the ec-fft
    const size_t n_by_2 = (n >> 1);
    for (size_t i = 0; i < n; i++) {
        if (i == 0 || i == n_by_2) {
            lagrange_srs[i] = g1::affine_element(monomial_srs_jac[i] * domain.domain_inverse);
        } else {
            lagrange_srs[i] = g1::affine_element(monomial_srs_jac[n - i] * domain.domain_inverse);
        }
    }
}

} // namespace g1_fft
} // namespace waffle