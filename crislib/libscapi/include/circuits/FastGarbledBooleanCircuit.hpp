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
//#include <ScGarbledCircuitNoFixedKey/GarbledBooleanCircuit.h>
//#include <ScGarbledCircuitNoFixedKey/FastGarblingFourToTwoNoAssumptions.h>
//#include <ScGarbledCircuitNoFixedKey/FastGarblingFreeXorHalfGatesFixedKeyAssumptions.h>
#include <ScGarbledCircuit/RowReductionGarbledBooleanCircuit.h>
#include <ScGarbledCircuit/StandardGarbledBooleanCircuit.h>
#include <ScGarbledCircuit/FreeXorGarbledBooleanCircuit.h>
#include <ScGarbledCircuit/HalfGatesGarbledBooleanCircuit.h>

#include "../infra/Common.hpp"
#include "BooleanCircuits.hpp"
#include <openssl/rand.h>

/**
* This is a general interface for garbled tables holder. <P>
* There are multiple ways to hold a garbled tables, each one of them will have a concrete class that implement this interface.
*/
class GarbledTablesHolder {
public:
	/**
	* There are cases when we do not know which concrete holder we use and we need a general function that returns the garbled tables.
	* Thus we add this function that returns the tables in the most basic format - that all classes should have the ability to translate to - byte[][].
	* @return the garbled tables in a byte[][] format.
	*/
	virtual byte** toDoubleByteArray() =0 ;
	virtual int getArraySize(int arrIndex) = 0;
};

/**
* This class holds the garbled tables of the justGarbled circuit.<p>
* In just garbled the garbled tables is held in one dimensional byte array. Thus, when we wish to
* relate to it as a double byte array as held in SCAPI, we use a double byte array whose first location
* holds the one dimensional byte array.
* The garbled circuit will hold an instance of this class. <p>
* This way, when we want to change the garbled tables, we just have to change the pointer of the tables in this class.
*/
class JustGarbledGarbledTablesHolder : public GarbledTablesHolder {
private:
	byte* garbledTables;
	int size;
public:
	/**
	* Sets the given garbled tables.
	* @param garbledTables
	*/
	JustGarbledGarbledTablesHolder(byte* garbledTables, int size) { 
		this->garbledTables = garbledTables; 
		this->size = size;
	};
	byte** toDoubleByteArray() override {
		byte** garbledTablesInZeroLocation = new byte*[1];
		garbledTablesInZeroLocation[0] = garbledTables;
		return garbledTablesInZeroLocation;
	};
	/**
	* Sets the given garbled tables. <P>
	* This allows changing the circuit inner content with no time.
	* @param garbledTables of the circuit.
	*/
	void setGarbledTables(byte* garbledTables, int size) { 
		this->garbledTables = garbledTables; 
		this->size = size;
	};

	int getArraySize(int arrIndex) override {
		if (arrIndex != 0)
			throw invalid_argument("JustGarbledGarbledTablesHolder has only one array");
		return size;
	};
};

/**
* A class that hold the values used to create the circuit. <p>
* These values are:<P>
* 1. Both keys of the input and the output wires.<p>
* 2. The translation table of the circuit.<p>
*/
class FastCircuitCreationValues {
private:
	byte* allInputWireValues=NULL;
	byte* allOutputWireValues = NULL;
	byte* translationTable = NULL;
public:
	FastCircuitCreationValues() {};
	/**
	* Sets the given arguments.
	* @param allInputWireValues Both keys for all input wires.
	* @param allOutputWireValues Both keys for all output wires.
	* @param translationTable Signal bits of all output wires.
	*/
	FastCircuitCreationValues(byte* allInputWireValues, byte*  allOutputWireValues, byte*  translationTable) {
		this->allInputWireValues = allInputWireValues;
		this->allOutputWireValues = allOutputWireValues;
		this->translationTable = translationTable;
	}
	byte*  getAllInputWireValues() { return allInputWireValues; };
	byte*  getAllOutputWireValues() { return allOutputWireValues; };
	byte*  getTranslationTable() { return translationTable; };
};

