#include "utils.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <list>
#include <atomic>
#include "clientFileManager.h"
#include "BrokerServer.h"

#define BROKER_IP "127.0.0.1"
#define BROKER_PORT 1033

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 1067

using namespace std;

// Variable global para controlar la salida del servidor
static atomic<bool> serverExit = false;
static list<int> activeClients;
static mutex clientsMutex;

// Función para leer comandos desde stdin
void readCommands() {
	string command;
	string line;
	do {
		getline(cin, line);
		stringstream ss(line);
		ss >> command;

		if (command == "exit()") {
			// Verificar si hay clientes conectados
			clientsMutex.lock();
			unsigned long numClients = activeClients.size();
			clientsMutex.unlock();

			if (numClients > 0) {
				cout << "ERROR: No se puede apagar el servidor. Hay " << numClients
				     << " cliente(s) conectado(s)." << endl;
				cout << "Espere a que todos los clientes se desconecten antes de apagar." << endl;
			} else {
				cout << "Comando de salida recibido. Apagando el servidor..." << endl;
				serverExit = true;
			}
		} else if (!command.empty()) {
			cout << "Comando desconocido: " << command << ". Use 'exit()' para apagar." << endl;
		}
	} while (command != "exit()" || !serverExit);
}

// Función wrapper para manejar desconexión de clientes
void handleClient(int clientId) {
	clientFileManager::resolveClientMessages(clientId);

	// Remover cliente de la lista cuando se desconecta
	clientsMutex.lock();
	activeClients.remove(clientId);
	unsigned long remaining = activeClients.size();
	clientsMutex.unlock();

	cout << "Cliente " << clientId << " desconectado. Clientes restantes: " << remaining << endl;
}

int main(int argc, char **argv) {
	// Parse command line arguments: [server_port] [broker_ip] [broker_port]
	int port = SERVER_PORT; // default port
	string brokerIp = BROKER_IP;
	int brokerPort = BROKER_PORT;

	if (argc >= 2) {
		port = atoi(argv[1]);
		if (port < 1024 || port > 65535) {
			cout << "Número de puerto inválido (debe estar entre 1024-65535), usando el predeterminado: " << SERVER_PORT
					<< endl;
			port = SERVER_PORT;
		}
	}

	if (argc >= 3) {
		brokerIp = argv[2];
	}

	if (argc >= 4) {
		brokerPort = atoi(argv[3]);
		if (brokerPort < 1024 || brokerPort > 65535) {
			cout << "Puerto del broker inválido (debe estar entre 1024-65535), usando el predeterminado: " << BROKER_PORT
					<< endl;
			brokerPort = BROKER_PORT;
		}
	}

	//start/open the server in a free port: 1067
	cout << "Servidor abriendo puerto " << port << "\n";
	int serverPortId = initServer(port);
	cout << "Puerto del servidor abierto, esperando conexiones\n";

	// Conectar al broker y registrarse
	cout << "Conectando al broker en " << brokerIp << ":" << brokerPort << "\n";
	BrokerServer brokerConnection(brokerIp, brokerPort, SERVER_IP, port);

	if (brokerConnection.registrarEnBroker()) {
		cout << "Servidor registrado exitosamente en el broker\n";
	} else {
		cout << "ERROR: No se pudo registrar en el broker\n";
		close(serverPortId);
		return 1;
	}

	cout << "Escriba 'exit()' para apagar el servidor\n";

	// Crear thread para leer comandos desde stdin
	thread commandThread(readCommands);

	vector<thread*> threads;

	while (!serverExit) {
		//wait for connections
		while (!checkClient() && !serverExit) usleep(100);

		if (serverExit) break;

		//attend client
		int clientId = getLastClientID();
		cout << "Cliente " << clientId << " conectado\n";

		clientsMutex.lock();
		activeClients.push_back(clientId);
		unsigned long numClients = activeClients.size();
		clientsMutex.unlock();

		cout << "Clientes activos: " << numClients << endl;

		//send/recv messages
		auto *th = new thread(handleClient, clientId);
		threads.push_back(th);  // Guardar referencia al thread
	}

	// Esperar a que termine el thread de comandos
	if (commandThread.joinable()) {
		commandThread.join();
	}



	// Esperar a que terminen todos los threads
	for (thread* th : threads) {
		if (th->joinable())
			th->join();
		delete th;
	}



	close(serverPortId);
	cout << "Servidor cerrado\n";
	return 0;
}
