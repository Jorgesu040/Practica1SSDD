#ifndef BROKERDEOBJETOS_H
#define BROKERDEOBJETOS_H

#include <iostream>
#include <map>
#include <vector>
#include <mutex>

using namespace std;

class BrokerDeObjetos {

public:
    typedef enum {
        REGISTRO_SERVIDOR, // Servidor se registra en el broker
        PETICION_SERVIDOR, // Cliente pide IP de un servidor
        RESPUESTA_SERVIDOR, // Broker envía IP de servidor al cliente
        DESCONEXION_SERVIDOR, // Servidor se va a desconectar
        DESCONEXION_CLIENTE, // Cliente se desconecta
        ack = 5, // Confirmación
        ERROR_NO_SERVIDORES // No hay servidores disponibles
    } brokerMessageType;

    struct IpPuerto {
        string ip;
        int port;

        // Comparadores para el uso de std::map
        bool operator<(const IpPuerto &other) const {
            if (ip != other.ip) return ip < other.ip;
            return port < other.port;
        }

        bool operator==(const IpPuerto &other) const {
            return ip == other.ip && port == other.port;
        }
    };

private:
    // Lista de servidores disponibles (orden de registro)
    static inline vector<IpPuerto> servidoresRegistrados;

    // Mapa: IP_Servidor -> clientIDs
    static inline map<IpPuerto, int> clientesEnServidor;

    // Mapa: clientID -> IP_Servidor:Puerto_Servidor (para saber a qué servidor está conectado)
    static inline map<int, IpPuerto> servidorDeCliente;

    static inline mutex brokerMutex;

public:
    // Registrar un nuevo servidor FileManager
    static void registrarServidor(const IpPuerto &servidor);

    // Obtener IP del servidor con menos conexiones
    static IpPuerto obtenerServidorDisponible();

    // Incrementar contador cuando cliente se conecta a un servidor
    static void registrarConexionCliente(const IpPuerto &ipServidor, int idCliente);

    // Decrementar contador cuando cliente se desconecta
    static void liberarConexionCliente(int idCliente);

    // Verificar si hay servidores disponibles
    static bool hayServidoresDisponibles();

    // ===== MÉTODOS AUXILIARES =====

    // Resolver petición de cliente o servidor
    static void resolverPeticion(int clientId);

    // Eliminar servidor si se desconecta
    static void desregistrarServidor(const IpPuerto &ipServidor);
};
#endif // BROKERDEOBJETOS_H
