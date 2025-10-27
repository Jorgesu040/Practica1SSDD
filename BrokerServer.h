//
// Created by jorge on 10/24/25.
//

#ifndef BROKER_BROKERSERVER_H
#define BROKER_BROKERSERVER_H

#include <string>
#include "BrokerDeObjetos.h"

// BrokerServer.h - Para uso del servidor FileManager
class BrokerServer {
private:
    unsigned int brokerServerId; // ID de conexión al broker
    BrokerDeObjetos::IpPuerto selfInfo;    // IP y puerto servidor

public:
    BrokerServer(const string& brokerIP, int brokerPort,
                 const string& myIP, int myPort);
    ~BrokerServer(); // Desregistrarse al destruir

    // Registrarse en el broker
    bool registrarEnBroker();

    // Desregistrarse del broker (llamado en destructor)
    void desregistrarDeBroker();
};



#endif //BROKER_BROKERSERVER_H