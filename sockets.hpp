#define _WIN32_WINNT 0x0600  // Habilitar funciones más nuevas de Winsock
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <vector>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

const int MAX_CLIENTES = 2;
const int BUFFER_SIZE = 4096;
const int PUERTO = 80;

bool InicializarSockets(){
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

SOCKET CrearSocket(){
    SOCKET _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == INVALID_SOCKET){
        std::cerr << "Error al crear el Socket." << std::endl;
        exit(0);
    }
    return _socket;
}

std::string ObtenerDireccionIP(SOCKET _socket){
    sockaddr_in ipAddress;
    int ipAddressLength = sizeof(ipAddress);
    if(getsockname(_socket, reinterpret_cast<sockaddr*>(&ipAddress), &ipAddressLength) == SOCKET_ERROR){
        std::cerr << "Error al obtener la direccion IP del servidor." << std::endl;
        return "";
    }
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, (&ipAddress.sin_addr), ip, sizeof(ip));
    return ip;
}

SOCKET AceptarConexionCliente(SOCKET serverSocket){
	sockaddr_in clientAddress;
	int clientAddressSize = sizeof(clientAddress);
	SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressSize);
	
	if(clientSocket == INVALID_SOCKET){
		std::cerr << "Error al aceptar conexion del cliente." << std::endl;
		continue;
	}
	
	return clientSocket;
}

void CerrarSockets(SOCKET _socket){
    closesocket(_socket);
    WSACleanup();
}
