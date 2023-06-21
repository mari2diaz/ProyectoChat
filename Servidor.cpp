#include "sockets.hpp"

std::vector<SOCKET> clientes;

void ProcesarCliente(SOCKET clienteSocket){
	char buffer[BUFFER_SIZE];
	while(true){
		memset(buffer, 0, BUFFER_SIZE);
		// Recibir mensaje del cliente
		int leerBytes = recv(clienteSocket, buffer, BUFFER_SIZE, 0);
		if(leerBytes == 0){
			// Error o cliente desconectado
			continue;
		}
		
		// Verificar si es comando de salida
		if(strncmp(buffer, "salir", 5) == 0){
			std::cout << "Cliente " << clienteSocket << " desconectado." << std::endl;
			break;
		}
		
		// Envia mensaje a todos los demas clientes conectados
		for(SOCKET cliente : clientes){
			if(cliente != clienteSocket){
				send(cliente, buffer, leerBytes, 0);
			}
		}
	}
	
	// Cerrar todos los sockets de los clientes
	for(SOCKET cliente : clientes){
		closesocket(clienteSocket);
	}
	clientes.clear();

	std::cout << "Todos los clientes han sido desconectados. Cerrando el servidor." << std::endl;
	exit(0);
}

int main(){
	
	if(!InicializarSockets()){
		std::cerr << "Error al inicializar Winsock." << std::endl;
		return 1;
	}
	
	// Crear socket del servidor
	SOCKET servidorSocket = CrearSocket();
	
	// Configurar direccion del servidor
	sockaddr_in servidorAddress;
	servidorAddress.sin_family = AF_INET;
	servidorAddress.sin_addr.s_addr = inet_addr("192.168.1.105");
	servidorAddress.sin_port = htons(PUERTO); 
	
	std::cout << "Direccion IP del servidor: " << ObtenerDireccionIP(servidorSocket) << std::endl;
	
	// Enlazar socket a la direccion del servidor
	if(bind(servidorSocket, reinterpret_cast<struct sockaddr*>(&servidorAddress), sizeof(servidorAddress)) == SOCKET_ERROR){
		std::cerr << "Error al enlazar el socket." << std::endl;
		return 1;
	}
	
	// Escuchar nuevas conexiones
	if(listen(servidorSocket, MAX_CLIENTES) == SOCKET_ERROR){
		std::cerr << "Error al escuchar nuevas conexiones." << std::endl;
		return 1;
	}
	
	std::cout << "Servidor en ejecucion. Esperando clientes......" << std::endl;
	
    while(true){
        // Aceptar nueva conexion de cliente
        SOCKET clienteSocket = AceptarConexionCliente(servidorSocket);

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
	
	CerrarSockets(servidorSocket);
	
	return 0;
}
