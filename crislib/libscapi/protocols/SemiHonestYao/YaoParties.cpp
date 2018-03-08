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



#include "YaoParties.hpp"
#include <tuple>
#ifndef _WIN32
#include "../../include/circuits/Compat.h"
#endif

vector<byte> readInputAsVector(string input_file, int numInputs) {
	auto sc = scannerpp::Scanner(new scannerpp::File(input_file));
	vector<byte> inputVector(numInputs);
	for (int i = 0; i < numInputs; i++) {
		inputVector[i] = (byte) sc.nextInt();
	}
	return inputVector;
}

/*********************************/
/*          PartyOne             */
/*********************************/

PartyOne::PartyOne(int argc, char* argv[]) : Protocol("SemiHonestYao", argc, argv) {
	//t = boost::thread(boost::bind(&boost::asio::io_service::run, &io_service));

	id = stoi(arguments["partyID"]);

	YaoConfig yao_config(arguments["configFile"]);
	this->yaoConfig = yao_config;

	vector<string> subTaskNames{"Garble", "SendCircuitAndInputs", "OT"};
	timer = new Measurement(*this, subTaskNames);

	//open parties file
	ConfigFile cf(arguments["partiesFile"]);

	string receiver_ip, sender_ip;
	int receiver_port, sender_port;

	//get partys IPs and ports data
	sender_port = stoi(cf.Value("", "party_0_port"));
	sender_ip = cf.Value("", "party_0_ip");
	receiver_port = stoi(cf.Value("", "party_1_port"));
	receiver_ip = cf.Value("", "party_1_ip");

	cout<<"sender ip: "<<sender_ip <<"port:"<<sender_port<<endl;
	cout<<"receiver ip: "<<receiver_ip<<"port:"<<receiver_port<<endl;
	SocketPartyData me(IpAddress::from_string(sender_ip), sender_port++);

	SocketPartyData other(IpAddress::from_string(receiver_ip), receiver_port);
	cout<<"my ip: "<<me.getIpAddress() <<"port:"<<me.getPort()<<endl;
	cout<<"other ip: "<<other.getIpAddress() <<"port:"<<other.getPort()<<endl;
	channel = make_shared<CommPartyTCPSynced>(io_service, me, other);

	// create the garbled circuit
#ifdef NO_AESNI
    circuit = new GarbledBooleanCircuitNoIntrinsics(yao_config.circuit_file.c_str());
#else
    circuit = GarbledCircuitFactory::createCircuit(yao_config.circuit_file,
		                GarbledCircuitFactory::CircuitType::FIXED_KEY_FREE_XOR_HALF_GATES, false);
#endif

	setInputs(yao_config.input_file_1, circuit->getNumberOfInputs(1));
    // create the semi honest OT extension sender
	SocketPartyData senderParty(IpAddress::from_string(sender_ip), sender_port);
	cout<<"sender ip: "<<senderParty.getIpAddress() <<"port:"<<senderParty.getPort()<<endl;
#ifdef _WIN32
	otSender = new OTSemiHonestExtensionSender(senderParty, 163, 1);
#else

    #ifdef NO_AESNI
        otSender = new OTExtensionLiboteSender(sender_ip, senderParty.getPort(), true, false, channel.get());
    #else
        otSender = new OTExtensionBristolSender(senderParty.getPort(), true, channel);
    #endif
#endif

	// connect to party two
	channel->join(500, 5000);
};

void PartyOne::setInputs(string inputFileName, int numInputs) {
	ungarbledInput = readInputAsVector(inputFileName, numInputs);
}

void PartyOne::sendP1Inputs(byte* ungarbledInput) {
	byte* allInputs = (byte*)std::get<0>(values);
	// get the size of party one inputs
	int numberOfp1Inputs = 0;
	numberOfp1Inputs = circuit->getNumberOfInputs(1);
	int inputsSize = numberOfp1Inputs*KEY_SIZE;
	byte* p1Inputs = new byte[inputsSize];

	// create an array with the keys corresponding the given input.
	int inputStartIndex;
	for (int i = 0; i < numberOfp1Inputs; i++) {
		inputStartIndex = (2 * i + ((int) ungarbledInput[i]))*KEY_SIZE;
		memcpy(p1Inputs + i*KEY_SIZE, allInputs + inputStartIndex, KEY_SIZE);
	}
	// send the keys to p2.
	channel->write(p1Inputs, inputsSize);
	delete p1Inputs;
}

