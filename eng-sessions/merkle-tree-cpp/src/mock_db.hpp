#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <stdexcept>
#include <optional>

/**
 * A simple mock for a key-value store, mimicking the minimal interface we need
 * from something like LevelUp in the TypeScript code.
 *
 * In a real DB scenario, these operations would be asynchronous and possibly
 * require a batch process for efficiency.
 *
 * We'll add a simple 'batch_write' as a "bonus" that would let us do a
 * combined write operation. In this mock, it's effectively just multiple puts.
 */


/**
 * A "batch" operation is just a list of (key, value) pairs to write.
 */
struct MockDBBatchItem {
    std::string key;
    std::array<uint8_t, 32> value;
};

class MockDB {
public:
    MockDB() = default;
    // Store to mock a DB.
    std::unordered_map<std::string, std::array<uint8_t, 32>> store{};
    // retrieve a value from the store
    std::optional<std::array<uint8_t, 32>> get(const std::string& key) const
    {
        auto it = store.find(key);
        if (it == store.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    // put a value into the store
    void put(const std::string& key, const std::array<uint8_t, 32>& value) { store[key] = value; }

    // bonus: naive batch write (in a real DB, this might be atomic)
    void batch_write(const std::vector<MockDBBatchItem> &items) {
        for (auto &item : items) {
            store[item.key] = item.value;
        }
    }
};
