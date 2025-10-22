
#include "utils.h"
#include "filemanager.h"
#include <string>


using namespace std;

class clientFileManager{
public:
	typedef enum{
		fileManagerConstructor,
		fileManagerDestructor,
		listFilesF,
		readFileF,
		writeFileF,
		ack
	}msgTypes;


	// Instances de bases de datos creadas por los clientes
	// Formato clave: clientID - valor: base de datos
	static inline map<int, FileManager> fileManagerInstances; 

	// Mapa para relacionar instancias de bases de datos con conexiones de clientes
	// Formato clave: puntero a base de datos - valor: serverId
	static inline int serverID;


	static void resolveClientMessages(int clientId);
	
};