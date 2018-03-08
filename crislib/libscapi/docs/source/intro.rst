Introduction
============

LibSCAPI is an *open-source* general library tailored for **Secure Computation** implementations. libscapi provides a flexible and efficient infrastructure for the implementation of secure computation protocols, that is both easy to use and robust. We hope that SCAPI will help to promote the goal of making secure computation practical.

Why Should I Use libscapi?
--------------------------

*  **libscapi provides uniformity.** As of today, different research groups are using different implementions. It is hard to compare different results, and implementations carried out by one group cannot be used by others. libscapi is trying to solve this problem by offering a modular codebase to be used as the standard library for Secure Computation.

*  **libscapi is flexible.** libscapi's lower-level primitives inherit from modular interfaces, so that primitives can be replaced easily. libscapi leaves the choice of which concrete primitives to actually use to the high-level application calling the protocol. This flexibility can be used to find the most efficient primitives for each specific problem.

*  **libscapi is efficient.** libscapi is implemented in c++ and wraps the most efficient libraries and implementations in order to run more efficiently. For example, elliptic curve operations in libscapi are implemented using the extremely efficient Miracl library written in C.

*  **libscapi is built to please.** libscapi has been written with the understanding that others will be using it, and so an emphasis has been placed on clean design and coding, documentation, and so on.


Architecture
------------

libscapi is composed of the following four layers:

1. **Low-level primitives:** these are functions that are basic building blocks for cryptographic constructions (e.g., pseudorandom functions, pseudorandom generators, discrete logarithm groups, and hash functions belong to this layer).

2. **Non-interactive mid-level protocols**: these are non-interactive functions that can be applications within themselves in addition to being tools (e.g., encryption and signature schemes belong to this layer).

3. **Interactive mid-level protocols:** these are interactive protocols involving two or more parties; typically, the protocols in this layer are popular building blocks like commitments, zero knowledge and oblivious transfer.

4. **High level Protocols:** these are implementations of known cryptographic multi-party and two-party protocols. For example, Yao and GMW.  

In addition to these four main layers, there is an orthogonal communication layer that is used for setting up communication channels and sending messages and also a circuit package that contains boolean circuits and garbled boolean circuit implementations used in libscapi's layers.


..
   Layer 1 - Basic Primitives
   --------------------------

   The first, lowest layer of libscapi contains basic cryptographic primitives. Most of our code at this level consists of wrapping code from other libraries into a unified format. The primitives implemented in this layer are: pseudorandom functions and permutations, cryptographic hash functions, universal hash functions, trapdoor permutations, pseudorandom generators, key derivation functions (a.k.a. randomness extractors), and discrete log groups.

   Layer 2 - Non Interactive Schemes
   ---------------------------------

   The second layer consists of non-interactive cryptographic schemes. Specifically, this layer contains symmetric and asymmetric encryption, message authentication codes and digital signatures. Regarding asymmetric encryption, SCAPI supports RSA-OAEP (from Bouncy Castle and from Crypto++), El-Gamal (over any discrete log group), Cramer-Shoup (over any discrete log group), and Damgard-Jurik additively homomorphic encryption (which is an extension of Paillier). We remark that both ElGamal and Cramer-Shoup can receive group elements or byte arrays as plaintext; the former case is often needed in protocols where the algebraic structure of the ciphertext is needed for efficiently  proving statements in zero knowledge.

   Layer 3 - Interactive Protocols
   -------------------------------

   The third layer of libscapi contains interactive protocols and schemes that are widely used in protocols for secure computation. The main schemes are:

   *  **Sigma protocols and zero knowledge:** SCAPI contains over 10 common Sigma protocols (e.g., discrete log, Diffie-Hellman tuple, etc.). In addition, the following operations on *arbitrary* Sigma protocols are included: AND of multiple statements, OR of two or many statements, transformation to zero-knowledge, transformation to zero-knowledge proof of knowledge, and Fiat-Shamir transformation to non-interactive zero-knowledge. 

   *  **Commitments:** SCAPI includes Pedersen commitments, ElGamal commitments, Hash-based commitments, and equivocal commitments. Additional schemes like extractable commitments, fully trapdoor commitments, homomorphic commitments, non-malleable commitments and UC-secure commitments will be released in the near future.

   *  **Oblivious transfer:** Many oblivious transfer protocols are implemented in SCAPI, with security in the presence of semi-honest and malicious adversaries. For the case of malicious adversaries, protocols achieving privacy only, one-sided simulation, full simulation-based security, and UC-security are included. In the very near future, highly optimized oblivious transfer extension for semi-honest adversaries will be included (enabling semi-honest OT at a rate of close to 1 million transfers per second).
