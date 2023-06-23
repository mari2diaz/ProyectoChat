#include "sockets.hpp"

void RecibirMensajes(SOCKET clienteSocket){
	char buffer[BUFFER_SIZE];
	while(true){
		memset(buffer, 0, BUFFER_SIZE);
		// Recibir mensaje del servidor
		int leerBytes = recv(clienteSocket, buffer, BUFFER_SIZE, 0);
		if(leerBytes <= 0){
			// Error o servidor desconectado
			break;
		}

		std::string mensaje(buffer);
        std::cout << "Mensaje recibido: " << mensaje << std::endl;
	}
}

int main(){

	if(!InicializarSockets()){
		std::cerr << "Error al inicializar Winsock." << std::endl;
		return 1;
	}

	// Crear el socket del cliente
	SOCKET clienteSocket = CrearSocket();

	// Obtener la direccion IP del servidor
	std::string direccionIP = ObtenerDireccionIPServidor("localhost", PUERTO);
	if(direccionIP.empty()){
		std::cerr << "No se pudo obtener la direccion IP del servidor." << std::endl;
		CerrarSockets(clienteSocket);
		return 1;
	}
	
	// Configurar direccion del servidor
	sockaddr_in servidorAddress{};
	servidorAddress.sin_family = AF_INET;
	servidorAddress.sin_addr.s_addr = inet_addr(direccionIP.c_str());
	servidorAddress.sin_port = htons(PUERTO);

	// Conectar al servidor
	if(connect(clienteSocket, reinterpret_cast<struct sockaddr*>(&servidorAddress), sizeof(servidorAddress)) == SOCKET_ERROR){
		std::cerr << "Error al conectar al servidor" << std::endl;
		CerrarSockets(clienteSocket);
		return 1;
	}

	// Crear hilo para recibir mensajes
	std::thread recibirThread(RecibirMensajes, clienteSocket);
	recibirThread.detach();

	std::string mensaje;
	while(true){
		std::getline(std::cin, mensaje);

        EnviarMensaje(clienteSocket, mensaje.c_str());
	}

	CerrarSockets(clienteSocket);

	return 0;
}

