#include "filemanager.h"
#include "utils.h"
#include <string>
#include "clientFileManager.h"
#include "BrokerClient.h"
#include "BrokerDeObjetos.h"

#include <iostream>
#include <fstream>

#define BROKER_ID "127.0.0.1"
#define BROKER_PORT 1033

#define DEFAULT_MY_IP "127.0.0.1"
#define DEFAULT_MY_PORT 2067

#define ERRORLOG(msg) std::cerr << "[ERROR] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;

using namespace std;

// Mapa para relacionar FileManager con el ID del servidor, en el caso de
// lanzar varios fileManagers desde el mismo programa cliente
static inline map<FileManager*, int> fileManagerServerIds;
static inline mutex serverMapMutex;


static BrokerClient& getBroker() {
    static BrokerClient broker(BROKER_ID, BROKER_PORT, DEFAULT_MY_IP, DEFAULT_MY_PORT);
    return broker;
}

FileManager::FileManager() : FileManager("./") {
}

FileManager::~FileManager(){
    serverMapMutex.lock();

    auto it = fileManagerServerIds.find(this);
    if (it != fileManagerServerIds.end()) {
        closeConnection(it->second);
        fileManagerServerIds.erase(it);
    }

    serverMapMutex.unlock();
}

FileManager::FileManager(string path){
    
    vector<unsigned char> buffer;

    BrokerClient& broker = getBroker();
    BrokerDeObjetos::IpPuerto server = broker.obtenerServidorDisponible();

    if (server.port == -1){
        ERRORLOG("No se ha podido obtener un servidor disponible del broker");
        return;
    }

    int serverID = initClient(server.ip, server.port).serverId;

    pack(buffer, clientFileManager::fileManagerConstructor);
    pack(buffer, path.size());
    packv(buffer, path.data(), path.size());

    sendMSG(serverID, buffer);

    buffer.clear();
    recvMSG(serverID, buffer);
    if (unpack<clientFileManager::msgTypes>(buffer) != clientFileManager::ack){
        ERRORLOG("No se ha recibido el ack del servidor");
    }

    // Guardar serverID asociado a esta instancia
    serverMapMutex.lock();
    fileManagerServerIds[this] = serverID;
    serverMapMutex.unlock();
}

vector<string> FileManager::listFiles(){
    vector<unsigned char> buffer;

    // Obtener serverID de esta instancia
    serverMapMutex.lock();
    int myServerID = fileManagerServerIds[this];
    serverMapMutex.unlock();

    pack(buffer, clientFileManager::listFilesF);
    sendMSG(myServerID, buffer);

    buffer.clear();
    recvMSG(myServerID, buffer);

    vector<string> listedFiles;
    listedFiles.resize(unpack<long int>(buffer));

    if (unpack<clientFileManager::msgTypes>(buffer) != clientFileManager::ack){
        ERRORLOG("No se ha recibido el ack del servidor");
    }

    return listedFiles;
}

void FileManager::readFile(string fileName, vector<unsigned char> &data){
    vector<unsigned char> buffer;

    serverMapMutex.lock();
    int myServerID = fileManagerServerIds[this];
    serverMapMutex.unlock();

    pack(buffer, clientFileManager::readFileF);
    pack(buffer, fileName.size());
    packv(buffer, fileName.data(), fileName.size());
    sendMSG(myServerID, buffer);

    buffer.clear();
    recvMSG(myServerID, buffer);

    data.resize(unpack<long int>(buffer));
    unpackv(buffer, data.data(), data.size());

    if (unpack<clientFileManager::msgTypes>(buffer) != clientFileManager::ack){
        ERRORLOG("No se ha recibido el ack del servidor");
    }
}

void FileManager::writeFile(string fileName, vector<unsigned char> &data){
    vector<unsigned char> buffer;

    serverMapMutex.lock();
    int myServerID = fileManagerServerIds[this];
    serverMapMutex.unlock();

    pack(buffer, clientFileManager::writeFileF);
    pack<long int>(buffer, fileName.size());
    packv(buffer, fileName.data(), fileName.size());
    pack<long int>(buffer, data.size());
    packv(buffer, data.data(), data.size());

    sendMSG(myServerID, buffer);

    buffer.clear();
    recvMSG(myServerID, buffer);
    if (unpack<clientFileManager::msgTypes>(buffer) != clientFileManager::ack){
        ERRORLOG("No se ha recibido el ack del servidor");
    }
}
