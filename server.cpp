#include "utils.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
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
static vector<int> activeClients;
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
void handleConnection(int clientId) {
	clientFileManager::resolveClientMessages(clientId);

	// Remover cliente de la lista cuando se desconecta
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

	cout << "Cliente " << clientId << " desconectado. Clientes restantes: " << remaining << endl;
}

int main(int argc, char **argv) {
	// Parse command line arguments: [server_ip] [server_port] [broker_ip] [broker_port]
	string serverIp = SERVER_IP;
	int port = SERVER_PORT; // default port
	string brokerIp = BROKER_IP;
	int brokerPort = BROKER_PORT;

	if (argc >= 2) {
		serverIp = argv[1];
		cout << "Usando IP del servidor: " << serverIp << endl;
		cout << "Y por defecto: [SERVER_PORT]" << SERVER_PORT << "[BROKER_IP]" << BROKER_IP << "[BROKER_PORT]" << BROKER_PORT << endl;
	}
	if (argc >= 3) {
		port = atoi(argv[2]);
		if (port < 1024 || port > 65535) {
			cout << "Número de puerto inválido (debe estar entre 1024-65535), usando el predeterminado: " << SERVER_PORT
					<< endl;
			port = SERVER_PORT;
		}
		cout << "Usando IP del servidor: " << serverIp << " Puerto del servidor: " << port << endl;
		cout << "Y por defecto: [BROKER_IP]" << BROKER_IP << "[BROKER_PORT]" << BROKER_PORT << endl;
	}

	if (argc >= 4) {
		brokerIp = argv[3];
		cout << "Usando IP del servidor: " << serverIp << " Puerto del servidor: " << port << " IP del broker: " << brokerIp
				<< endl;
		cout << "Y por defecto: [BROKER_PORT]" << BROKER_PORT << endl;
	}

	if (argc >= 5) {
		brokerPort = atoi(argv[4]);
		if (brokerPort < 1024 || brokerPort > 65535) {
			cout << "Puerto del broker inválido (debe estar entre 1024-65535), usando el predeterminado: " << BROKER_PORT
					<< endl;
			brokerPort = BROKER_PORT;
		}
		cout << "Usando IP del servidor: " << serverIp << " Puerto del servidor: " << port << " IP del broker: " << brokerIp
				<< " Puerto del broker: " << brokerPort << endl;
	}

	//start/open the server in a free port: 1067
	cout << "Servidor abriendo puerto " << port << "\n";
	int serverPortId = initServer(port);
	cout << "Puerto del servidor abierto, esperando conexiones\n";

	// Conectar al broker y registrarse
	cout << "Conectando al broker en " << brokerIp << ":" << brokerPort << "\n";
	BrokerServer brokerConnection(brokerIp, brokerPort, serverIp, port);

	if (brokerConnection.registrarEnBroker()) {
		cout << "Servidor registrado exitosamente en el broker\n";
	} else {
		cout << "ERROR: No se pudo registrar en el broker\n";
		close(serverPortId);
		return 1;
	}

	cout << "Escriba 'exit()' para apagar el servidor\n";

	// Crear thread para leer comandos desde stdin
	thread *commandThread = new thread(readCommands);

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
		auto *th = new thread(handleConnection, clientId);
		threads.push_back(th);  // Guardar referencia al thread
	}

	// Esperar a que termine el thread de comandos
	if (commandThread->joinable()) {
		commandThread->join();
		delete commandThread;
	}


	cout << "Servidor cerrado\n";
	return 0;
}
