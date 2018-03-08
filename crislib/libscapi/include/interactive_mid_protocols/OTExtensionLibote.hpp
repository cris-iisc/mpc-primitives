//
// Created by moriya on 13/11/17.
//

#ifndef SCAPI_OTEXTENSIONLIBOTE_H
#define SCAPI_OTEXTENSIONLIBOTE_H

#include "OTBatch.hpp"
#include "../primitives/Prg.hpp"
#include <libOTe/cryptoTools/cryptoTools/Network/IOService.h>
#include <libOTe/cryptoTools/cryptoTools/Network/Channel.h>
#include <libOTe/cryptoTools/cryptoTools/Network/Endpoint.h>
#include <libOTe/libOTe/TwoChooseOne/IknpOtExtReceiver.h>
#include <libOTe/libOTe/TwoChooseOne/IknpOtExtSender.h>
#include <libOTe/libOTe/TwoChooseOne/KosOtExtReceiver.h>
#include <libOTe/libOTe/TwoChooseOne/KosOtExtSender.h>
#include <libOTe/libOTe/TwoChooseOne/IknpDotExtReceiver.h>
#include <libOTe/libOTe/TwoChooseOne/IknpDotExtSender.h>
#include <libOTe/libOTe/TwoChooseOne/KosDotExtReceiver.h>
#include <libOTe/libOTe/TwoChooseOne/KosDotExtSender.h>
#include <libOTe/libOTe/Base/naor-pinkas.h>

using namespace osuCrypto;


class OTExtensionLiboteSender : public OTBatchSender {
private:
    osuCrypto::IOService ios_ot;             //used in LibOTe communication
    PrgFromOpenSSLAES prg;
    OtExtSender* sender;
    CommParty* channel;              //this is a shared pointer for the general case where there is one communication between the
								                //sender and the receiver after the libOTe ot extension is done in providing random x0 and x1 for the sender.
    //ot channel
    osuCrypto::Endpoint* ep;
    osuCrypto::Channel otChannel;

    bool isSemiHonest;
public:

    OTExtensionLiboteSender(string address, int port, bool isSemiHonest, bool isCorrelated, CommParty* channel);
    ~OTExtensionLiboteSender();

    shared_ptr<OTBatchSOutput> transfer(OTBatchSInput * input);

};

class OTExtensionLiboteReceiver : public OTBatchReceiver {
private:
    osuCrypto::IOService ios_ot;             //used in LibOTe communication

    //ot channel
    osuCrypto::Endpoint* ep;
    osuCrypto::Channel otChannel;
    CommParty* channel;              //this is a shared pointer for the general case where there is one communication between the
								                //sender and the receiver after the libOTe ot extension is done in providing random x0 and x1 for the sender.

    PrgFromOpenSSLAES prg;

    OtExtReceiver* receiver;
public:

    OTExtensionLiboteReceiver(string address, int port, bool isSemiHonest, bool isCorrelated, CommParty* channel);
    ~OTExtensionLiboteReceiver();

    shared_ptr<OTBatchROutput> transfer(OTBatchRInput * input);

};
#endif //SCAPI_OTEXTENSIONLIBOTE_H
