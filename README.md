# Hierarchical deterministic password manager

## What is this password manager

This is a proof-of-concept deterministic mnemonic-based password manager. It utilizes the seed generated from mnemonic<sup>\[[2](#BIP-39)\]</sup> to provision secrets<sup>\[[1](#BIP-32)\]</sup> (as of now - passwords). This allows for synchronization of passwords across devices without the need to store _any_ secrets on third-party servers. After synchronization of the metadata (password groups, names and details) via third-party servers, the passwords are generated from mnemonic inputted by the user on each new device.


## The idea behind this project

The possibility of using a password manager that stores secrets on a third-party server is not acceptable, but so is using one that does not synchronize the secrets across devices. Using the mnemonic to generate a seed enables just that - the metadata can be safely stored on a third-party server, as it only holds the dedrivation path, and the secrets themselves are generated from a mnemonic sentence. The mnemonic can be written on a piece of paper or engraved on a metal plate.


## Difference from brain wallets

Brain wallets (using hashes of words as secrets) are susceptible to dictionary attacks.

The mnemonic at the heart of this password manager is not.


## Roadmap

0. Create working proof of concept
1. Define a roadmap
2. Formally specify seed generation, secret derivation and metadata exchange format


## Contributing

Contributions are welcome! Please remember to run clang-format and clang-tidy before submitting PRs.


## Requirements

Build requirements: cmake, gcc>=8 or clang>=6, ncursesw.

Develompent: all of the above + clang-tidy and clang-format.

## Compilation Instructions

**MACOS (10.15)**  
This project requires an `ncurses` implementation, you can use homebrew to get one:
```bash
$ brew install ncurses
```
Then it's time to compile with:
```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## License

All code outside "[external](external)" is licensed under GPLv3.

## References

### BIP-32
Bitcoin's [hardened] key derivation [BIP-32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki)

### BIP-39
Bitcoin's mnemonic deterministic key generation [BIP-39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki)
