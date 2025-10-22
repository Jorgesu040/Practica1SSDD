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
/**
 * @brief FileManager::FileManager Constructor of the FileManager class. It receives by parameters the directory
 * that this class will use to index, store and read files. It is recommended to use a full path to the directory,
 * from the root of the file system.
 *
 * @param path Path to the directory you want to use.
 */
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
/**
 * @brief FileManager::listFiles Used to access the list of files stored in the path
 * that was used in the class constructor. Only lists files, directories are ignored.
 *
 */
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

    }


    // Ack
    buffer.clear();
    recvMSG(clientFileManager::serverID, buffer);
    if ( unpack<clientFileManager::msgTypes>(buffer) != clientFileManager::ack){
		ERRORLOG("No se ha recibido el ack del servidor");
	}

    return listedFiles;

}


/**
 * @brief FileManager::readFile Given the name of a file stored in the directory used in the constructor,
 * the variable "data" will be filled with the contents of the file
 *
 * @param fileName Name of the file to read
 * @param data File data
 */
void FileManager::readFile(string fileName, vector<unsigned char> &data){

    vector<unsigned char> buffer;

    pack(buffer, clientFileManager::readFileF);
    sendMSG(clientFileManager::serverID, buffer);

    buffer.clear();
    recvMSG(clientFileManager::serverID, buffer);

    data.resize(unpack<long int>(buffer));
    unpackv(buffer, data.data(), data.size());

    // Ack
    buffer.clear();
    recvMSG(clientFileManager::serverID, buffer);
    if ( unpack<clientFileManager::msgTypes>(buffer) != clientFileManager::ack){
		ERRORLOG("No se ha recibido el ack del servidor");
	}

}


/**
 * @brief FileManager::writeFile Given a new name of a file to be stored in the directory used in the constructor,
 * the contents of the data array stored in "data" will be written. It will overwrite a file in the directory if it has the same name.
 *
 * @param fileName Name of the file to write.
 * @param data Data of the file.
 */
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