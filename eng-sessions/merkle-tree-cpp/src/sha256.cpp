#include "sha256.hpp"
#include <cstring>
#include <iomanip>
#include <sstream>

constexpr std::array<uint32_t, 64> SHA256::K;

SHA256::SHA256()
    : blocklen(0)
    , bitlen(0)
{
    state[0] = 0x6a09e667;
    state[1] = 0xbb67ae85;
    state[2] = 0x3c6ef372;
    state[3] = 0xa54ff53a;
    state[4] = 0x510e527f;
    state[5] = 0x9b05688c;
    state[6] = 0x1f83d9ab;
    state[7] = 0x5be0cd19;
}

void SHA256::update(const uint8_t* new_data, size_t length)
{
    for (size_t i = 0; i < length; i++) {
        data[blocklen++] = new_data[i];
        if (blocklen == 64) {
            transform();
            bitlen += 512;
            blocklen = 0;
        }
    }
}

void SHA256::update(const std::string& new_data)
{
    update(reinterpret_cast<const uint8_t*>(new_data.c_str()), new_data.size());
}

std::array<uint8_t, 32> SHA256::digest()
{
    std::array<uint8_t, 32> hash;
    pad();
    revert(hash);
    return hash;
}

uint32_t SHA256::rotr(uint32_t x, uint32_t n)
{
    return (x >> n) | (x << (32 - n));
}

uint32_t SHA256::choose(uint32_t e, uint32_t f, uint32_t g)
{
    return (e & f) ^ (~e & g);
}

uint32_t SHA256::majority(uint32_t a, uint32_t b, uint32_t c)
{
    return (a & (b | c)) | (b & c);
}

uint32_t SHA256::sig0(uint32_t x)
{
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

uint32_t SHA256::sig1(uint32_t x)
{
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

void SHA256::transform()
{
    uint32_t maj, xorA, ch, xorE, sum, newA, newE, m[64];
    uint32_t new_state[8];

    // Prepare the message schedule array
    for (uint8_t i = 0, j = 0; i < 16; i++, j += 4) {
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    }
    for (uint8_t k = 16; k < 64; k++) {
        m[k] = sig1(m[k - 2]) + m[k - 7] + sig0(m[k - 15]) + m[k - 16];
    }

    // Copy current hash state to working variables
    for (uint8_t i = 0; i < 8; i++) {
        new_state[i] = state[i];
    }

    // Compression function main loop
    for (uint8_t i = 0; i < 64; i++) {
        maj = majority(new_state[0], new_state[1], new_state[2]);
        xorA = rotr(new_state[0], 2) ^ rotr(new_state[0], 13) ^ rotr(new_state[0], 22);
        ch = choose(new_state[4], new_state[5], new_state[6]);
        xorE = rotr(new_state[4], 6) ^ rotr(new_state[4], 11) ^ rotr(new_state[4], 25);
        sum = m[i] + K[i] + new_state[7] + ch + xorE;
        newA = xorA + maj + sum;
        newE = new_state[3] + sum;

        new_state[7] = new_state[6];
        new_state[6] = new_state[5];
        new_state[5] = new_state[4];
        new_state[4] = newE;
        new_state[3] = new_state[2];
        new_state[2] = new_state[1];
        new_state[1] = new_state[0];
        new_state[0] = newA;
    }

    // Add the compressed chunk to the current hash value
    for (uint8_t i = 0; i < 8; i++) {
        state[i] += new_state[i];
    }
}

void SHA256::pad()
{
    // Save the current block length (the number of bytes already in the block)
    uint64_t orig_blocklen = blocklen;

    // Append the bit '1' (i.e. 0x80) to the message.
    data[blocklen++] = 0x80;

    // If the current block length is now greater than 56 bytes,
    // pad with zeros, transform the block, and then reset blocklen.
    if (blocklen > 56) {
        while (blocklen < 64) {
            data[blocklen++] = 0x00;
        }
        transform();
        blocklen = 0;
    }

    // Pad with zeros until the block is 56 bytes long.
    while (blocklen < 56) {
        data[blocklen++] = 0x00;
    }

    // Compute the total message length in bits.
    uint64_t total_bits = bitlen + orig_blocklen * 8;

    // Append the length as a 64-bit big-endian integer.
    for (int i = 0; i < 8; ++i) {
        data[63 - i] = total_bits & 0xff;
        total_bits >>= 8;
    }

    // Process the final block.
    transform();
}

void SHA256::revert(std::array<uint8_t, 32>& hash)
{
    // SHA uses big-endian byte ordering, so convert each 32-bit chunk.
    for (uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            hash[i + (j * 4)] = (state[j] >> (24 - i * 8)) & 0x000000ff;
        }
    }
}

std::string SHA256::to_string(const std::array<uint8_t, 32>& digest)
{
    std::stringstream s;
    s << std::setfill('0') << std::hex;
    for (uint8_t byte : digest) {
        s << std::setw(2) << static_cast<unsigned int>(byte);
    }
    return s.str();
}
