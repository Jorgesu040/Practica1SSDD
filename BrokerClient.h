//
// Created by jorge on 10/24/25.
//
#ifndef BROKER_BROKERCLIENT_H
#define BROKER_BROKERCLIENT_H

#include <string>
#include "BrokerDeObjetos.h"

using namespace std;

class BrokerClient {
    int brokerServerId;
    string myIp;
    int myPort = -1;

public:
    BrokerClient(const string& brokerAddress, int brokerPort, const string &myIp, int myPort);

    BrokerDeObjetos::IpPuerto obtenerServidorDisponible();

    // Avisar al broker de que se ha desconectado el cliente
    void desconectarCliente();

    ~BrokerClient();
};


#endif //BROKER_BROKERCLIENT_H