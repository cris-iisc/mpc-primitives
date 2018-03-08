//
// Created by moriya on 13/11/17.
//

#include "../../include/interactive_mid_protocols/OTExtensionLibote.hpp"

OTExtensionLiboteSender::OTExtensionLiboteSender(string address, int port, bool isSemiHonest, bool isCorrelated, CommParty* channel): channel(channel), isSemiHonest(isSemiHonest){

    //connect ot channel
    ep = new osuCrypto::Endpoint(ios_ot, address, port, osuCrypto::EpMode::Server, "ep");
    otChannel = ep->addChannel("chl0", "chl0");

    auto key = prg.generateKey(128);
    prg.setKey(key);

    osuCrypto::PRNG prng(prg.getRandom128());

    vector<block> baseRecv(128);

    auto sigma = prg.getRandom128();
    osuCrypto::BitVector baseChoice((byte*)&sigma, 128);

    osuCrypto::NaorPinkas base;
    base.receive(baseChoice, baseRecv,prng, otChannel, 2);

    if (isSemiHonest){
        if (isCorrelated){
            sender = new IknpDotExtSender();
        } else {
            sender = new IknpOtExtSender();
        }
    } else {
        if (isCorrelated){
            sender = new KosDotExtSender();
        } else {
            sender = new KosOtExtSender();
        }
    }
    sender->setBaseOts(baseRecv, baseChoice);

}

OTExtensionLiboteSender::~OTExtensionLiboteSender(){

    otChannel.close();
    ep->stop();
    ios_ot.stop();
    delete ep;

    delete sender;
}

void expandOutput(int elementSize, byte * key, vector<byte> & output, int factor, const byte *counters,
                  EVP_CIPHER_CTX *aes, byte * aesOutput, int i) {
    //init the aes prp with the given key
    EVP_CIPHER_CTX_init(aes);
    EVP_EncryptInit(aes, EVP_aes_128_ecb(), key, nullptr);

    //Compute AES on the counters array
    int outLength;
    EVP_EncryptUpdate(aes, aesOutput, &outLength, (byte *)counters, factor * 16);

    //Copy the result to the given location
    memcpy(output.data() + i*elementSize/8, aesOutput, elementSize/8);

    //Cleaning the environment before setting the next key.
    EVP_CIPHER_CTX_cleanup(aes);
}

byte* createCountersArray(int factor) {//Create the array of indices to be the plaintext for the AES
    auto counters = new byte[factor * 16];
    //assign zero to the array of indices which are set as the plaintext.
    memset(counters, 0, 16 * factor);
    long *countersArray = (long *)counters;
    //go over the array and set the 64 list significant bits for evey 128 bit value, we use only half of the 128 bit variables
    for (long i = 0; i < factor; i++) {
        countersArray[i * 2 + 1] = i;
    }
    return counters;
}

