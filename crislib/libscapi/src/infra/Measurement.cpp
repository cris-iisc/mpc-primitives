//
// Created by liork on 17/09/17.
//

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

#include "../../include/infra/Measurement.hpp"

using namespace std;


Measurement::Measurement(Protocol &protocol)
{
    init(protocol);
}

Measurement::Measurement(Protocol &protocol, vector<string> names)
{
    init(protocol);
    init(names);
}

void Measurement::setTaskNames(vector<string> & names)
{
    init(names);
}

void Measurement::init(Protocol &protocol)
{
    map<string, string> arguments = protocol.getArguments();
    m_protocolName = arguments["protocolName"];
    m_numberOfIterations = stoi(arguments["internalIterationsNumber"]);
    map<string, string>::iterator it = arguments.find("partyID");
    if(it != arguments.end())
    {
        m_partyId =  stoi(arguments["partyID"]);
    }

    string partiesFile = arguments["partiesFile"];
    m_numOfParties = atoi(arguments["partiesNumber"].c_str());
    setCommInterface(partiesFile);
}

void Measurement::init(vector <string> names)
{
    m_cpuStartTimes = new vector<vector<long>>(names.size(), vector<long>(m_numberOfIterations));
    m_commSentStartTimes = new vector<vector<unsigned long int>>(names.size(),
            vector<unsigned long int>(m_numberOfIterations));
    m_commReceivedStartTimes = new vector<vector<unsigned long int>>(names.size(),
            vector<unsigned long int>(m_numberOfIterations));
    m_memoryUsage = new vector<vector<long>>(names.size(), vector<long>(m_numberOfIterations));
    m_cpuEndTimes = new vector<vector<long>>(names.size(), vector<long>(m_numberOfIterations));
    m_commSentEndTimes = new vector<vector<unsigned long int>>(names.size(),
            vector<unsigned long int>(m_numberOfIterations));
    m_commReceivedEndTimes = new vector<vector<unsigned long int>>(names.size(),
            vector<unsigned long int>(m_numberOfIterations));
    m_names = move(names);
}


int Measurement::getTaskIdx(string name)
{
    auto it = std::find(m_names.begin(), m_names.end(), name);
    auto idx = distance(m_names.begin(), it);
    return idx;
}

void Measurement::setCommInterface(string partiesFile)
{
    ConfigFile cf(partiesFile);
    string ipPattern = "party_0_ip";
    string ip = cf.Value("", ipPattern);

    //define addresses prefixes
    string localPrefix = "127.0";
    string awsPrefix = "172.0";
    string serversPrefix = "10.0";

    if(ip.find(localPrefix) != string::npos)
        m_interface = "lo";
    else if (ip.find(serversPrefix) != string::npos)
        m_interface = "eth0";
    else
        m_interface = "ens3";
}

void Measurement::startSubTask(string taskName, int currentIterationNum)
{
    //calculate cpu start time
    auto now = system_clock::now();

    //Cast the time point to ms, then get its duration, then get the duration's count.
    auto ms = time_point_cast<milliseconds>(now).time_since_epoch().count();

    int taskIdx = getTaskIdx(taskName);

    (*m_cpuStartTimes)[taskIdx][currentIterationNum] = ms;
    tuple<unsigned long int, unsigned long int> startData = commData(m_interface.c_str());
    (*m_commSentStartTimes)[taskIdx][currentIterationNum] = get<0>(startData);
    (*m_commReceivedStartTimes)[taskIdx][currentIterationNum] = get<1>(startData);
}

