//
// Created by jorge on 10/24/25.
//

#include "BrokerServer.h"
#include "utils.h"

#define ERRORLOG(msg) std::cerr << "[ERROR] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;

BrokerServer::BrokerServer(const string &brokerIP, int brokerPort, const string &myIP, int myPort) {
    this->selfInfo.ip = myIP;
    this->selfInfo.port = myPort;
    this->brokerServerId = initClient(brokerIP, brokerPort).serverId;
}

bool BrokerServer::registrarEnBroker() {

    vector<unsigned char> buffer;

    pack(buffer, BrokerDeObjetos::REGISTRO_SERVIDOR);
    pack(buffer, selfInfo.ip.size());
    packv(buffer, selfInfo.ip.data(), selfInfo.ip.size());
    pack(buffer, selfInfo.port);


    sendMSG(this->brokerServerId, buffer);


    buffer.clear();
    recvMSG(this->brokerServerId, buffer);
    if ( unpack<BrokerDeObjetos::brokerMessageType>(buffer) != BrokerDeObjetos::ack){
        ERRORLOG("No se ha recibido el ack del broker");
        return false;
    }

    return true;
}

void BrokerServer::desregistrarDeBroker() {
    vector<unsigned char> buffer;

    pack(buffer, BrokerDeObjetos::DESCONEXION_SERVIDOR);
    pack(buffer, selfInfo.ip.size());
    packv(buffer, selfInfo.ip.data(), selfInfo.ip.size());
    pack(buffer, selfInfo.port);

    sendMSG(this->brokerServerId, buffer);

    buffer.clear();
    recvMSG(this->brokerServerId, buffer);

    if (unpack<BrokerDeObjetos::brokerMessageType>(buffer) != BrokerDeObjetos::ack) {
        ERRORLOG("No se recibió ACK al desregistrar servidor");
    }

    closeConnection(this->brokerServerId);
}

BrokerServer::~BrokerServer() {
    desregistrarDeBroker();
}