shared_ptr<OTBatchSOutput> OTExtensionLiboteSender::transfer(OTBatchSInput * input){
//    auto start = scapi_now();
    osuCrypto::PRNG prng(prg.getRandom128());
//    print_elapsed_ms(start, "create prng");
//    sender.send(q, prng, otChannel);

    if(input->getType()!= OTBatchSInputTypes::OTExtensionRandomizedSInput &&
	   input->getType()!= OTBatchSInputTypes::OTExtensionGeneralSInput &&
	   input->getType()!= OTBatchSInputTypes::OTExtensionCorrelatedSInput){
		throw invalid_argument("input should be instance of OTExtensionRandomizedSInput or OTExtensionGeneralSInput or OTExtensionCorrelatedSInput.");
	} else {
        int nOTsReal, elementSize;

        //get the number OTs and element size from the input object.
        //need to cast to the right class according to the type.
        if (input->getType() == OTBatchSInputTypes::OTExtensionGeneralSInput) {

            nOTsReal = ((OTExtensionGeneralSInput *) input)->getNumOfOts();
            elementSize = 8 * (((OTExtensionGeneralSInput *) input)->getX0Arr().size() / nOTsReal);

        } else if (input->getType() == OTBatchSInputTypes::OTExtensionRandomizedSInput) {

            nOTsReal = ((OTExtensionRandomizedSInput *) input)->getNumOfOts();
            elementSize = ((OTExtensionRandomizedSInput *) input)->getElementSize();

        } else {

            nOTsReal = ((OTExtensionCorrelatedSInput *) input)->getNumOfOts();
            elementSize = 8 * (((OTExtensionCorrelatedSInput *) input)->getDeltaArr().size());
            block delta;
            memcpy((byte*)&delta, (((OTExtensionCorrelatedSInput *) input)->getDeltaArr().data()), 16);
            if (isSemiHonest) {
                ((IknpDotExtSender*)sender)->setDelta(delta);
            } else {
                ((KosDotExtSender*)sender)->setDelta(delta);
            }
        }
        vector<array<block, 2>> q(nOTsReal);
//        start = scapi_now();

        sender->send(q, prng, otChannel);

//        print_elapsed_ms(start, "send");
//        cout<<"element size = "<<elementSize<<endl;
//        for (int i = 0; i < nOTsReal; i++) {
//            cout << "ot no. " << i << ":" << endl;
//            for (int j = 0; j < 16; j++) {
//                cout << (int) ((byte *) (&q[i][0]))[j] << " ";
//            }
//            cout << endl;
//            for (int j = 0; j < 16; j++) {
//                cout << (int) ((byte *) (&q[i][1]))[j] << " ";
//            }
//            cout << endl;
//        }

//        start = scapi_now();
        // Convert the result of the transfer function to the required size
        int size = elementSize / 8 * nOTsReal;
        vector<byte> aesX0(size);
        vector<byte> aesX1(size);

        //Copy only the required OTs.
        if (elementSize <= 128) {

            for (int i = 0; i < nOTsReal; i++) {
                memcpy(aesX0.data() + i * elementSize / 8, (byte *) &q[i][0], elementSize / 8);
                memcpy(aesX1.data() + i * elementSize / 8, (byte *) &q[i][1], elementSize / 8);
            }

        //The required size is bigger than the output. Expand x0 and x1.
        } else {

            //number of 128 bit ot's needed.
            int factor = (elementSize + 127) / 128;
            byte * counters = createCountersArray(factor);

            auto aes = new EVP_CIPHER_CTX();
            auto output = new byte[factor * 16];
            for (int i=0; i < nOTsReal; i++){
                //Expand the x0[i] to the required size.
                //Put the result in x0.
                expandOutput(elementSize, (byte*) &q[i][0], aesX0, factor, counters, aes, output, i);

                //Expand the x0[i] to the required size.
                //Put the result in x0.
                expandOutput(elementSize, (byte*) &q[i][1], aesX1, factor, counters, aes, output, i);
            }
            delete [] counters;
            delete [] output;
            delete aes;
        }
//        print_elapsed_ms(start, "prepare output");

        //If this if the general case we need another round of communication using the channel member, since OT bristol only works on random case.
        if(input->getType()== OTBatchSInputTypes::OTExtensionGeneralSInput) {

            if (channel == NULL) {
                throw runtime_error("In order to execute a general ot extension the channel must be given");
            }

            auto x0Vec = ((OTExtensionGeneralSInput *) input)->getX0Arr();
            auto x1Vec = ((OTExtensionGeneralSInput *) input)->getX1Arr();

            //Xor the given x0 and x1 with the OT output
            for (int i = 0; i < size; i++) {

                aesX0[i] = x0Vec[i] ^ aesX0[i];
                aesX1[i] = x1Vec[i] ^ aesX1[i];
            }

            //send the xored arrrays over the channel.
            channel->write(aesX0.data(), size);
            channel->write(aesX1.data(), size);

            return nullptr;

        //If this if the correlated case we need another round of communication using the channel member, since OT bristol only works on random case.
        } /*else if (input->getType()== OTBatchSInputTypes::OTExtensionCorrelatedSInput){

			if (channel == NULL) {
				throw runtime_error("In order to execute a correlated ot extension the channel must be given");
			}

			auto delta = ((OTExtensionCorrelatedSInput *)input)->getDeltaArr();

            vector<byte> newX1(size);
            byte* newDelta = new byte[size];

            //X0 = x0
            //X1 - delta ^ x0
			for(int i=0; i<size; i++){

                newX1[i] = delta[i] ^ aesX0[i];

				//we use delta in order not to create an additional array
                newDelta[i] =  newX1[i] ^ aesX1[i];
			}

			//send R1^R0^delta over the channel.
			channel->write(newDelta, size);
            delete [] newDelta;

			//the output for the sender is x0 and x0^delta
			return make_shared<OTExtensionCorrelatedSOutput>(aesX0, newX1);

		}
*/
		else{
			//return a shared pointer of the output as it taken from the ot object of the library
			return make_shared<OTExtensionBristolRandomizedSOutput>(aesX0, aesX1);
		}

	}
}

OTExtensionLiboteReceiver::OTExtensionLiboteReceiver(string address, int port, bool isSemiHonest, bool isCorrelated, CommParty* channel): channel(channel) {

    //connect ot channel
    ep = new osuCrypto::Endpoint(ios_ot, address, port, osuCrypto::EpMode::Client, "ep");
    otChannel = ep->addChannel("chl0", "chl0");
    auto key = prg.generateKey(128);
    prg.setKey(key);

    osuCrypto::PRNG prng(prg.getRandom128());

    std::vector<std::array<block, 2>> baseSend(128);

    osuCrypto::NaorPinkas base;
    base.send(baseSend, prng, otChannel, 2);

    if (isSemiHonest){
        if (isCorrelated){
            receiver = new IknpDotExtReceiver();
        } else {
            receiver = new IknpOtExtReceiver();
        }
    } else {
        if (isCorrelated){
            receiver = new KosDotExtReceiver();
        } else {
            receiver = new KosOtExtReceiver();
        }
    }
    receiver->setBaseOts(baseSend);
}

