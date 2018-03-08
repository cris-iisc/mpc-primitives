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

#include <boost/thread/thread.hpp>
#include "../../include/comm/Comm.hpp"
#define AES_KEY BC_AES_KEY // AES_KEY is defined both in GarbledBooleanCircuit and in OTSemiHonestExtension
//#include "../../include/circuits/FastGarbledBooleanCircuit.hpp"
#include "../../include/circuits/GarbledBooleanCircuit.h"
#include "../../include/interactive_mid_protocols/OTExtensionBristol.hpp"
#undef AES_KEY
#define AES_KEY OT_AES_KEY
#undef AES_KEY
#include <thread>
#include "../../include/infra/Scanner.hpp"
#include "../../include/infra/ConfigFile.hpp"



/**
* This is an implementation of party one of Yao protocol.
*/
class PartyOne {
private:
	OTBatchSender * otSender;			//The OT object that used in the protocol.	
	GarbledBooleanCircuit * circuit;	//The garbled circuit used in the protocol.
	CommParty * channel;				//The channel between both parties.
	tuple<block*, block*, vector<byte> > values;//this tuple includes the input and output keys (block*) and the translation table (vector)
														 //to be used after filled by garbling the circuit

	/**
	* Sends p1 input keys to p2.
	* @param ungarbledInput The boolean input of each wire.
	* @param bs The keys for each wire.
	*/
	void sendP1Inputs(byte* ungarbledInput);
public:
	/**
	* Constructor that sets the parameters of the OT protocol and creates the garbled circuit.
	* @param channel The channel between both parties.
	* @param bc The boolean circuit that should be garbled.
	* @param mes The encryption scheme to use in the garbled circuit.
	* @param otSender The OT object to use in the protocol.
	* @param inputForTest
	*/
	PartyOne(CommParty * channel, OTBatchSender * otSender, GarbledBooleanCircuit * circuit) {
		this->channel = channel;
		this->otSender = otSender;
		this->circuit = circuit;
	};
	/**
	* Runs the protocol.
	* @param ungarbledInput The input for the circuit, each p1's input wire gets 0 or 1.
	*/
	void run(byte* ungarbledInput);
	/**
	* Runs OT protocol in order to send p2 the necessary keys without revealing any other information.
	* @param allInputWireValues The keys for each wire.
	*/
	void runOTProtocol();
};

/**
* This is an implementation of party one of Yao protocol.
*/
class PartyTwo {
private:
	OTBatchReceiver * otReceiver;			//The OT object that used in the protocol.	
	GarbledBooleanCircuit * circuit;	//The garbled circuit used in the protocol.
	CommParty * channel;				//The channel between both parties.
	byte* p1Inputs;
	int p1InputsSize;

	/**
	* Compute the garbled circuit.
	* @param otOutput The output from the OT protocol, which are party two inputs.
	*/
	byte* computeCircuit(OTBatchROutput * otOutput);

public:
	/**
	* Constructor that sets the parameters of the OT protocol and creates the garbled circuit.
	* @param channel The channel between both parties.
	* @param bc The boolean circuit that should be garbled.
	* @param mes The encryption scheme to use in the garbled circuit.
	* @param otSender The OT object to use in the protocol.
	* @param inputForTest
	*/
	PartyTwo(CommParty * channel, OTBatchReceiver * otReceiver, GarbledBooleanCircuit * circuit) {
		this->channel = channel;
		this->otReceiver = otReceiver;
		this->circuit = circuit;
	};

	/**
	* Runs the protocol.
	* @param ungarbledInput The input for the circuit, each p1's input wire gets 0 or 1.
	*/
	void run(byte * ungarbledInput, int inputSize, bool print_output=false);
	/**
	* Receive the circuit's garbled tables and translation table.
	*/
	void receiveCircuit();
	/**
	* Receives party one input.
	*/
	void receiveP1Inputs();
	/**
	* Run OT protocol in order to get party two input without revealing any information.
	* @param sigmaArr Contains a byte indicates for each input wire which key to get.
	* @return The output from the OT protocol, party tw oinputs.
	*/
	shared_ptr<OTBatchROutput> runOTProtocol(byte* sigmaArr, int arrSize) {
		//Create an OT input object with the given sigmaArr.
		vector<byte> sigma;
		copy_byte_array_to_byte_vector(sigmaArr, arrSize, sigma, 0);
		int elementSize = 128;
		OTBatchRInput * input = new OTExtensionGeneralRInput(sigma, elementSize);
		//Run the Ot protocol.
		return otReceiver->transfer(input);
	};
};