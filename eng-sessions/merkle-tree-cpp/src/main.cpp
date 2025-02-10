#include <cassert>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "hash_path.hpp"
#include "merkle_tree.hpp"
#include "mock_db.hpp"
#include "sha256_hasher.hpp"

/**
 * Utility function: converts a vector of bytes to a lowercase hexadecimal string.
 */
template <typename T> std::string to_hex(const T& data)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto byte : data) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    return oss.str();
}

/**
 * Asserts that the hexadecimal representation of 'actual' matches 'expected_hex'.
 * Throws std::runtime_error if the assertion fails.
 */
template <typename T>
void assert_equal_hex(const T& actual, const std::string& expected_hex, const std::string& test_message = "")
{
    std::string actual_hex = to_hex(actual);
    if (actual_hex != expected_hex) {
        std::ostringstream oss;
        oss << "\nAssertion failed! " << test_message << "\n"
            << "  Expected: " << expected_hex << "\n"
            << "  Actual  : " << actual_hex << "\n";
        throw std::runtime_error(oss.str());
    }
}

/**
 * Creates a 64-byte leaf value where the first 4 bytes encode the integer 'i' in little-endian order.
 */
std::vector<uint8_t> make_leaf_64(uint32_t i)
{
    std::vector<uint8_t> leaf(64, 0);
    leaf[0] = static_cast<uint8_t>(i & 0xff);
    leaf[1] = static_cast<uint8_t>((i >> 8) & 0xff);
    leaf[2] = static_cast<uint8_t>((i >> 16) & 0xff);
    leaf[3] = static_cast<uint8_t>((i >> 24) & 0xff);
    return leaf;
}

