#pragma once
#include <ecc/curves/bn254/g1.hpp>

namespace waffle {

struct KZGProof {
    barretenberg::g1::affine_element source_poly_commitment;
    barretenberg::g1::affine_element opening_poly_commitment;
    barretenberg::fr source_poly_evaluation;

    bool operator==(KZGProof const&) const = default;

    std::ostream& operator<<(std::ostream& os)
    {
        os << "source_poly_commitment = " << source_poly_commitment
           << "\nopening_poly_commitment = " << opening_poly_commitment
           << "\nnsource_poly_evaluation = " << source_poly_evaluation;
        return os;
    }

    void read(uint8_t const*& it)
    {
        using serialize::read;
        read(it, source_poly_commitment);
        read(it, opening_poly_commitment);
        read(it, source_poly_evaluation);
    }

    inline void write(std::vector<uint8_t>& buf)
    {
        using serialize::write;
        write(buf, source_poly_commitment);
        write(buf, opening_poly_commitment);
        write(buf, source_poly_evaluation);
    }
};
} // namespace waffle