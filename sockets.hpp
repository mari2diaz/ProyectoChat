#define _WIN32_WINNT 0x0600  // Habilitar funciones más nuevas de Winsock
#include <iostream>
#include <string>
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


std::string ObtenerDireccionIPServidor(const std::string& host, int puerto){
    struct addrinfo hints, *resultado = nullptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Obtener la informacion de direcciones del servidor
    int resultadoConsulta = getaddrinfo(host.c_str(), std::to_string(puerto).c_str(), &hints, &resultado);
    if (resultadoConsulta != 0) {
        std::cerr << "Error al obtener la informacion de direcciones: " << gai_strerror(resultadoConsulta) << std::endl;
        return "";
    }

    std::string direccionIP;

    // Recorrer las direcciones encontradas hasta encontrar una direccion IPv4
    for (struct addrinfo* actual = resultado; actual != nullptr; actual = actual->ai_next) {
        if (actual->ai_family == AF_INET) {
            struct sockaddr_in* sockaddr = reinterpret_cast<struct sockaddr_in*>(actual->ai_addr);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(sockaddr->sin_addr), ip, INET_ADDRSTRLEN);
            direccionIP = std::string(ip);
            break;
        }
    }

    freeaddrinfo(resultado);

    return direccionIP;
}

SOCKET AceptarConexionCliente(SOCKET serverSocket){
	sockaddr_in clientAddress;
	int clientAddressSize = sizeof(clientAddress);
	SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressSize);

	if(clientSocket == INVALID_SOCKET){
		return -1;
	}

	return clientSocket;
}

void EnviarMensaje(SOCKET socket, const char* mensaje){
    send(socket, mensaje, strlen(mensaje), 0);
}

void CerrarSockets(SOCKET _socket){
    closesocket(_socket);
    WSACleanup();
}
