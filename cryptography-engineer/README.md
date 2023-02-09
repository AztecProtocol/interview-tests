## Cryptography Take-Home Tests

Welcome to Aztec's cryptography tests as a part of your interview process. This module contains some coding tests designed to be attempted by candidates for either of the roles listed below.

1. Cryptography Engineer
2. Applied Cryptography Engineer
3. Applied Cryptographer

This module contains the following exercises:

1. [Indexed Merkle Tree](./src/indexed_merkle_tree/README.md)
2. [EC-FFT](./src/ec_fft/README.md)

Since these exercises use our in-house cryptography library barretenberg in the backend, you need to run the following commands to get started after you have cloned this repository. Please _do not_ fork the original repository, instead clone it and push it to your private repository.

```console
$ cd cryptography-engineer
$ ./bootstrap.sh              # this clones the barretenberg submodule and builds it
$ cd build
$ make <module_name>_tests    # this compiles the given test module
$ ./bin/<module_name>_tests   # this runs the tests in that module
```

Here, `module_name` must be replaced with `indexed_merkle_tree` for the first exercise. In case you face any issues with setting up this framework, feel free to reach out to [suyash@aztecprotocol.com](mailto:suyash@aztecprotocol.com) or [cody@aztecprotocol.com](mailto:cody@aztecprotocol.com).
