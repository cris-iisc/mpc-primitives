//
// Created by moriya on 24/09/17.
//
#include <../../include/cryptoInfra/Protocol.hpp>

string CmdParser::getKey(string parameter)
{
    if (parameter[0] == '-')
        return parameter.substr(1);
    else
        return parameter;
}

map<string, string> CmdParser::parseArguments(string protocolName, int argc, char* argv[])
{
    map<string, string> arguments;

    string key;

    //Put the protocol name in the map
    arguments["protocolName"] = protocolName;

    //Put all other parameters in the map
    for(int i=1; i<argc; i+=2)
    {

        key = getKey(string(argv[i]));
        arguments[key] = argv[i+1];

        cout<<"key = "<<key<<" value = "<<arguments[key]<<endl;
    }

    return arguments;
}

Protocol::Protocol(string protocolName, int argc, char* argv[])
{
    arguments = parser.parseArguments(protocolName, argc, argv);
}

map <string, string> Protocol::getArguments()
{
    return arguments;
}
