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


#include "YaoExample.hpp"
#include "../../include/circuits/GarbledCircuitFactory.hpp"

struct YaoConfig {
	int number_of_iterations;
	bool print_output;
	string circuit_type;
	string circuit_file;
	string input_file_1;
	string input_file_2;
	IpAddress sender_ip;
	IpAddress receiver_ip;
	YaoConfig(int n_iterations, bool print, string c_file, string input_file_1,
		string input_file_2, string sender_ip_str, string rec_ip_str, string circuit_type) {
		number_of_iterations = n_iterations;
		print_output = print;
		circuit_file = c_file;
		this->input_file_1 = input_file_1;
		this->input_file_2 = input_file_2;
		sender_ip = IpAddress::from_string(sender_ip_str);
		receiver_ip = IpAddress::from_string(rec_ip_str);
		this->circuit_type = circuit_type;
	};
};


vector<byte> * readInputAsVector(string input_file) {
	cout << "reading from file " << input_file << endl;;
	auto sc = scannerpp::Scanner(new scannerpp::File(input_file));
	int inputsNumber = sc.nextInt();
	auto inputVector = new vector<byte>(inputsNumber);
	for (int i = 0; i < inputsNumber; i++)
		(*inputVector)[i] = (byte)sc.nextInt();
	return inputVector;
}

GarbledBooleanCircuit * create_circuit(YaoConfig yao_config) {
	return GarbledCircuitFactory::createCircuit(yao_config.circuit_file, 
		GarbledCircuitFactory::CircuitType::FIXED_KEY_FREE_XOR_HALF_GATES, false);
}

void execute_party_one(YaoConfig yao_config) {
	auto start = scapi_now();
	boost::asio::io_service io_service;
	SocketPartyData me(yao_config.sender_ip, 1213);
	SocketPartyData other(yao_config.receiver_ip, 1212);
	shared_ptr<CommParty> channel = make_shared<CommPartyTCPSynced>(io_service, me, other);
	boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
	print_elapsed_ms(start, "PartyOne: Init");
	
	// create the garbled circuit
	start = chrono::system_clock::now();
	//FastGarbledBooleanCircuit * circuit = create_circuit(yao_config);

	GarbledBooleanCircuit* circuit = GarbledCircuitFactory::createCircuit(yao_config.circuit_file, 
		GarbledCircuitFactory::CircuitType::FIXED_KEY_FREE_XOR_HALF_GATES, false);

	print_elapsed_ms(start, "PartyOne: Creating GarbledBooleanCircuit");

	// create the semi honest OT extension sender
	SocketPartyData senderParty(yao_config.sender_ip, 7766);
	OTBatchSender * otSender = new OTExtensionBristolSender(senderParty.getPort(), true, channel);
	
	// connect to party two
	channel->join(500, 5000);
	
	// get the inputs of P1 
	vector<byte>* ungarbledInput = readInputAsVector(yao_config.input_file_1);

	PartyOne * p1;
	auto all = scapi_now();

	cout << "after reading input";
	// create Party one with the previous created objects.
	p1 = new PartyOne(channel.get(), otSender, circuit);
	for (int i = 0; i < yao_config.number_of_iterations ; i++) {
		// run Party one
		p1->run(ungarbledInput->data());
	}
	auto end = std::chrono::system_clock::now();
	int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - all).count();
	cout << "********************* PartyOne ********\nRunning " << yao_config.number_of_iterations <<
		" iterations took: " << elapsed_ms << " milliseconds" << endl 
		<< "Average time per iteration: " << elapsed_ms / (float)yao_config.number_of_iterations << " milliseconds" << endl;
	
	// exit cleanly
	delete p1;
    delete circuit;
    delete otSender;
    delete ungarbledInput;
	io_service.stop();
	t.join();
}

void execute_party_two(YaoConfig yao_config) {
	// init
	auto start = scapi_now();
	boost::asio::io_service io_service;
	SocketPartyData me(yao_config.receiver_ip, 1212);
	SocketPartyData other(yao_config.sender_ip, 1213);
	//SocketPartyData receiverParty(yao_config.receiver_ip, 7766);
	shared_ptr<CommParty> channel = make_shared<CommPartyTCPSynced>(io_service, me, other);
	boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
	print_elapsed_ms(start, "PartyTwo: Init");

	// create the garbled circuit
	start = scapi_now();
	GarbledBooleanCircuit * circuit = create_circuit(yao_config);
	print_elapsed_ms(start, "PartyTwo: creating FastGarbledBooleanCircuit");



	// create the OT receiver.
	start = scapi_now();
	SocketPartyData senderParty(yao_config.sender_ip, 7766);
	
	OTBatchReceiver * otReceiver = new OTExtensionBristolReceiver(senderParty.getIpAddress().to_string(), senderParty.getPort(), true, channel);
	print_elapsed_ms(start, "PartyTwo: creating OTSemiHonestExtensionReceiver");

	// connect to party one
	channel->join(500, 5000);


	// create Party two with the previous created objec ts			
	vector<byte> * ungarbledInput = readInputAsVector(yao_config.input_file_2);

	cout << "after reading input";

	PartyTwo * p2;
	auto all = scapi_now();
	p2 = new PartyTwo(channel.get(), otReceiver, circuit);
	for (int i = 0; i < yao_config.number_of_iterations ; i++) {
		// init the P1 yao protocol and run party two of Yao protocol.
		p2->run(&(ungarbledInput->at(0)), ungarbledInput->size(), yao_config.print_output);
	}
	auto end = std::chrono::system_clock::now();
	int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - all).count();
	cout << "********************* PartyTwo ********\nRunning " << yao_config.number_of_iterations <<
		" iterations took: " << elapsed_ms << " milliseconds" << endl;
	cout << "Average time per iteration: " << elapsed_ms / (float)yao_config.number_of_iterations 
		<< " milliseconds" << endl;
	delete p2;
    delete circuit;
    delete otReceiver;
    delete ungarbledInput;
	io_service.stop();
	t.join();
}

YaoConfig read_yao_config(string config_file) {
#ifdef _WIN32
	string os = "Windows";
#else
	string os = "Linux";
#endif
	ConfigFile cf(config_file);
	int number_of_iterations = stoi(cf.Value("", "number_of_iterations"));
	string str_print_output = cf.Value("", "print_output");
	bool print_output;
	istringstream(str_print_output) >> std::boolalpha >> print_output;
	string input_section = cf.Value("", "input_section") + "-" + os;
	string circuit_file = cf.Value(input_section, "circuit_file");
	string input_file_1 = cf.Value(input_section, "input_file_party_1");
	string input_file_2 = cf.Value(input_section, "input_file_party_2");
	string sender_ip_str = cf.Value("", "sender_ip");
	string receiver_ip_str = cf.Value("", "receiver_ip");
	string circuit_type = cf.Value("", "circuit_type");
	return YaoConfig(number_of_iterations, print_output, circuit_file, input_file_1,
		input_file_2, sender_ip_str, receiver_ip_str, circuit_type);
}

int mainYao(string partyNum, string configPath) {
	YaoConfig yao_config = read_yao_config(configPath); 
	if (partyNum == "1")
		execute_party_one(yao_config);
	else if (partyNum == "2") 
		execute_party_two(yao_config);
	else {
		std::cerr << "Usage: libscapi_examples yao <party_number(1|2)> <config_path>" << std::endl;
		return 1;
	}

	//system("pause");
	return 0;
}

