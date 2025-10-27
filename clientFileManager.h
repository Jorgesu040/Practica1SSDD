#pragma once
#include "utils.h"
#include "filemanager.h"


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

	static inline mutex fileManagerMutex;

	static void resolveClientMessages(int clientId);

};