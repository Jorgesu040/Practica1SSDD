#include "BrokerDeObjetos.h"
#include "utils.h"
#include <iostream>
#include <thread>

#define BROKER_IP "127.0.0.1"
#define BROKER_PORT 1033

using namespace std;


int main(int argc, char** argv)
{
    // Parse command line arguments for port (default: 1033)
    int port = BROKER_PORT;  // default port
    if(argc >= 2) {
        port = atoi(argv[1]);
        if(port < 1024 || port > 65535) {
            cout << "Invalid port number (must be between 1024-65535), using default: " << BROKER_PORT << endl;
            port = BROKER_PORT;
        }
    }

    bool exit=false;
    //start/open the server in a free port : 1033
    cout<<"Broker opening port " << port << "\n";
    int serverPortId=initServer(port);
    cout<<"Broker port opened\n";

    while(!exit){
        //wait for connections
        while(!checkClient()) usleep(100);
        //attend client
        int clientId=getLastClientID();
        cout<<"Client "<<clientId<<" connected\n";
        //send/recv messages
        thread* th=new thread(BrokerDeObjetos::resolverPeticion,clientId);
    }

    close(serverPortId);
    return 0;
}
