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
		ack = 5
	}msgTypes;


	// Instances de bases de datos creadas por los clientes
	// Formato clave: clientID - valor: FileManager
	static inline map<int, FileManager> fileManagerInstances; 

	// id del servidor: serverID
	static inline int serverID;


	static void resolveClientMessages(int clientId);
	
};