void PartyOne::run() {
	for (currentIteration = 0; currentIteration<yaoConfig.number_of_iterations; currentIteration++){
		runOnline();
	}

}

void PartyOne::runOnline() {
	timer->startSubTask("Garble", currentIteration);
	values = circuit->garble();
	timer->endSubTask("Garble", currentIteration);
	// send garbled tables and the translation table to p2.
	auto garbledTables = circuit->getGarbledTables();

	timer->startSubTask("SendCircuitAndInputs", currentIteration);
	channel->write((byte *) garbledTables, circuit->getGarbledTableSize());
	channel->write(circuit->getTranslationTable().data(), circuit->getNumberOfOutputs());
	// send p1 input keys to p2.
	sendP1Inputs(ungarbledInput.data());
	timer->endSubTask("SendCircuitAndInputs", currentIteration);

	// run OT protocol in order to send p2 the necessary keys without revealing any information.
	timer->startSubTask("OT", currentIteration);
	runOTProtocol();
	timer->endSubTask("OT", currentIteration);
}

void PartyOne::runOTProtocol() {
	//Get the indices of p2 input wires.
	int p1InputSize, p2InputSize;
	byte* allInputWireValues = (byte*)std::get<0>(values);
	p1InputSize = circuit->getNumberOfInputs(1);
	p2InputSize = circuit->getNumberOfInputs(2);
//    auto p2inputIndices = circuit->getInputWireIndices(2);
	vector<byte> x0Arr;
	x0Arr.reserve(p2InputSize * KEY_SIZE);
	vector<byte> x1Arr;
	x1Arr.reserve(p2InputSize * KEY_SIZE);
	int beginIndex0, beginIndex1;
	for (int i = 0; i<p2InputSize; i++) {
        beginIndex0 = p1InputSize * 2 * KEY_SIZE + 2 * i*KEY_SIZE;
        beginIndex1 = p1InputSize * 2 * KEY_SIZE + (2 * i + 1)*KEY_SIZE;

		x0Arr.insert(x0Arr.end(), &allInputWireValues[beginIndex0], &allInputWireValues[beginIndex0 + KEY_SIZE]);
		x1Arr.insert(x1Arr.end(), &allInputWireValues[beginIndex1], &allInputWireValues[beginIndex1 + KEY_SIZE]);
	}
	// create an OT input object with the keys arrays.
	OTBatchSInput * input = new OTExtensionGeneralSInput(x0Arr, x1Arr, p2InputSize);
	// run the OT's transfer phase.
	otSender->transfer(input);
}

/*********************************/
/*          PartyTwo             */
/*********************************/

PartyTwo::PartyTwo(int argc, char* argv[]) : Protocol("SemiHonestYao", argc, argv){

	id = stoi(arguments["partyID"]);

	YaoConfig yao_config(arguments["configFile"]);
	this->yaoConfig = yao_config;

	this->print_output = yaoConfig.print_output;

	//open parties file
	ConfigFile cf(arguments["partiesFile"]);

	vector<string> subTaskNames{"ReceiveCircuitAndInputs", "OT", "ComputeCircuit"};
	timer = new Measurement(*this, subTaskNames);
	string receiver_ip, sender_ip;
	int receiver_port, sender_port;

	//get partys IPs and ports data
	sender_port = stoi(cf.Value("", "party_0_port"));
	sender_ip = cf.Value("", "party_0_ip");
	receiver_port = stoi(cf.Value("", "party_1_port"));
	receiver_ip = cf.Value("", "party_1_ip");

	SocketPartyData me(IpAddress::from_string(receiver_ip), receiver_port);

	SocketPartyData other(IpAddress::from_string(sender_ip), sender_port++);
	channel = make_shared<CommPartyTCPSynced>(io_service, me, other);

	// create the garbled circuit
#ifdef NO_AESNI
    circuit = new GarbledBooleanCircuitNoIntrinsics(yao_config.circuit_file.c_str());
#else
    circuit = GarbledCircuitFactory::createCircuit(yao_config.circuit_file,
                                                   GarbledCircuitFactory::CircuitType::FIXED_KEY_FREE_XOR_HALF_GATES, false);
#endif
	setInputs(yao_config.input_file_2,  circuit->getNumberOfInputs(2));
	// create the OT receiver.
	SocketPartyData senderParty(IpAddress::from_string(sender_ip), sender_port);
#ifdef _WIN32
	otReceiver = new OTSemiHonestExtensionReceiver(senderParty, 163, 1);
#else

    #ifdef NO_AESNI
        otReceiver = new OTExtensionLiboteReceiver(sender_ip, sender_port, true, false, channel.get());
    #else
        otReceiver = new OTExtensionBristolReceiver(senderParty.getIpAddress().to_string(), senderParty.getPort(), true, channel);
    #endif

#endif


	// connect to party one
	channel->join(500, 5000);
}

