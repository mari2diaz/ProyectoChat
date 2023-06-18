#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#pragma comment(lib, "Ws2_32.lib")

const int BUFFER_SIZE = 4096;
const int PUERTO = 80;

bool InicializarSockets(){
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void CerrarSockets(){
	WSACleanup();
}

std::string ObtenerDireccionIPServidor(const std::string& host, int puerto){
	struct addrinfo hints, *resultado = nullptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Obtener la informacion de direcciones del servidor
    int resultadoConsulta = getaddrinfo(host.c_str(), std::to_string(puerto).c_str(), &hints, &resultado);
    if(resultadoConsulta != 0){
        std::cerr << "Error al obtener la informacion de direcciones: " << gai_strerror(resultadoConsulta) << std::endl;
        return "";
    }

    std::string direccionIP;

    // Recorrer las direcciones encontradas hasta encontrar una direccion IPv4
    for(struct addrinfo* actual = resultado; actual != nullptr; actual = actual->ai_next){
        if(actual->ai_family == AF_INET){
            struct sockaddr_in* sockaddr = reinterpret_cast<struct sockaddr_in*>(actual->ai_addr);
            unsigned char* ipBytes = reinterpret_cast<unsigned char*>(&(sockaddr->sin_addr));
            char ip[16];
            snprintf(ip, sizeof(ip), "%d.%d.%d.%d", ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]);
            direccionIP = std::string(ip);
            break;
        }
    }

    freeaddrinfo(resultado);

    return direccionIP;
}

void recibirArchivo(SOCKET clienteSocket, const std::string& nombreArchivo){
	std::ofstream archivo(nombreArchivo, std::ios::binary);
	if(!archivo){
		std::cerr << "Error al crear el archivo: " << nombreArchivo << std::endl;
		return;
	}
	
	// Recibir el tamaño del archivo desde el servidor
	std::streamsize tamano;
	recv(clienteSocket, reinterpret_cast<char*>(&tamano), sizeof(tamano), 0);
	
	// Crear buffer para almacenar los datos recibidos 
	std::vector<char> buffer(BUFFER_SIZE);
	
	// Recibir y escribir los datos en el servidor
	std::streamsize tatalBytesRecibidos = 0;
	while(tatalBytesRecibidos < tamano){
		
		int bytesRecibidos = recv(clienteSocket, buffer.data(), BUFFER_SIZE, 0);
		if(bytesRecibidos <= 0){
			std::cerr << "Error al recibir datos del servidor." << std::endl;
			archivo.close();
			std::remove(nombreArchivo.c_str()); // Eliminar el archivo incompleto
			return;
		}
		
		archivo.write(buffer.data(), bytesRecibidos);
		tatalBytesRecibidos += bytesRecibidos;
	}
	
	archivo.close();
	std::cout << "Archivo recibido: " << nombreArchivo << std::endl;
}

void recibirMensajesArchivos(SOCKET clienteSocket){
	char buffer[BUFFER_SIZE];
	while(true){
		memset(buffer, 0, BUFFER_SIZE);
		// Recibir mensaje del servidor
		int leerBytes = recv(clienteSocket, buffer, BUFFER_SIZE, 0);
		if(leerBytes <= 0){
			// Error o servidor desconectado
			break;
		}
		
		// Verificar si es un mensaje o un nombre de archivo
		std::string mensaje(buffer);
		if(mensaje.substr(0, 9) == "ARCHIVO: "){
			std::string nombreArchivo = mensaje.substr(9);
			recibirArchivo(clienteSocket, nombreArchivo);
		}else{
			std::cout << "Mensaje recibido: " << mensaje << std::endl;
		}
		
		// Cerrar socket si servidor desconectado
		std::cout << "El servidor se ha desconectado." << std::endl;
		closesocket(clienteSocket);
		CerrarSockets(); 
	}
}

void enviarMensaje(SOCKET clienteSocket, const std::string& mensaje){
	send(clienteSocket, mensaje.c_str(), mensaje.length(), 0);
}

void enviarArchivo(SOCKET clienteSocket, const std::string& nombreArchivo){
	std::ifstream archivo(nombreArchivo, std::ios::binary);
	if(!archivo){
		std::cerr << "Error al abrir el archivo: " << nombreArchivo << std::endl;
		return;
	}
	
	// Obtener tmaño del archivo
	archivo.seekg(0, std::ios::end);
	std::streamsize tamano = archivo.tellg();
	archivo.seekg(0, std::ios::beg);
	
	// Crear el buffer para almacenar el archivo
	std::vector<char> buffer(tamano);
	
	// Leer el archivo en el buffer
	if(!archivo.read(buffer.data(), tamano)){
		std::cerr << "Error al leer el archivo: " << nombreArchivo << std::endl;
		return;
	}
	
	// Enviar el tamano del archivo al servidor 
	send(clienteSocket, reinterpret_cast<const char*>(&tamano), sizeof(tamano), 0);
	
	// Enviar archivo al servidor
	send(clienteSocket, buffer.data(), tamano, 0);
	
	archivo.close();
}

int main(){
	
	if(!InicializarSockets()){
		std::cerr << "Error al inicializar Winsock." << std::endl;
		return 1;
	}
	
	// Crear el socket del cliente
	SOCKET clienteSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clienteSocket == INVALID_SOCKET){
		std::cerr << "Error al crear el socket." << std::endl;
		return 1;
	}
	
	// Obtener la direccion IP del servidor
	std::string direccionIP = ObtenerDireccionIPServidor("localhost", PUERTO);
	if(!direccionIP.empty()){
		std::cout << "Direccion IP del servidor: " << direccionIP << std::endl;
	}else{
		std::cout << "No se pudo obtener la direccion IP del servidor." << std::endl;
	}
	
	// Configurar direccion del servidor
	sockaddr_in servidorAddress{};
	servidorAddress.sin_family = AF_INET;
	servidorAddress.sin_addr.s_addr = inet_addr(direccionIP.c_str());
	servidorAddress.sin_port = htons(PUERTO); 
	
	// Conectar al servidor
	if(connect(clienteSocket, reinterpret_cast<struct sockaddr*>(&servidorAddress), sizeof(servidorAddress)) == SOCKET_ERROR){
		std::cerr << "Error al conectar al servidor" << std::endl;
		closesocket(clienteSocket);
		CerrarSockets();
		return 1;
	}
	
	// Crear hilo para recibir mensajes y archivos del servidor
	std::thread recibirThread(recibirMensajesArchivos, clienteSocket);
	recibirThread.detach();
	
	// Bucle para enviar mensajes y archivos
	while(true){
		std::string entrada;
		std::cout << "Ingrese un mensaje o el nombre de un archivo (s para salir): ";
		std::getline(std::cin, entrada);
		
		if(entrada == "q" || entrada.empty()){
			break;
		}
		
		if(std::ifstream(entrada)){
			// Es un archivo
			enviarMensaje(clienteSocket, "ARCHIVO: " + entrada);
			enviarArchivo(clienteSocket, entrada);
		}else{
			// Es un mensaje
			enviarMensaje(clienteSocket, entrada);
		}
	}
	
	closesocket(clienteSocket);
	CerrarSockets();
	
	return 0;
}