void Measurement::endSubTask(string taskName, int currentIterationNum)
{
    int taskIdx = getTaskIdx(taskName);
    struct rusage r_usage;
    getrusage(RUSAGE_SELF, &r_usage);
    (*m_memoryUsage)[taskIdx][currentIterationNum] = r_usage.ru_maxrss;

    auto now = system_clock::now();
    //Cast the time point to ms, then get its duration, then get the duration's count.
    auto ms = time_point_cast<milliseconds>(now).time_since_epoch().count();
    (*m_cpuEndTimes)[taskIdx][currentIterationNum] = ms - (*m_cpuStartTimes)[taskIdx][currentIterationNum];

    tuple<unsigned long int, unsigned long int> endData = commData(m_interface.c_str());
    (*m_commSentEndTimes)[taskIdx][currentIterationNum] = get<0>(endData) -
            (*m_commSentStartTimes)[taskIdx][currentIterationNum];
    (*m_commReceivedEndTimes)[taskIdx][currentIterationNum] = get<1>(endData) -
            (*m_commReceivedStartTimes)[taskIdx][currentIterationNum];
}

tuple<unsigned long int, unsigned long int> Measurement::commData(const char * nic_)
{
	unsigned long rbytes = 0, tbytes = 0;
	std::string nic = nic_;
	FILE * pf = fopen("/proc/net/dev", "r");
	if(NULL != pf)
	{
		char sz[2048];
		while(NULL != fgets(sz, 2048, pf))
		{
			std::string line = sz;
            while(isspace(line[0])) line.erase(0, 1);
			std::string::size_type i = line.find(nic);
			if(std::string::npos != i)
			{
				line = line.substr(nic.size() + 1); //+1 for colon ':'
				for(int j = 0; j < 9; j++)
				{
					while(isspace(line[0])) line.erase(0, 1);
					i = line.find_first_of(" \f\n\r\t\v");
					if(std::string::npos == i) break;

					if(j == 0) //rbytes
					{
						rbytes = strtol(line.substr(0, i).c_str(), NULL, 10);
					}
					else if(j == 8)//tbytes
					{
						tbytes = strtol(line.substr(0, i).c_str(), NULL, 10);
					}
					line = line.substr(i);
				}
                break;
			}
		}
		fclose(pf);
	}
	return make_tuple(tbytes, rbytes);
}


void Measurement::analyze(string type)
{
    string filePath = getcwdStr();
    string fileName = filePath + "/" + m_protocolName + "_" + type + "_partyId=" + to_string(m_partyId)
                      +"_numOfParties=" + to_string(m_numOfParties) + ".json";

    //party is the root of the json objects
    json party = json::array();

    for (int taskNameIdx = 0; taskNameIdx < m_names.size(); taskNameIdx++)
    {
        //Write for each task name all the iteration
        json task = json::object();
        task["name"] = m_names[taskNameIdx];

        for (int iterationIdx = 0; iterationIdx < m_numberOfIterations; iterationIdx++)
        {
            if(type.compare("cpu") == 0)
                task["iteration_" + to_string(iterationIdx)] = (*m_cpuEndTimes)[taskNameIdx][iterationIdx];
            else if (type.compare("commSent") == 0)
                task["iteration_" + to_string(iterationIdx)] = (*m_commSentEndTimes)[taskNameIdx][iterationIdx];
            else if (type.compare("commReceived") == 0)
                task["iteration_" + to_string(iterationIdx)] = (*m_commReceivedStartTimes)[taskNameIdx][iterationIdx];
            else if (type.compare("memory") == 0)
                task["iteration_" + to_string(iterationIdx)] = (*m_memoryUsage)[taskNameIdx][iterationIdx];
        }
        party.insert(party.begin(), task);
    }

    //send json object to create file
    createJsonFile(party, fileName);
}


void Measurement::analyzeCpuData()
{
    analyze("cpu");
}

void Measurement::analyzeCommSentData()
{
    analyze("commSent");
}

void Measurement::analyzeCommReceivedData()
{
    analyze("commReceived");
}

void Measurement::analyzeMemory()
{
    analyze("memory");
}

void Measurement::createJsonFile(json j, string fileName)
{
    try
    {
        ofstream myfile (fileName, ostream::out);
        myfile << j;
    }

    catch (exception& e)
    {
        cout << "Exception thrown : " << e.what() << endl;
    }
}


Measurement::~Measurement()
{
    analyzeCpuData();
    analyzeCommSentData();
    analyzeCommReceivedData();
    analyzeMemory();
}