void PartyTwo::setInputs(string inputFileName, int numInputs) {
	ungarbledInput = readInputAsVector(inputFileName, numInputs);
}

void PartyTwo::computeCircuit(OTBatchROutput * otOutput) {

	// Get the input of the protocol.
	vector<byte> p2Inputs = ((OTOnByteArrayROutput *)otOutput)->getXSigma();
	int p2InputsSize = ((OTOnByteArrayROutput *)otOutput)->getLength();
	// Get party two input wires' indices.
	vector<byte> allInputs(p1InputsSize + p2InputsSize);
    memcpy(&allInputs[0], p1Inputs, p1InputsSize);
	memcpy(&allInputs[p1InputsSize], p2Inputs.data(), p2InputsSize);

    // compute the circuit.

#ifdef NO_AESNI
    byte* garbledOutput = new byte[KEY_SIZE * 2 * circuit->getNumberOfOutputs()];
	circuit->compute(&allInputs[0], garbledOutput);
#else
    block* garbledOutput = (block *)_aligned_malloc(sizeof(block) * 2 * circuit->getNumberOfOutputs(), SIZE_OF_BLOCK); ;
    circuit->compute((block*)&allInputs[0], garbledOutput);
#endif

	// translate the result from compute.
	circuitOutput.resize(circuit->getNumberOfOutputs());
	circuit->translate(garbledOutput, circuitOutput.data());
}

void PartyTwo::run() {
	for (currentIteration = 0; currentIteration<yaoConfig.number_of_iterations; currentIteration++){
		runOnline();
	}
}

void PartyTwo::runOnline() {
	// receive tables and inputs
	timer->startSubTask("ReceiveCircuitAndInputs", currentIteration);
	receiveCircuit();
	receiveP1Inputs();
	timer->endSubTask("ReceiveCircuitAndInputs", currentIteration);

	timer->startSubTask("OT", currentIteration);
	// run OT protocol in order to get the necessary keys without revealing any information.
	auto output = runOTProtocol(ungarbledInput.data(), ungarbledInput.size());
	timer->endSubTask("OT", currentIteration);

	// Compute the circuit.
	timer->startSubTask("ComputeCircuit", currentIteration);
	computeCircuit(output.get());
	timer->endSubTask("ComputeCircuit", currentIteration);

	// we're done print the output
	if (print_output)
	{
		int outputSize = circuit->getNumberOfOutputs();
		cout << "PartyTwo: printing outputSize: " << outputSize << endl;
		for (int i = 0; i < outputSize; i++)
			cout << (int)circuitOutput[i];
		cout << endl;
	}
}

void PartyTwo::receiveCircuit() {

	// receive garbled tables.
	channel->read((byte*)circuit->getGarbledTables(), circuit->getGarbledTableSize());
	byte * translationTable = new byte[circuit->getNumberOfOutputs()];

	// receive translation table.
	channel->read(translationTable, circuit->getNumberOfOutputs());
	std::vector<byte> translationTableVec(translationTable, translationTable + circuit->getNumberOfOutputs());
	
	// set garbled tables and translation table to the circuit.

	//TODO MEITAL remove after the comm layer changes
	//block* garbledTabledAligned = (block *)_aligned_malloc(circuit->getGarbledTableSize(), SIZE_OF_BLOCK);

	//copy to the aligned memory
	//memcpy(garbledTabledAligned, garbledTables, circuit->getGarbledTableSize());

	//circuit->setGarbledTables((block*)garbledTables);
	circuit->setTranslationTable(translationTableVec);
}

void PartyTwo::receiveP1Inputs() {
	p1InputsSize = circuit->getNumberOfInputs(1)*KEY_SIZE;
	p1Inputs = new byte[p1InputsSize];
	channel->read(p1Inputs, p1InputsSize);
}
