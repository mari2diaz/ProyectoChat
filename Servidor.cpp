#include "sockets.hpp"

std::vector<SOCKET> clientes;

void SolicitudesHTTP(SOCKET clienteSocket){
    char buffer[BUFFER_SIZE];
    while(true){
        memset(buffer, 0, BUFFER_SIZE);
        // Recibir mensaje del cliente
        int leerBytes = recv(clienteSocket, buffer, BUFFER_SIZE, 0);
        if (leerBytes == 0) {
            // Error o cliente desconectado
            continue;
        }

        // Enviar mensaje a todos los demas clientes conectados
        for (SOCKET cliente : clientes) {
            if (cliente != clienteSocket) {
                send(cliente, buffer, leerBytes, 0);
            }
        }
    }
}

int main(){
    if (!InicializarSockets()) {
        std::cerr << "Error al inicializar Winsock." << std::endl;
        return 1;
    }

    // Crear socket del servidor
    SOCKET servidorSocket = CrearSocket();

    // Configurar direccion del servidor
    sockaddr_in servidorAddress;
    servidorAddress.sin_family = AF_INET;
    servidorAddress.sin_addr.s_addr = INADDR_ANY;
    //servidorAddress.sin_addr.s_addr = inet_addr("192.168.1.105");
    servidorAddress.sin_port = htons(PUERTO);

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

    std::cout << "Servidor en ejecucion. Esperando clientes..." << std::endl;

    while(true){
        // Aceptar nueva conexion de cliente
        SOCKET clienteSocket = AceptarConexionCliente(servidorSocket);

        if(clienteSocket == -1){
            std::cout << "Error al aceptar conexion del cliente." << std::endl;
            continue;
        }

        std::cout << "Cliente " << clienteSocket << " conectado." << std::endl;

        // Enviar mensaje al cliente
        EnviarMensaje(clienteSocket, "¡Bienvenido! Al simulacro de WhatsApp.\n");

        // Agregar el nuevo cliente a la lista
        clientes.push_back(clienteSocket);

        if(clientes.size() == MAX_CLIENTES){
            std::cout << "Se ha alcanzado el numero maximo de clientes." << std::endl;
        }

        // Crear un hilo para procesar al cliente
        std::thread clienteThread(SolicitudesHTTP, clienteSocket);
        clienteThread.detach();
    }

    CerrarSockets(servidorSocket);

    return 0;
}