OTExtensionLiboteReceiver::~OTExtensionLiboteReceiver(){

    otChannel.close();
    ep->stop();
    ios_ot.stop();
    delete ep;

    delete receiver;

}

shared_ptr<OTBatchROutput> OTExtensionLiboteReceiver::transfer(OTBatchRInput * input) {
    if (input->getType() != OTBatchRInputTypes::OTExtensionGeneralRInput &&
        input->getType() != OTBatchRInputTypes::OTExtensionRandomizedRInput &&
        input->getType() != OTBatchRInputTypes::OTExtensionCorrelatedRInput) {
        throw invalid_argument(
                "input should be instance of OTExtensionGeneralRInput or OTExtensionRandomizedRInput or OTExtensionCorrelatedRInput.");
    } else {

        osuCrypto::PRNG prng(prg.getRandom128());
        auto sigmaArr = ((OTExtensionRInput *) input)->getSigmaArr();

        auto nOTsReal = sigmaArr.size();
        vector<block> t(nOTsReal);

        osuCrypto::BitVector sigma(sigmaArr.size());
        for (int i = 0; i < sigmaArr.size(); i++) {
            sigma[i] = sigmaArr[i];
        }

        receiver->receive(sigma, t, prng, otChannel);

        auto elementSize = (((OTExtensionRInput *) input)->getElementSize());

        //Convert the result of the transfer function to the required size
        vector<byte> aesOutput(nOTsReal * elementSize / 8);

        //There is no need to change the element size, copy only the required OTs.
        if (elementSize == 128) {
            memcpy(aesOutput.data(), t.data(), nOTsReal * 16);
            //The required size is smaller than the output. Shrink it.
        } else if (elementSize < 128) {
            for (int i = 0; i < nOTsReal; i++) {
                memcpy(aesOutput.data() + i * elementSize / 8, (byte *) &t[i], elementSize / 8);
            }
        //The required size is bigger than the output. Expand it.
        } else {
            //number of 128 bit ot's needed.
            int factor = (elementSize + 127)/128;
            //Create the array of indices to be the plaintext for the AES
            auto counters = createCountersArray(factor);

            auto aes = new EVP_CIPHER_CTX();
            auto outputArr = new byte[16 * factor];

            for (int i=0; i < (int) nOTsReal; i++){

                //Expand the output to the required size.
                expandOutput(elementSize, (byte*) &t[i], aesOutput, factor, counters, aes, outputArr, i);
            }
            delete [] counters;
            delete [] outputArr;
            delete aes;
        }
        int size = elementSize/8 * nOTsReal;

        //If this if the general case we need another round of communication using the channel member, since OT bristol only works on random case.
        //we need to get the xor of the randomized and real data from the sender.
        if(input->getType() == OTBatchRInputTypes::OTExtensionGeneralRInput){

            if (channel == NULL) {
                throw runtime_error("In order to execute a general ot extension the channel must be given");
            }

            //Read x0 and x1 from the sender
            byte* x0Arr = new byte[size];
            byte* x1Arr = new byte[size];

            channel->read(x0Arr, size);
            channel->read(x1Arr, size);

            //xor each randomized output with the relevant xored sent from the sender
            for(int i=0; i < size; i++){
                if (sigmaArr[i/(elementSize/8)] == 0) {
                    aesOutput[i] = x0Arr[i] ^ aesOutput[i];
                } else {
                    aesOutput[i] = x1Arr[i] ^ aesOutput[i];
                }
            }

            delete [] x0Arr;
            delete [] x1Arr;

            return make_shared<OTOnByteArrayROutput>(aesOutput);
        }/*
            //If this if the correlated case we need another round of communication using the channel member, since OT bristol only works on random case.
            //we need to get the xor of the randomized and real data from the sender.
        else if(input->getType() == OTBatchRInputTypes::OTExtensionCorrelatedRInput){

            if (channel == NULL) {
                throw runtime_error("In order to execute a correlated ot extension the channel must be given");
            }

            byte* delta = new byte[size];

            channel->read(delta, size);

            //xor each randomized output with the relevant xored sent from the sender
            for(int i=0; i < size; i++){
                if (sigmaArr[i/(elementSize/8)] != 0) {
                    //aesOutput[i] = aesOutput[i];
                    // } else {
                    //x1 = delta^x1 = x1^ x0^delta^x1 = xo^delta=x1
                    aesOutput[i] = delta[i] ^ aesOutput[i];
                }
            }

            delete [] delta;

            return make_shared<OTOnByteArrayROutput>(aesOutput);
        } */else{
            return make_shared<OTOnByteArrayROutput>(aesOutput);
        }


    }
}