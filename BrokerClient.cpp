//
// Created by jorge on 10/24/25.
//

#include "BrokerClient.h"
#include "utils.h"
#include "BrokerDeObjetos.h"
#define ERRORLOG(msg) std::cerr << "[ERROR] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;

using namespace std;

BrokerClient::BrokerClient(const string &brokerAddress, int brokerPort) {
    this->brokerServerId = initClient(brokerAddress, brokerPort).serverId;
}

BrokerDeObjetos::IpPuerto BrokerClient::obtenerServidorDisponible() {

    BrokerDeObjetos::IpPuerto servidor = {};
    vector<unsigned char> buffer;

    pack(buffer, BrokerDeObjetos::PETICION_SERVIDOR);
    sendMSG(this->brokerServerId, buffer);

    buffer.clear();
    recvMSG(this->brokerServerId, buffer);

    // Leer el primer valor
    auto firstValue = unpack<BrokerDeObjetos::brokerMessageType>(buffer);

    // Verificar si es código de error
    if (firstValue == BrokerDeObjetos::ERROR_NO_SERVIDORES) {
        ERRORLOG("No hay servidores disponibles");
        servidor.port = -1;

        // Leer ACK antes de cerrar
        if (unpack<BrokerDeObjetos::brokerMessageType>(buffer) != BrokerDeObjetos::ack) {
            ERRORLOG("No se recibió ACK del broker");
        }

        closeConnection(brokerServerId);

        brokerServerId = -1;
        return servidor;
    }

    // Si no es error, entonces es el tamaño de la IP
    servidor.ip.resize(unpack<long int>(buffer));
    unpackv(buffer, servidor.ip.data(), servidor.ip.size());
    servidor.port = unpack<int>(buffer);

    // ACK
    if (unpack<BrokerDeObjetos::brokerMessageType>(buffer) != BrokerDeObjetos::ack) {
        ERRORLOG("No se ha recibido el ack del broker");
    }

    return servidor;
}



void BrokerClient::desconectarCliente() {
    vector<unsigned char> buffer;

    pack(buffer, BrokerDeObjetos::DESCONEXION_CLIENTE);
    sendMSG(this->brokerServerId, buffer);

    buffer.clear();
    recvMSG(this->brokerServerId, buffer);

    if (unpack<BrokerDeObjetos::brokerMessageType>(buffer) != BrokerDeObjetos::ack) {
        ERRORLOG("No se recibió ACK al desconectar cliente");
    }

    closeConnection(this->brokerServerId);
}


BrokerClient::~BrokerClient() {
    if (brokerServerId != -1)
        desconectarCliente();
}

