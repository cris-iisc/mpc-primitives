# LIBSCAPI - The Secure Computation API

libscapi is the Open source C++ library for implementing high performance secure two-party and multiparty computation protocols (SCAPI stands for the "Secure Computation API"). It provides a reliable, efficient, and highly flexible cryptographic infrastructure.

The latest release can be found here. Alternately you can simply clone the repository to get the latest and greatest.

Libscapi is developed by [Bar Ilan University Cryptography Research Group](http://crypto.biu.ac.il/). The goal of libscapi is to promote research by Academy and Industry practitioners in this field by providing:

- A consistent API over Primitives, Mid-Layer Protocols, Interactive Mid-Layer Protocols and Communication Channels, simplifying the development and evaluation fo new protocols. We focus on keeping libscapi easy to build and use.
- Integrating best performance open-source implementations by other Academy Research Institutes.  
- High Performance implementation on standard Linux & Intelx64 Architecture. We use modern techniques like Intel Intrinsics Instructions, Pipelining and TCP optimizations. However, we avoid using techniques that are too advanced or not available on common platforms (such as Intel AVX-512 and DPDK, GPGPU exc).   
- Provide a common platfrom for benchmarking different alogirthms and implementations

## libscapi Modules
- __Primitives__: Dlog, Cryptographic Hash Function, HMAC and KDF, Pseudorandom Functions and Permutations, Pseudo Random Generator, Trapdoor Permutation, Random Oracle (to be elaborated)
- __Mid-layer protocols__: Currently includes Public Key Encryption Schemes: Cramer-Shoup, Damgard-Jurik, El-Gamal
- __Interactive Mid-layer protocols__: Sigma Protocols, Zero Knowledge Proofs, Commitment Schemes (to be elaborated)
- __OT Extension__ - Semi-Honest and Malicious 
- __Circuits__: To be elaborated
- __Communication Channel__: To be elaborated

## License information
Libscapi is released under the MIT open source license. However, some of the libraries we use have different licenses. For further information pleare refer to [LICENSE.MD](build_scripts/LICENSE.MD)

## Documentation

Go to http://biulibscapi.readthedocs.org/ for a detailed explanations of our implementation.

## Installing libscapi

Libscapi has is tested on Ubuntu 14.04/16.04LTS and should run on all major versions of Linux. It also executes on Windows 8.1/10 but with performance limitations. Internally we use the Windows option for development only. All tests execute on Linux.
Mac OSX has not been tested so far. 
For detailed instructions, see [INSTALL.MD](build_scripts/INSTALL.MD)

## Libraries used by libscapi

### Math and General Purpose Libraries

##### OpenSSL
[https://www.openssl.org/](https://www.openssl.org/)

 OpenSSL is an open source project that provides a robust, commercial-grade, and full-featured toolkit for the Transport Layer Security (TLS) and Secure Sockets Layer (SSL) protocols. It is also a general-purpose cryptography library. For more information about the team and community around the project, or to start making your own contributions, start with the community page. To get the latest news, download the source, and so on, please see the sidebar or the buttons at the top of every page.

##### The GNU Multiple Precision Arithmetic Library (GMP)
[https://gmplib.org/](https://gmplib.org/)

GMP is a free library for arbitrary precision arithmetic, operating on signed integers, rational numbers, and floating-point numbers. There is no practical limit to the precision except the ones implied by the available memory in the machine GMP runs on. GMP has a rich set of functions, and the functions have a regular interface.
The main target applications for GMP are cryptography applications and research, Internet security applications, algebra systems, computational algebra research, etc.

##### NTL: A Library for doing Number Theory- Victor Shoup
[ttp://www.shoup.net/ntl/](http://www.shoup.net/ntl/)

 NTL is a high-performance, portable C++ library providing data structures and algorithms for manipulating signed, arbitrary length integers, and for vectors, matrices, and polynomials over the integers and over finite fields.On modern platforms supporting C++11, NTL can be compiled in thread safe and exception safe modes. 

##### MIRACL Cryptographic SDK
[https://github.com/miracl/MIRACL](https://github.com/miracl/MIRACL)

MIRACL Cryptographic SDK: Multiprecision Integer and Rational Arithmetic Cryptographic Library C software library

##### Boost 1.64
[http://www.boost.org/](http://www.boost.org/)

Boost provides free peer-reviewed portable C++ source libraries. We emphasize libraries that work well with the C++ Standard Library. Boost libraries are intended to be widely useful, and usable across a broad spectrum of applications

##### Cereal C++ Serialization Library
[https://github.com/USCiLab/cereal](http://uscilab.github.io/cereal/)

cereal is a header-only C++11 serialization library. cereal takes arbitrary data types and reversibly turns them into different representations, such as compact binary encodings, XML, or JSON. cereal was designed to be fast, light-weight, and easy to extend - it has no external dependencies and can be easily bundled with other code or used standalone.

##### JsonCpp : A library for creating json files in C++
[https://github.com/open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)

JsonCpp is used to create our logger log files.

### Implementations by other Academic Institutes

##### Engineering Cryptographic Protocols Group at TU Darmstadt OT Extension
[https://github.com/encryptogroup/OTExtension](https://github.com/encryptogroup/OTExtension)

Implementation of the passive secure OT extension protocol of [1] and the active secure OT extension protocols of [2] and [3]. Implements the general OT (G_OT), correlated OT (C_OT), global correlated OT (GC_OT), sender random OT (SR_OT), and receiver random OT (RR_OT) (Definitions of the functionalities will follow). Implements the base-OTs by Naor-Pinkas [4], Peikert-Vaikuntanathan-Waters [5], and Chou-Orlandi [6]. The code is based on the OT extension implementation of [7] and uses the MIRACL libary [8] for elliptic curve arithmetic. Update: Implemented 1-out-of-2 OT from the 1-out-of-N OT extension of [10].

##### University of Bristol: Advanced Protocols for Real-world Implementation of Computational Oblivious Transfers
[https://github.com/bristolcrypto/apricot](https://github.com/bristolcrypto/apricot)

##### Tung Chou and Claudio Orlandi: The Simplest Oblivious Transfer Protocol
[http://users-cs.au.dk/orlandi/simpleOT/](http://users-cs.au.dk/orlandi/simpleOT/)
