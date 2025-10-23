#include "filemanager.h"
#include "utils.h"
#include <string>
#include "clientFileManager.h"

#include <iostream>
#include <fstream>

#define SERVER_ID "127.0.0.1"
#define SERVER_PORT 1067
#define ERRORLOG(msg) std::cerr << "[ERROR] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;

using namespace std;

FileManager::FileManager() : FileManager("./") {
}

FileManager::~FileManager(){

}

FileManager::FileManager(string path){
    vector<unsigned char> buffer;
    int serverID = initClient(SERVER_ID, SERVER_PORT).serverId;

    pack(buffer, clientFileManager::fileManagerConstructor);
    pack(buffer, path.size());
    packv(buffer, path.data(), path.size());

    sendMSG(serverID, buffer);

    // Ack
    buffer.clear();
    recvMSG(serverID, buffer);
    if ( unpack<clientFileManager::msgTypes>(buffer) != clientFileManager::ack){
		ERRORLOG("No se ha recibido el ack del servidor");
	}
    
    clientFileManager::serverID = serverID;
}

vector<string> FileManager::listFiles(){

    vector<unsigned char> buffer;

    pack(buffer, clientFileManager::listFilesF);
    sendMSG(clientFileManager::serverID, buffer);


    buffer.clear();
    recvMSG(clientFileManager::serverID, buffer);

    vector<string> listedFiles;

    listedFiles.resize(unpack<long int>(buffer));

    for (auto &fileName : listedFiles) {

        fileName.resize(unpack<long int>(buffer));
        unpackv(buffer, fileName.data(), fileName.size());
        std::cout << "Reading file: " << fileName << std::endl;

    }

    if ( unpack<clientFileManager::msgTypes>(buffer) != clientFileManager::ack){
		ERRORLOG("No se ha recibido el ack del servidor");
	}

    return listedFiles;

}


void FileManager::readFile(string fileName, vector<unsigned char> &data){

    vector<unsigned char> buffer;

    pack(buffer, clientFileManager::readFileF);
	pack(buffer, fileName.size());
	packv(buffer, fileName.data(), fileName.size());
    sendMSG(clientFileManager::serverID, buffer);

    buffer.clear();
    recvMSG(clientFileManager::serverID, buffer);

    data.resize(unpack<long int>(buffer));
    unpackv(buffer, data.data(), data.size());
    

    if ( unpack<clientFileManager::msgTypes>(buffer) != clientFileManager::ack){
		ERRORLOG("No se ha recibido el ack del servidor");
	}

}


void FileManager::writeFile(string fileName, vector<unsigned char> &data){

    vector<unsigned char> buffer;

    
    pack(buffer, clientFileManager::writeFileF);

    pack<long int>(buffer, fileName.size());
    packv(buffer, fileName.data(), fileName.size());

    pack<long int>(buffer, data.size());
    packv(buffer, data.data(), data.size());

    sendMSG(clientFileManager::serverID, buffer);

    // Ack
    buffer.clear();
    recvMSG(clientFileManager::serverID, buffer);
    if ( unpack<clientFileManager::msgTypes>(buffer) != clientFileManager::ack){
		ERRORLOG("No se ha recibido el ack del servidor");
	}

}