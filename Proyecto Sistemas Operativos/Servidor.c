#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment (lib, "Ws2_32.lib")


#define PUERTO 80
#define MAX_CLIENTE 2

void manejarSolicitudHttp(SOCKET clienteSocket);

int main(void){
	WSADATA wsaDATA;
	SOCKET servidorSocket, clienteSocket;
	struct sockaddr_in servidorAddress, clienteAddress;
	int longitudClienteAddr = sizeof(clienteAddress);
	int numClientes = 0;

	// Inicializar winsock
	if(WSAStartup(MAKEWORD(2, 2), &wsaDATA) != 0){
		printf("Error al inicializar winsock\n");
		return 1;
	}
	
	// Crear socket del servidor
	servidorSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(servidorSocket == INVALID_SOCKET){
		printf("Error al crear el socket\n");
		return 1;
	}
	
	// Configurar la direccion del servidor
	servidorAddress.sin_family = AF_INET;
	servidorAddress.sin_addr.s_addr = INADDR_ANY; // Direccion IP del servidor
	servidorAddress.sin_port = htons(PUERTO);     // Puerto de escucha (PORT 80)
	
	// Enlazar el socket del servidor a la direccion IP y al puerto
	if(bind(servidorSocket, (struct sockaddr*)&servidorAddress, sizeof(servidorAddress)) == SOCKET_ERROR){
		printf("Error al enlazar el socket\n");
		return 1;
	}
	
	// Poner el socket en modo de escucha
	if(listen(servidorSocket, SOMAXCONN) == SOCKET_ERROR){
		printf("Error al escucher el servidor\n");
		return 1;
	}else{
		printf("Escuchando......\n");
	}
	
	while(numClientes < MAX_CLIENTE){
		// Aceptar una coneccion de cliente
		clienteSocket = accept(servidorSocket, (struct sockaddr*)&clienteAddress, &longitudClienteAddr);
		if(clienteSocket == SOCKET_ERROR){
			printf("Error al aceptar la conexion del cliente\n");
			return 1;
		}
		
		// Incrementar la contador de cliente
		numClientes++;
		printf("Cliente %d conectado\n", numClientes);
		
		if(numClientes == MAX_CLIENTE){
			printf("Se han alcanzado el numero maximo de clientes\n");
		}
		
		// Enviar mensaje al cliente
		const char* mensaje = "Conectado\n";
		send(clienteSocket, mensaje, strlen(mensaje), 0);
	}
	
	// Esperar a que los clientes se desconecten
	while(numClientes > 0){
		// Buffer para almacenar los datos del cliente
		char buffer[256];
		int resultado = recv(clienteSocket, buffer, sizeof(buffer), 0);
		// Comprueba si hay error o si el cliente se desconecto
		if(resultado == SOCKET_ERROR || resultado == 0){
			printf("Cliente desconectado\n");
			// Cerrar los socket del cliente
			closesocket(clienteSocket);
			numClientes--;
			break;
		}
		
	}
	
	// Cerrar socket del servidor
	closesocket(servidorSocket);
	// Limpiar winsock
	WSACleanup();
	
	return 0;
}