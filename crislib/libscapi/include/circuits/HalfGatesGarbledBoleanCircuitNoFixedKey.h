/**
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* 
* Copyright (c) 2016 LIBSCAPI (http://crypto.biu.ac.il/SCAPI)
* This file is part of the SCAPI project.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
* We request that any publication and/or code referring to and/or based on SCAPI contain an appropriate citation to SCAPI, including a reference to
* http://crypto.biu.ac.il/SCAPI.
* 
* Libscapi uses several open source libraries. Please see these projects for any further licensing issues.
* For more information , See https://github.com/cryptobiu/libscapi/blob/master/LICENSE.MD
*
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* 
*/

#pragma once
#include "GarbledBooleanCircuitNoFixedKey.h"


/*
* The HalfGatesGarbledBoleanCircuitNoFixedKey uses the Free XOR technique that is explained in depth in "Free XOR Gates and
* Applications" by Validimir Kolesnikov and Thomas Schneider. This class does not use the Fixed key assumption, unlike in the original paper. Instead, 
* it used a fast optimized teqniche written by Shay Gueron from Intel to set keys and encrypt utilizing the pipline.
* This class is based on the paper "Two Halves Make a Whole Reducing Data Transfer in Garbled Circuits using Half Gates" where each AND gate* uses only 2 rows in the garbled table. It is better than the Row reduction technique where each non Xor gate uses 3 rows.* This class only accept AND, XOR, XORNOT and NOT gates and does not deal with other general gates.* Unlike the previous techniques, this class also assumes circuilarity in addition to fixed key and free xor.
* It uses the optimization of XOR and NON_XOR gates that can be computed without an encryption due to the way the
* wire's keys were chosen. See the above papers also for a proof of security of this method. 
* This class implements the virtual functions garble and internalVerify since the diffrences with other circuits are fundemental and thus
* can not be used in the base abstract class.
*
* @author Cryptography and Computer Security Research Group Department of Computer Science Bar - Ilan University(Meital Levy)
*/
class HalfGatesGarbledBoleanCircuitNoFixedKey :
	public GarbledBooleanCircuitNoFixedKey
{
public:
	HalfGatesGarbledBoleanCircuitNoFixedKey();
	HalfGatesGarbledBoleanCircuitNoFixedKey(const char* fileName);
	virtual ~HalfGatesGarbledBoleanCircuitNoFixedKey();

private:

	block *deltaFreeXor;//This is used to get the second garbled value in freeXor optimization. The second garbled value is the XOR 
	//of the first key and the delta. The delta is chosen at random.
	//We use a pointer since new with 32 bit does not 16-align the variable by default.

	block* encryptedChunkKeys;//The result of chunk encrypting indexArray.
	block* indexArray;//An array that holds the number 0 to the number of nonXorGates and is calculated in advence.
	//The purpuse of this array is that we can calculate. this array and all the keys of the circuit in advence using ecb mode
	//with one chuck gaining pipelining	




protected:
	/*
	* Creates the memory needed for this class in addition to the memory that is allocated by the base class.
	*/
	void createCircuitMemory(const char* fileName, bool isNonXorOutputsRequired = false) override;



public:
	/**
	* This method computes the circuit for the given input singleWiresInputKeys.
	* It returns a the garbled values keys for the output wires. This output can be translated via the translate() method
	* if the translation table is set.
	*/
	void compute(block * singleWiresInputKeys, block * Output) override;

	/**
	* This function behaves exactly as the verify method except the last phase.
	* The verify function verifies that the translation table matches the resulted output garbled values, while this function does not, rather,
	* it returns the resulted output garbled values.
	*
	* In this verify method we compare the garbled table of the circuit with the garbled table that should be created
	* with the given input keys. Since the garble method does not use randomization except for picking the input keys using the seed,
	* the garbled table, given the input keys is deterministic.
	*
	* bothWiresInputKeys : both keys for each input wire. This array must be filled with both input keys
	* emptyBothWireOutputKeys :This array will be filled with both output keys during the process of the function. It must be empty.
	*
	* returns : true if the garbled table of this circuit is complied with the given input keys, false otherwise.
	*/
	bool internalVerify(block *bothInputKeys, block *emptyBothWireOutputKeys) override;

private:

	/*
	* This method generates both keys for each wire. Then, creates the garbled table according to those values with the row reduction technique.
	* In the row reduction technique the garbled table of has three encryptions instead of four, the last row is not saved and will be calculated
	* when the compute function will be called.
	* The keys for each wire are not saved. The input keys and the output keys that were created are retuned to the
	* user. The user usually saves these value for later use. The user also gets the generated translation table, which is
	* the signal bits of the output wires.
	*
	* emptyBothInputKeys : An empty block array that will be filled with both input keys generated in garble.
	* emptyBothOutputKeys : An empty block array that will be filled with both output keys generated in garble.
	* emptyTranslationTable : An empty char array that will be filled with 0/1 signal bits that wre choosen in random in this function.
	*/
	void garble(block *emptyBothInputKeys, block *emptyBothOutputKeys, std::vector<byte> & emptyTranslationTable, block seed) override;

	/*
	* This function inits the keys for all the wires in the circuit and initializes the two aes encryptions (seed and fixedKey as keys). It also choses
	* the input keys at random using the aes with seed. It also creates memory for the translation table.
	*/
	void initAesEncryptionsAndInputKeys(block* emptyBothInputKeys);

};

