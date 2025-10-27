
#include "utils.h"
#include "filemanager.h"
#include <string>
#include "clientFileManager.h"

#define ERRORLOG(msg) std::cerr << "[ERROR] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;


void clientFileManager::resolveClientMessages(int clientId)
{
    vector<unsigned char> buffer;
    bool logOut = false;
    do
    {
        // receive a packet from client
        recvMSG(clientId, buffer);
        auto type = unpack<msgTypes>(buffer);
        // switch type of the packet
        switch (type)
        {

        case fileManagerConstructor:
        {
            // Desempaquetar lugar de destino (directorio)
            string param1;
            param1.resize(unpack<long int>(buffer));
            unpackv(buffer, param1.data(), param1.size());
            // Crear la base datos
            FileManager fileManager(param1);
            // Guardamos la instancia

            fileManagerMutex.lock();
            fileManagerInstances[clientId] = fileManager;
            fileManagerMutex.unlock();
            buffer.clear();
        }
        break;
        case fileManagerDestructor:
        {
            fileManagerMutex.lock();
            fileManagerInstances.erase(clientId);
            fileManagerMutex.unlock();
            buffer.clear();
            logOut = true;
        }
        break;
        case listFilesF:
        {
            vector<string> res = fileManagerInstances[clientId].listFiles();

            buffer.clear();

            pack(buffer, res.size());
            // Empaquetar cada elemento de vector
            for (auto &str : res)
            {
                pack(buffer, str.size());
                packv(buffer, str.data(), str.size());
            }
            
        }
        break;
        case readFileF:
        {
            
            string fileName;
            vector<unsigned char> fileData;
            fileName.resize(unpack<long int>(buffer));
            unpackv(buffer, fileName.data(), fileName.size());

            fileManagerInstances[clientId].readFile(fileName, fileData);
            
            buffer.clear();
            pack(buffer, fileData.size());
            packv(buffer, fileData.data(), fileData.size());

        }
        break;
        case writeFileF:
        {
            string fileName = "";
            vector<unsigned char> fileData;
            fileName.resize(unpack<long int>(buffer));
            unpackv(buffer, fileName.data(), fileName.size());

            fileData.resize(unpack<long int>(buffer));
            unpackv(buffer, fileData.data(), fileName.size());

            fileManagerInstances[clientId].writeFile(fileName, fileData);
        }break;

        default:
                ERRORLOG("Mensaje no reconocido. Tipo recibido: " + to_string(type));
                logOut = true;

            break;
        }

        pack(buffer, ack);
        sendMSG(clientId, buffer);

    } while (!logOut);
    // close connection

    closeConnection(clientId);
}