void run_tests()
{

    // Prepare 1024 leaves, each 64 bytes, with the index encoded in the first 4 bytes.
    std::vector<std::vector<uint8_t>> values(1024);
    for (int i = 0; i < 1024; ++i) {
        values[i] = make_leaf_64(i);
    }

    // Test 1: Verify that an empty tree of depth 32 has the correct root.
    {
        std::cout << "Test 1: Verify that an empty tree of depth 32 has the correct root." << std::endl;
        MockDB db;
        auto tree = MerkleTree::create(db, "test", 32);

        sha256_hash_t actual_root = tree.get_root();
        assert_equal_hex(actual_root,
                         "1c9a7e5ff1cf48b4ad1582d3f4e4a1004f3b20d8c5a2b71387a4254ad933ebc5",
                         "Empty tree root check at depth 32");
        std::cout << "Test 1 success" << std::endl;
    }

    // Test 2: Create a tree of depth 2 and verify correct root and hash paths.
    {
        std::cout << "Test 2: Create a tree of depth 2 and verify correct root and hash paths." << std::endl;
        MockDB db;
        Sha256Hasher hasher;

        // Compute expected hashes for 4 leaves.
        sha256_hash_t e00 = hasher.hash(values[0]);
        sha256_hash_t e01 = hasher.hash(values[1]);
        sha256_hash_t e02 = hasher.hash(values[2]);
        sha256_hash_t e03 = hasher.hash(values[3]);

        sha256_hash_t e10 = hasher.compress(e00, e01);
        sha256_hash_t e11 = hasher.compress(e02, e03);
        sha256_hash_t expected_root = hasher.compress(e10, e11);

        std::string expected_root_hex = to_hex(expected_root);
        if (expected_root_hex != "e645e6b5445483a358c4d15c1923c616a0e6884906b05c196d341ece93b2de42") {
            throw std::runtime_error("Error in the test's basic SHA256 assumptions!");
        }

        // Create a depth=2 tree and insert 4 leaves.
        auto tree = MerkleTree::create(db, "test", 2);
        for (int i = 0; i < 4; ++i) {
            tree.update_element(i, values[i]);
        }

        // Verify hash paths:
        // For indices 0 and 1, the hash path should be [ [e00, e01], [e10, e11] ]
        HashPath expected_path_0 = HashPath({ { e00, e01 }, { e10, e11 } });
        if (!(tree.get_hash_path(0) == expected_path_0)) {
            throw std::runtime_error("Hash path for index 0 does not match expected value.");
        }
        if (!(tree.get_hash_path(1) == expected_path_0)) {
            throw std::runtime_error("Hash path for index 1 does not match expected value.");
        }

        // For indices 2 and 3, the hash path should be [ [e02, e03], [e10, e11] ]
        HashPath expected_path_2 = HashPath({ { e02, e03 }, { e10, e11 } });
        if (!(tree.get_hash_path(2) == expected_path_2)) {
            throw std::runtime_error("Hash path for index 2 does not match expected value.");
        }
        if (!(tree.get_hash_path(3) == expected_path_2)) {
            throw std::runtime_error("Hash path for index 3 does not match expected value.");
        }

        // Verify the tree root.
        auto actual_root = tree.get_root();
        assert_equal_hex(actual_root, expected_root_hex, "Depth=2 root check");

        std::cout << "Test 2 success" << std::endl;
    }

    // Test 3: Verify that a tree of depth 10 can be restored from previous data.
    {
        std::cout << "Test 3: Verify that a tree of depth 10 can be restored from previous data." << std::endl;
        MockDB db;

        auto tree = MerkleTree::create(db, "test", 10);
        for (int i = 0; i < 128; ++i) {
            tree.update_element(i, values[i]);
        }
        auto tree2 = MerkleTree::create(db, "test", 10);
        assert_equal_hex(tree2.get_root(),
                         "4b8404d05a963de56f7212fbf8123204b1eb77a4cb16ae3875679a898aaa5daa",
                         "Restored depth=10 root check");

        for (int i = 0; i < 128; ++i) {
            HashPath hp1 = tree.get_hash_path(i);
            HashPath hp2 =
                tree2.get_hash_path(i);
            if (!(hp1 == hp2)) {
                throw std::runtime_error("Mismatch in hash paths for index " + std::to_string(i));
            }
        }

        std::cout << "Test 3 success" << std::endl;
    }

    // Test 4: Insert 1024 values into a depth=32 tree and verify final root and hash path for index 100.
    {
        std::cout
            << "Test 4: Insert 1024 values into a depth=32 tree and verify final root and hash path for index 100."
            << std::endl;
        MockDB db;

        auto tree = MerkleTree::create(db, "test", 32);
        for (int i = 0; i < 1024; ++i) {
            tree.update_element(i, values[i]);
        }

        // Expected final root from TypeScript test.
        std::string expected_hex = "26996bfcb0aaf96422aefdd789396a3f6c8a4fa6dccc73e55060e03e2a238db0";
        assert_equal_hex(tree.get_root(), expected_hex, "Insert 1024 values into 32-depth tree root check");

        // Expected hash path for index 100 (a long hex string from TS test).
        std::string expected_path_hex =
            "f59927591e6e3283d4419e376e4ebb4e08f4f547a3d1076474a29c9d44a07b28e703b6c67d0d1d2a7ef4bd70b8cda5840"
            "61db4d9e3673f79d3cafab5ecbd9b1e6478de41cd35e7937dd9ac9f1bb59aaeb71c3baec571778d8eb3e22116810bcda2"
            "15145f0f4ca7c22c5c149359d8597258d8f1e3630b5d74f9035a69bb19bd9bc415913d1a01741ed6e881871baa9e3b3fa"
            "bfa5ae6009a2ec009ea22bbced51b1077408bc95eae2d5ba2c7cdd8d5690d6fb27702d295fa801212f5d0bc6cc923b8d4"
            "5c43b2e08d36dbfba1e74a5297242c9c460a111c89067daf1c59a6d44a062c7882de61cfe65b71bc5fe9fe636c825cc6a"
            "96df0b5b4e2885f9e974e80a00fe5022a268652893af91ffa3bf545d9c852c7181547bbc6e0ce29ee85fac6a7eca17c8b"
            "1a90804b7f360d319e43e590be9e92c66c74ffc5579e5e6e309de98a1f9b4911d80fe9b0e3e9db5f0cc67b2ebeb29a706"
            "1f1c7a2c623495b1ec4e4bef82f9d5be75ae2392b70bda91a35285be08b82684cddf58f9a27c1c5dad5a9eb9f33801429"
            "23feebd36b5b4cf22d70b9552d82b887d00aa2da4b65442a31030e9b809d10356452b8f549fef05433e17b308a5acc5f4"
            "0ae0c7e106c916a03ce229ed8fab8cb151ae4353f689e40324ffbd0a76b0e9b82d6d5686bdd5510a7ed3e64aba719d04f"
            "9015a6f55f69930f5fbb0da3800640af69d10528e7360f76d1a58a6e0bce4cfe43cf119b8f5173b6e5cfa351f35253eec"
            "e828e6c1433f76b88329232f5a6988105158fc273823f7525a53821a2aed0a185f4857b12650dd7be180906dcfdf5b108"
            "0bb93e1a157ed947c2184dc35e097843a4094f88dd50eb4a0d5828b0055f617c272a0bf7ed0665ed9fc4789e817ab7305"
            "013faa7724a3cf5b2192203f55bcc35322e08aee3d77d686c4590b140f5d9047ea13380b2442e0d001a6cf04127db0544"
            "1cd833107a52be852868890e4317e6a02ab47683aa759642206fe8f537ae7a17d3e37896a770ffe6715a97863ce29ccc8"
            "a059a33c0026add13b7d05f875f140027ef5118a2247bbb84ce8f2f0f1123623085daf7960c329f5fa8ac942e18e6ce87"
            "a78488cda91b88d9407627093af97852b350208a21bee3d1df6af5f5bbdb6be9ef8aa618e4bf8073960867171e29676f8"
            "b284dea6a08a85ef446e2aa248796ac561107c12c1be9f3e6fe5e08a5d0c005f314a4d08e393118b58d900f5e182e3c50"
            "ef74969ea16c7726c549757cc23523c369587da72937846c7860e58be630894eb80ec8b079672089d2cd222544e39ba45"
            "6fc2c32716738d49a7502ffcfb0340b1d7885688500ca308161a7f96b62df9d083b71fcc8f2bbb22e774a0c40a7772a3f"
            "bc9fa8e5381b695652ca7af0fa783af2f08586dba0f18fe6b1689256c0d385f42f5bbe2027a22c1996e110ba97c171d3e"
            "5948de92beb661a23b4ea755618621a94f28e34249c24280de0e084a7336e3c870a2d7904758d0d63c39ebade8509e0ae"
            "3c9c3876fb5fa112be18f905ecacfecb92057603ab9935eeed1d12d309a184dc60b6fb92a0a9d0a6c3495d8478798b3c9"
            "b46fcf66995eec8b2e541cad4e91de38385f2e046619f54496c2382cb6cacd5b98c26f5a4adbdffbd8298a18020d86c3b"
            "1f750ab7b25a3e13a5b8a235a82bdbabbb498868f893e908917775b62bff23294dbbe3a1cd8e6cc1c35b4801887b646a6"
            "f81f17f321d317d099bfac9d91c2b818f184161d8c3b2516ae643914451fd0bb492dc5fcddba7b592e3133393c16194fa"
            "c7431abf2f5485ed711db282183c819e08ebaa86b650ff5ee1c07d86c6605fa93b83b4d83e59733b5d244008b841ca13c"
            "cb2058a8d7fe3af8caa085a7639a832001457dfb9128a8061142ad0335629ff23ff9c4b7bf8c19a29dbbed382a13fdb41"
            "dd8630d2c76ab6e5f971b3c47c5065387763feb3c337d7a51a6fbf00b9e34c52e1c9195c969bd4e7a0bfd51d5c5bed9c11"
            "67b6483422f56919ce2f54dd9a9b0162455a5a75e83ac7c1965b2a44530ed65098e71f0aa83cc32edfbefa9f4d3e0174ca"
            "85182eec9f3a09f6a6c0df6377a510d7f5edf6415b5d1e1b4726f61a52a115319e3284020532a55c2d2f21aa5e7b806231"
            "206fa80a50bb6abe29085058f16212212a60eec8f049fecb92d8c8e0a84bc02bf86d2cef052a013d83a745d32b7d111ed3"
            "269866091cb266897ddfe124546b21352bfecbeddde993839f614c3dac0a3ee37543f9b412b16199dc158e23b544c4dc4"
            "cf039332ba8521be07ced0b8b09735cebf122d1c1ee7e388e03922704fc619e312724bb6d7c3153ed9de791d764a366b3"
            "89af13c58bf8a8d90481a467653cb1a76abf102ae036aa9ecb12c4e36998ebc183f0efc1628c6c76b813b97ccc7cdd2986"
            "268250628d0c10e385c58c6191e6fbe05191bcc04f133f2cea72c1c4b0ae7a85396184a39332cfb7630df1dac5c25fed0d"
            "fca561e9b81c96681fdd6b848930bd7ba8cac54661072113fb278869e07bb8587f91392933374d017bcbe13c90183e8f0c"
            "1dc88aa7f3ee1776e8f8ca4fa5efcea07191d96c35b286c3382e8869ff2c22b28cc10510d9853292803328be4fb0e80495"
            "e8bb8d271f5b889636a1e5e441b5b5ce9d1fc3def4c6e474045348a0cac3e35f03e6e4324400d2b4ddb5fe28e79f1b850f"
            "8658246ce9b6a1e7b49fc06db7143e8fe0b4f2b0c5523a5c1d48bdddad3b8a062632e00d4fd83dc6ff8aab7bf3adc647e0"
            "f1cfdd43a81a65985e929f70af28d0bdd1a90a808f977f597c7c778c489e98d3bd8910d31ac0f769896f30b46bea4e13cbe"
            "6d1377a90416df6e1e265052ad6017c0c1f2b28b47cc6f67e02e6e4e1bdefb994c6098953f34636ba2b6ca20a4721d2b26a886"
            "722ff";
        auto hash_path_100 = tree.get_hash_path(100);
        std::string actual_path_hex = to_hex(hash_path_100.to_buffer());
        if (actual_path_hex != expected_path_hex) {
            throw std::runtime_error("HashPath for index 100 does not match expected value.");
        }
        std::cout << "Test 4 success" << std::endl;
    }

    std::cout << "All tests passed successfully!\n";
}

int main()
{
    try {
        run_tests();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Test error: " << ex.what() << std::endl;
        return 1;
    }
}
