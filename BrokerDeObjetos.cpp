#include "BrokerDeObjetos.h"
#include "utils.h"

void BrokerDeObjetos::registrarServidor(const IpPuerto &servidor) {
    brokerMutex.lock();
    servidoresRegistrados.push_back(servidor);
    clientesEnServidor[servidor] = 0;
    brokerMutex.unlock();
}

BrokerDeObjetos::IpPuerto BrokerDeObjetos::obtenerServidorDisponible() {
    brokerMutex.lock();

    IpPuerto servidorConMenorCarga = *servidoresRegistrados.begin();
    int cargaMejorServidor = clientesEnServidor[servidorConMenorCarga];

    for (const auto &ipPuerto: servidoresRegistrados) {
        int cargaActual = clientesEnServidor[ipPuerto];

        if (cargaMejorServidor > cargaActual) {
            cargaMejorServidor = cargaActual;
            servidorConMenorCarga = ipPuerto;
        }
    }

    brokerMutex.unlock();
    return servidorConMenorCarga;
}

void BrokerDeObjetos::registrarConexionCliente(const IpPuerto &ipServidor, const int idCliente) {
    brokerMutex.lock();
    clientesEnServidor[ipServidor]++;
    servidorDeCliente[idCliente] = ipServidor;
    brokerMutex.unlock();
}

void BrokerDeObjetos::liberarConexionCliente(const int idCliente) {
    brokerMutex.lock();

    IpPuerto serverIP;

    // Buscar a qué servidor estaba conectado el cliente
    if (servidorDeCliente.find(idCliente) != servidorDeCliente.end()) {
        serverIP = servidorDeCliente[idCliente];
        servidorDeCliente.erase(idCliente);
    }

    if (clientesEnServidor.find(serverIP) != clientesEnServidor.end()) {
        clientesEnServidor[serverIP]--;

        // Si no quedan clientes, no dejar valor negativo
        if (clientesEnServidor[serverIP] < 0) {
            clientesEnServidor[serverIP] = 0;
        }
    }

    brokerMutex.unlock();
}


bool BrokerDeObjetos::hayServidoresDisponibles() {
    brokerMutex.lock();
    bool disponible = !servidoresRegistrados.empty();
    brokerMutex.unlock();
    return disponible;
}

void BrokerDeObjetos::resolverPeticion(int clientId) {
    vector<unsigned char> buffer;
    bool logOut = false;
    do {
        recvMSG(clientId, buffer);
        brokerMessageType type = unpack<brokerMessageType>(buffer);

        switch (type) {
            case REGISTRO_SERVIDOR: {
                std::cout << "Broker: REGISTRO_SERVIDOR para cliente " << clientId << std::endl;


                IpPuerto serverIP;
                serverIP.ip.resize(unpack<long int>(buffer));
                unpackv(buffer, serverIP.ip.data(), serverIP.ip.size());
                serverIP.port = unpack<int>(buffer);

                registrarServidor(serverIP);
            }
            break;
            case PETICION_SERVIDOR: {
                std::cout << "Broker: PETICION_SERVIDOR para cliente " << clientId << std::endl;

                buffer.clear();

                if (hayServidoresDisponibles()) {
                    IpPuerto serverIP = obtenerServidorDisponible();

                    pack(buffer, PETICION_SERVIDOR);
                    pack(buffer, serverIP.ip.size());
                    packv(buffer, serverIP.ip.data(), serverIP.ip.size());
                    pack(buffer, serverIP.port);

                    registrarConexionCliente(serverIP, clientId);
                } else {
                    pack(buffer, ERROR_NO_SERVIDORES);
                    logOut = true;
                }

            }
            break;
            case DESCONEXION_SERVIDOR: {
                std::cout << "Broker: DESCONEXION_SERVIDOR para cliente " << clientId << std::endl;

                IpPuerto serverIP;

                serverIP.ip.resize(unpack<long int>(buffer));
                unpackv(buffer, serverIP.ip.data(), serverIP.ip.size());
                serverIP.port = unpack<int>(buffer);

                desregistrarServidor(serverIP);
                logOut = true;
            }
            break;
            case DESCONEXION_CLIENTE: {
                std::cout << "Broker: DESCONEXION_CLIENTE para cliente " << clientId << std::endl;
                liberarConexionCliente(clientId);
                logOut = true;
            }
            break;
            default:
                std::cout << "Broker: Petición desconocida (" << type << ") para cliente " << clientId << std::endl;
                break;
        }

        pack(buffer, ack);
        sendMSG(clientId, buffer);
        buffer.clear();
    } while (!logOut);

    closeConnection(clientId);
}

void BrokerDeObjetos::desregistrarServidor(const IpPuerto &ipServidor) {
    brokerMutex.lock();

    clientesEnServidor.erase(ipServidor);

    for (int i = 0; i < servidoresRegistrados.size(); ++i) {
        if (servidoresRegistrados[i] == ipServidor) {
            servidoresRegistrados.erase(servidoresRegistrados.begin() + i);
            break;
        }
    }

    brokerMutex.unlock();
}