/**
* {@code FastGarbledBooleanCircuit} is a general interface for all basic garbled circuits.
* Fast garbled boolean circuit includes the same functionality as the regular garbled boolean circuit
* but it does it faster due to simpler data structures.<p>

* As the garbledBooleanCircuit interface, fast garbled circuits have four main operations: <p>
* 1. The {@link #garble()} function that generates the keys and creates the garbled tables. <p>
* 2. The {@link #compute()} function computes a result on a garbled circuit whose input has been set. <p>
* 3. The {@link #verify(BooleanCircuit, Map)} method is used in the case of a malicious adversary to verify that the garbled circuit
* created is an honest garbling of the agreed upon non garbled circuit. For example, the constructing party constructs many garbled circuits and
* the second party chooses all but one of them to verify and test the honesty of the constructing party.<p>
* 4. The {@link #translate(Map)} that translates the garbled output from {@link #compute()} into meaningful output.<p>
*/
class FastGarbledBooleanCircuit {
public:
	/**
	* This method generates both keys for each wire. Then, creates the garbled table according to those values.<p>
	* @return FastCircuitCreationValues contains both keys for each input and output wire and the translation table.
	*/
	virtual FastCircuitCreationValues garble()=0;
	/**
	* This method generates both keys for each input wire using the seed.
	* It then creates the garbled table according to those values.<p>
	* @param seed Used to initialize the prg.
	* @return FastCircuitCreationValues Contains both keys for each input and output wire and the translation table.
	*/
	virtual FastCircuitCreationValues garble(byte * seed)=0;
	/**
	* This method takes an array containing the <b> non garbled</b> values, both keys for all input wires and the party number which the inputs belong to. <p>
	* This method then performs the lookup on the allInputWireValues according to the party number and returns the keys
	* of the corresponding input bits.
	* @param ungarbledInputBits An array containing the <b> non garbled</b> value for each input wire of the given party number.
	* @param allInputWireValues The array containing both garbled values (keys) for each input wire.
	* The input values are placed one after another, meaning that the input values are in the following format:
	*  [k0,0   k0,1    k1,0   k1,1   k2,0   k2,1 ....] (while k0,1 is key 1 of wire 0).
	* @param partyNumber The number of party which the inputs belong to.
	* @return an array containing a single key of each input wire of the given party.
	*/
	virtual byte* getGarbledInputFromUngarbledInput(byte* ungarbledInputBits, byte* allInputWireValues, int partyNumber) = 0;
	virtual void setInputs(vector<byte> & garbledInputs)=0;
	/**
	* This method computes the circuit using the given inputs. <p>
	* It returns an array containing the garbled output. This output can be translated via the {@link #translate()} method.
	* @param garbledInput A single key for each input wire.
	* @return returns an array containing the garbled value of each output wire.
	* @throws NotAllInputsSetException if the given inputs array does not includes a key for all input wires.
	*/
	virtual byte* compute() = 0;
	/**
	* The verify method is used in the case of malicious adversaries.<p>
	* Alice constructs n circuits and Bob can verify n-1 of them (of his choice) to confirm that they are indeed garbling of the
	* agreed upon non garbled circuit. In order to verify, Alice has to give Bob both keys for each of the input wires.
	* @param allInputWireValues An array containing both keys for each input wire.
	* The input values are placed one after another, meaning that the input values are in the following format:
	*  [k0,0   k0,1    k1,0   k1,1   k2,0   k2,1 ....] (while k0,1 is key 1 of wire 0).
	* @return {@code true} if this {@code GarbledBooleanCircuit} is a garbling the given keys, {@code false} if it is not.
	*/
	virtual bool verify(byte* allInputWireValues) = 0;
	/**
	* This function behaves exactly as the verify(vector<byte> allInputWireValues) method except the last part.
	* The verify function verifies that the translation table matches the resulted output garbled values, while this function does not check it
	* but return the resulted output garbled values.
	* @param allInputWireValues An array containing both keys for each input wire.
	* The input values are placed one after another, meaning that the input values are in the following format:
	*  [k0,0   k0,1    k1,0   k1,1   k2,0   k2,1 ....] (while k0,1 is key 1 of wire 0).
	* @param allOutputWireValues An array containing both keys for each output wire.
	* The output values are placed one after another, meaning that the output values are in the following format:
	*  [k0,0   k0,1    k1,0   k1,1   k2,0   k2,1 ....] (while k0,1 is key 1 of wire 0).
	* When calling the function this array should be empty and will be filled during the process of the function.
	* @return {@code true} if this {@code GarbledBooleanCircuit} is a garbling the given keys, {@code false} if it is not.
	*/
	virtual bool internalVerify(byte* allInputWireValues, byte* allOutputWireValues)=0;
	/**
	* This function does the last part of the verify function. It gets both keys of each output wire and checks that
	* their signal bits match the corresponding bit in the translation table.<p>
	*
	* The internalVerify function followed by this function are actually executes the whole verify of the circuit.
	* @param allOutputWireValues both keys of each output wire.
	* The output values are placed one after another, meaning that the output values are in the following format:
	*  [k0,0   k0,1    k1,0   k1,1   k2,0   k2,1 ....] (while k0,1 is key 1 of wire 0).
	* @return {@code true} if the given keys match the translation table ,{@code false} if not.
	*/
	virtual bool verifyTranslationTable(byte* allOutputWireValues)=0;
	/**
	* Translates the garbled output obtained from the {@link #compute()} function into a meaningful(i.e. 0-1) output.<p>
	* @param garbledOutput An array contains the garbled output.
	* @return an array contains the output bit for each output wire.
	*/
	virtual byte* translate(byte* garbledOutput)=0;
	/**
	* Verifies that the given garbledOutput is valid values according to the given all OutputWireValues. <p>
	* Meaning, for each output wire, checks that the garbled wire is one of the two possibilities.
	* Then, translates the garbled output obtained from the {@link #compute()} function into a meaningful(i.e. 0-1) output.<p>
	* @param garbledOutput An array contains the garbled output.
	* @param allOutputWireValues both values for each output wire.
	* The output values are placed one after another, meaning that the output values are in the following format:
	*  [k0,0   k0,1    k1,0   k1,1   k2,0   k2,1 ....] (while k0,1 is key 1 of wire 0).
	* @return an array contains the output bit for each output wire.
	* @throws CheatAttemptException if there is a garbledOutput values that is not one of the two possibilities.
	*/
	virtual byte* verifiedTranslate(byte* garbledOutput, byte* allOutputWireValues)=0;
	/**
	* The garbled tables are stored in the circuit for all the gates. This method returns the garbled tables. <p>
	* This function is useful if we would like to pass many garbled circuits built on the same boolean circuit. <p>
	* This is a compact way to define a circuit, that is, two garbled circuit with the same multi encryption scheme and the same
	* basic boolean circuit only differ in the garbled tables and the translation table. <p>
	* Thus we can hold one garbled circuit for all the circuits and only replace the garbled tables (and the translation tables if
	* necessary). The advantage is that the size of the tables only is much smaller that all the information stored in the circuit
	* (gates and other member variables). The size becomes important when sending large circuits.
	*
	*/
	virtual GarbledTablesHolder * getGarbledTables()=0;
	/**
	* Sets the garbled tables of this circuit.<p>
	* This function is useful if we would like to pass many garbled circuits built on the same boolean circuit. <p>
	* This is a compact way to define a circuit, that is, two garbled circuit with the same multi encryption scheme and the same
	* basic boolean circuit only differ in the garbled tables and the translation table. <p>
	* Thus we can hold one garbled circuit for all the circuits and only replace the garbled tables (and the translation tables if necessary).
	* The advantage is that the size of the tables only is much smaller that all the information stored in the circuit (gates and other
	* member variables). The size becomes important when sending large circuits.<p>
	* The receiver of the circuits will set the garbled tables for the relevant circuit.
	*/
	virtual void setGarbledTables(GarbledTablesHolder * garbledTables)=0;
	/**
	* Returns the translation table of the circuit. <P>
	* This is necessary since the constructor of the circuit may want to pass the translation table to an other party. <p>
	* Usually, this will be used when the other party (not the constructor of the circuit) creates a circuit, sets the garbled tables
	* and needs the translation table as well to complete the construction of the circuit.
	* @return the translation table of the circuit.
	*/
	virtual byte* getTranslationTable()=0;
	virtual int getTranslationTableSize() =0;
	/**
	* Sets the translation table of the circuit. <p>
	* This is necessary when the garbled tables where set and we would like to compute the circuit later on.
	* @param translationTable This value should match the garbled tables of the circuit.
	*/
	virtual void setTranslationTable(byte* translationTable)=0;
	/**
	* Returns the input wires' indices of the given party.
	* @param partyNumber The number of the party which we need his input wire indices.
	* @return an array contains the indices of the input wires of the given party number.
	* @throws NoSuchPartyException In case the given party number is not valid.
	*/
	virtual int* getInputWireIndices(int partyNumber) = 0;
	/**
	* @return an array containing the indices of the circuit's output wires.
	*/
	virtual int* getOutputWireIndices()=0;
	/**
	* @return an array containing the indices of the circuit's input wires.
	*/
	virtual int* getInputWireIndices()=0;
	/**
	* Returns the number of input wires of the given party.
	* @param partyNumber the number of the party which we need his number of inputs.
	* @return the number of inputs of this circuit.
	* @throws NoSuchPartyException In case the given party number is not valid.
	*/
	virtual int getNumberOfInputs(int partyNumber) = 0;

