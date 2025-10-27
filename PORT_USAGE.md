# Uso de Puertos en la Aplicación

## Validación de Puertos

Todos los ejecutables ahora validan que los puertos estén en el rango **1024-65535**:
- **Puertos 0-1023**: Puertos del sistema (requieren privilegios root)
- **Puertos 1024-49151**: Puertos registrados (recomendados para aplicaciones de usuario)
- **Puertos 49152-65535**: Puertos dinámicos/privados

## Uso de los Ejecutables

### Broker
```bash
./broker [puerto]
```
**Ejemplo:**
```bash
./broker 1033          # Usa puerto 1033
./broker               # Usa puerto por defecto: 1033
./broker 500           # Error: muestra mensaje y usa puerto por defecto
```

**Comandos disponibles:**
- `exit()` - Apagar el broker si el no hay clientes o servidores conectados

### Server
```bash
./server [server_ip] [server_port] [broker_ip] [broker_port]
```
**Ejemplo:**
```bash
./server 127.0.0.1 1067 127.0.0.1 1033  # Puerto 1067, conecta a broker en 127.0.0.1:1033 y le envia su IP y puerto
./server 127.0.0.1 1067                 # Puerto 1067, usa broker por defecto (127.0.0.1:1033)
./server                      # Usa todos los valores por defecto (Servidor: 127.0.0.1:1067 y Broker: 127.0.0.1:1033 
./server 70000                # Error: muestra mensaje y usa puerto por defecto
```

**Comandos disponibles:**
- `exit()` - Apagar el servidor de forma limpia (desregistrarse del broker) si no hay clientes conectados

### Client / FileManager
```bash
./client 
```
**Ejemplo:**
```bash
./client # Usa IP y Puerto por defecto en distributedFileManager.cpp (Cliente: 127.0.0.1:2067 y Broker: 127.0.0.1:1033)
```

#### **Nota**
- Como para mantener desacoplados el cliente de la clase distribuida `distributedFileManager.cpp`, y dado que no podía modificar `filemanager.h`, he optado por no leer por línea de comandos las IPs y puertos del cliente y broker
- En su lugar hay que modificar dentro de `distributedFileManager.cpp` los siguientes `#DEFINE`:
```c++
// Cambiar por los valores reales (si se lanza en localhost no hay problema, pero para IPs sería necesario cambiarlo)
#define BROKER_ID "127.0.0.1" 
#define BROKER_PORT 1033

#define DEFAULT_MY_IP "127.0.0.1"
#define DEFAULT_MY_PORT 2067
```

## Configuración Típica

1. **Iniciar el Broker:**
   ```bash
   ./broker 1033
   ```
   (Para apagar, escribir `exit()` y presionar Enter)

2. **Iniciar uno o más Servidores:**
   ```bash
   ./server 1067 127.0.0.1 1033
   ```
   (Para apagar, escribir `exit()` y presionar Enter)
   
   El servidor se conectará y registrará automáticamente en el broker.

3. **Iniciar Clientes:**
   ```bash
   ./client 127.0.0.1 1033
   ```
   
   Los clientes se conectarán al broker, que les asignará un servidor disponible.

## Notas
- Si un puerto está ocupado, el sistema operativo devolverá un error al intentar iniciar el servidor
- Los puertos por defecto son: Broker=1033, Server=1067
- Todos los componentes validan automáticamente el rango de puertos
- El servidor se registra automáticamente en el broker al iniciar
- Al escribir `exit()` en el servidor o broker, se apagan de forma limpia (el servidor se desregistra del broker automáticamente)
- Si el broker no está disponible, el servidor fallará al iniciar
- Los clientes obtienen servidores disponibles del broker automáticamente

