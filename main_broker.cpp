#include "BrokerDeObjetos.h"
#include "utils.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <sstream>
#include <unistd.h>

#define BROKER_PORT 1033

using namespace std;

static atomic<bool> brokerExit = false;
static vector<int> activeClients;
static mutex clientsMutex;

void readCommands() {
    string command;
    string line;
    do {
        getline(cin, line);
        stringstream ss(line);
        ss >> command;

        if (command == "exit()") {
            clientsMutex.lock();
            unsigned long numClients = activeClients.size();
            clientsMutex.unlock();

            if (numClients > 0) {
                cout << "ERROR: No se puede apagar el broker. Hay " << numClients
                     << " cliente(s) o servidor(es) conectado(s)." << endl;
                cout << "Espere a que todos se desconecten antes de apagar." << endl;
            } else {
                cout << "Comando de salida recibido. Apagando el broker..." << endl;
                brokerExit = true;
            }
        } else if (!command.empty()) {
            cout << "Comando desconocido: " << command << ". Use 'exit()' para apagar." << endl;
        }
    } while (command != "exit()" || !brokerExit);
}

// Función wrapper para manejar desconexión de clientes
void handleConnection(int clientId) {
	BrokerDeObjetos::resolverPeticion(clientId);
    // remove from active list when resolver returns
    clientsMutex.lock();
    
    auto it = activeClients.begin();

    while (it != activeClients.end()) {
        if (*it == clientId) {
            activeClients.erase(it);
            break;
        }
        ++it;
    }

    unsigned long remaining = activeClients.size();
    clientsMutex.unlock();
    cout << "Cliente " << clientId << " desconectado. Restantes: " << remaining << endl;
}

int main(int argc, char** argv)
{
    // Parse command line arguments for port (default: 1033)
    int port = BROKER_PORT;  // default port
    if(argc >= 2) {
        port = atoi(argv[1]);
        if(port < 1024 || port > 65535) {
            cout << "Puerto inválido (debe estar entre 1024-65535), usando el predeterminado: " << BROKER_PORT << endl;
            port = BROKER_PORT;
        }
    }

    //start/open the server in a free port : 1033
    cout<<"Broker usando el puerto " << port << "\n";
    int serverPortId=initServer(port);
    cout<<"Broker puerto abierto\n";

    cout<<"Escriba 'exit()' para apagar el broker\n";

    // start thread to read commands from stdin
    thread *commandThread = new thread(readCommands);

    vector<thread*> threads;

    while(!brokerExit){
        //wait for connections
        while(!checkClient() && !brokerExit) usleep(100);
        if (brokerExit) break;

        //attend client
        int clientId=getLastClientID();
        cout<<"Cliente "<<clientId<<" conectado\n";

        clientsMutex.lock();
        activeClients.push_back(clientId);
        unsigned long numClients = activeClients.size();
        clientsMutex.unlock();

        cout << "Conexiones activas: " << numClients << endl;

        //send/recv messages - wrap resolver so we can remove client when done
        thread* th = new thread(handleConnection, clientId);
        threads.push_back(th);
    }

    // wait for command thread
    if (commandThread->joinable()) commandThread->join();
    delete commandThread;

    // wait for all worker threads
    for (thread* th : threads) {
        if (th->joinable()) th->join();
        delete th;
    }

    close(serverPortId);
    cout << "Broker cerrado\n";
    return 0;
}