	virtual int getNumberOfOutputs() = 0;
	/**
	* Returns the number of parties using this circuit.
	*
	*/
	virtual int getNumberOfParties()=0;
	/**
	*
	* @return the size of the keys, in bytes.
	*/
	virtual int getKeySize()=0;
};

class NativeGarbledBooleanCircuitImpl : public FastGarbledBooleanCircuit {
public:
	FastCircuitCreationValues garble() override;
	FastCircuitCreationValues garble(byte * seed) override;
	bool verify(byte* allInputWireValues) override;
	byte* getGarbledInputFromUngarbledInput(byte* ungarbledInputBits, byte* allInputWireValues, int partyNumber) override;
	void setInputs(vector<byte>& garbledInputs) override { this->garbledInputs = garbledInputs; };
	byte* verifiedTranslate(byte* garbledOutput, byte * allOutputWireValues) override { return NULL; }
	int* getInputWireIndices(int partyNumber) override;
	byte* getTranslationTable() override { return garbledCircuitPtr->getTranslationTable(); };
	int getTranslationTableSize() override { return garbledCircuitPtr->getNumberOfOutputs(); };
	void setTranslationTable(byte* translationTable) override {
		garbledCircuitPtr->setTranslationTable(translationTable);
	};
	int* getOutputWireIndices() override { return garbledCircuitPtr->getOutputIndices(); };
	int getNumberOfInputs(int partyNumber) override { return garbledCircuitPtr->getNumOfInputsForEachParty()[partyNumber - 1]; };
	int getNumberOfOutputs() { return garbledCircuitPtr->getNumberOfOutputs(); };
	int getNumberOfParties() override { return garbledCircuitPtr->getNumberOfParties(); };
	int* getInputWireIndices() override { return garbledCircuitPtr->getInputIndices(); };
	int getKeySize() override { return SCAPI_NATIVE_KEY_SIZE; };
	byte* translate(byte * garbledOutput) override;
	bool verifyTranslationTable(byte* allOutputWireValues) override;
	~NativeGarbledBooleanCircuitImpl() { delete garbledCircuitPtr; };

protected:
	static const int SCAPI_NATIVE_KEY_SIZE = 16; // the number of bytes in each just garbled key 
	GarbledBooleanCircuit * garbledCircuitPtr = NULL; // pointer to the native garbledCircuit object
	vector<byte> garbledInputs;
};
/**
* A concrete implementation of FastGarbledBooleanCircuit that is a wrapper for a code of SCAPI written in c++y.<p>
* The circuit can be used as a regular circuit in java and the actual calculations are done in the c++ jni dll
* calling functions in the Native SCAPI library. In some cases, there is a need to get back information that
* is stored in the java class (such as the garbled tables, input keys, etc). This java wrapper gives us
* the flexibility to work from java, for example with 2 parties and sending information via the java channel.
*
* @author Cryptography and Computer Security Research Group Department of Computer Science Bar-Ilan University (Meital Levy)
*
*/
//class ScNativeGarbledBooleanCircuitNoFixedKey : public NativeGarbledBooleanCircuitImpl {
//public:
//	/*
//	* fileName the name of the circuit file.
//	* isFreeXor a flag indicating the use of the optimization of FreeXor
//	*/
//	ScNativeGarbledBooleanCircuitNoFixedKey(string fileName, bool isFreeXor);
//	byte* compute() override;
//	bool internalVerify(byte * allInputWireValues, byte* allOutputWireValues) override;
//	byte* verifyTranslate(byte* garbledOutput, byte* bothOutputKeys);
//	GarbledTablesHolder * getGarbledTables() override;
//	void setGarbledTables(GarbledTablesHolder * garbledTables) override;
//};

class ScNativeGarbledBooleanCircuit : public NativeGarbledBooleanCircuitImpl {
public:
	enum CircuitType {
		FREE_XOR_HALF_GATES,
		FREE_XOR_ROW_REDUCTION,
		FREE_XOR_STANDARD,
		STANDARD
	};
	ScNativeGarbledBooleanCircuit(string fileName, CircuitType type, bool isNonXorOutputsRequired);
	byte* compute() override;
	bool internalVerify(byte * allInputWireValues, byte* allOutputWireValues) override;
	GarbledTablesHolder * getGarbledTables() override;
	void setGarbledTables(GarbledTablesHolder * garbledTables) override;
private:
	bool isNonXorOutputsRequired;
	int getGarbledTableSize();
};

inline void* aligned_malloc(size_t size, size_t align) {
	void *result;
#ifdef _MSC_VER 
	result = _aligned_malloc(size, align);
#else 
	if (posix_memalign(&result, align, size)) result = 0;
#endif
	return result;
};

inline void aligned_free(void *ptr) {
#ifdef _MSC_VER 
	_aligned_free(ptr);
#else 
	free(ptr);
#endif

};