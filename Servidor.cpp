#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

const int MAX_CLIENTES = 2;
const int BUFFER_SIZE = 4096;
const int PUERTO = 80;

std::vector<SOCKET> clientes;

bool InicializarSockets(){
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void CerrarSockets(){
	WSACleanup();
}

void ProcesarCliente(SOCKET clienteSocket){
	char buffer[BUFFER_SIZE];
	while(true){
		memset(buffer, 0, BUFFER_SIZE);
		// Recibir mensaje del cliente
		int leerBytes = recv(clienteSocket, buffer, BUFFER_SIZE, 0);
		if(leerBytes == 0){
			// Error o cliente desconectado
			break;
		}
		
		// Envia mensaje a todos los demas clientes conectados
		for(SOCKET cliente : clientes){
			if(cliente != clienteSocket){
				send(cliente, buffer, leerBytes, 0);
			}
		}
	}
	
	// Cliente desconectado, cierra el socket y se elimina de la lista
	std::cout << "Cliente " << clienteSocket << " desconectado." << std::endl;
	closesocket(clienteSocket);
	clientes.erase(std::remove(clientes.begin(), clientes.end(), clienteSocket), clientes.end());
	
	if(clientes.empty()){
		std::cout << "No hay mas clientes conctados. Cerrando el servidor." << std::endl;
		exit(0);
	}
}

int main(){
	
	if(!InicializarSockets()){
		std::cerr << "Error al inicializar Winsock." << std::endl;
		return 1;
	}
	
	// Crear socket del servidor
	SOCKET servidorSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(servidorSocket == INVALID_SOCKET){
		std::cerr << "Error al crear el Socket." << std::endl;
		return 1;
	}
	
	// Configurar direccion del servidor
	sockaddr_in servidorAddress;
	servidorAddress.sin_family = AF_INET;
	servidorAddress.sin_addr.s_addr = INADDR_ANY;
	servidorAddress.sin_port = htons(PUERTO); 
	
	// Enlazar socket a la direccion del servidor
	if(bind(servidorSocket, (struct sockaddr*)&servidorAddress, sizeof(servidorAddress)) == SOCKET_ERROR){
		std::cerr << "Error al enlazar el socket." << std::endl;
		return 1;
	}
	
	// Escuchar nuevas conexiones
	if(listen(servidorSocket, MAX_CLIENTES) == SOCKET_ERROR){
		std::cerr << "Error al escuchar nuevas conexiones." << std::endl;
		return 1;
	}
	
	std::cout << "Servidor en ejecucion. Esperando clientes......" << std::endl;
	
	// Bucle principal para aceptar clientes
    while(true){
        // Aceptar nueva conexion de cliente
        sockaddr_in clienteAddress{};
        int clienteAddressSize = sizeof(clienteAddress);
        SOCKET clienteSocket = accept(servidorSocket, reinterpret_cast<struct sockaddr*>(&clienteAddress), &clienteAddressSize);

        if(clienteSocket == INVALID_SOCKET){
            std::cerr << "Error al aceptar la conexion del cliente." << std::endl;
            continue;
        }

        std::cout << "Cliente " << clienteSocket << " conectado." << std::endl;

        // Enviar mensaje al cliente
        const char* mensaje = "¡Bienvenido! Al simulacro de WhatsApp.\n";
        send(clienteSocket, mensaje, strlen(mensaje), 0);

        // Agregar el nuevo cliente a la lista
        clientes.push_back(clienteSocket);

        if(clientes.size() == MAX_CLIENTES){
            std::cout << "Se han alcanzado el numero maximo de clientes." << std::endl;
        }

        // Crear un hilo para procesar al cliente
        std::thread clienteThread(ProcesarCliente, clienteSocket);
        clienteThread.detach();
    }
	
	closesocket(servidorSocket);
	CerrarSockets();
	
	return 0;
}