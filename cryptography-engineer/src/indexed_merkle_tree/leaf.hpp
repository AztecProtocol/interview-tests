#pragma once
#include <stdlib/primitives/field/field.hpp>
#include <crypto/pedersen/pedersen.hpp>

namespace plonk {
namespace stdlib {
namespace indexed_merkle_tree {

using namespace barretenberg;
typedef uint256_t index_t;

struct leaf {
    fr value;
    index_t nextIndex;
    fr nextValue;

    bool operator==(leaf const&) const = default;

    std::ostream& operator<<(std::ostream& os)
    {
        os << "value = " << value << "\nnextIdx = " << nextIndex << "\nnextVal = " << nextValue;
        return os;
    }

    void read(uint8_t const*& it)
    {
        using serialize::read;
        read(it, value);
        read(it, nextIndex);
        read(it, nextValue);
    }

    inline void write(std::vector<uint8_t>& buf)
    {
        using serialize::write;
        write(buf, value);
        write(buf, nextIndex);
        write(buf, nextValue);
    }

    barretenberg::fr hash() const { return crypto::pedersen::compress_native({ value, nextIndex, nextValue }); }
};

inline barretenberg::fr compress_pair(barretenberg::fr const& lhs, barretenberg::fr const& rhs)
{
    return crypto::pedersen::compress_native({ lhs, rhs });
}

} // namespace indexed_merkle_tree
} // namespace stdlib
} // namespace plonk