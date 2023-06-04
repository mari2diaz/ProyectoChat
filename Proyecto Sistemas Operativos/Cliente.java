import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class Cliente{
    public static void main(String[] args){
        try{
            // Obtener direccion IP del srvidor
            InetAddress direccion = InetAddress.getLocalHost();
            String direccionIP = direccion.getHostAddress();
            System.out.println("Direccion IP del servidor: " + direccionIP);

            Socket socket = new Socket(direccionIP, 80);
            System.out.println("Conexion establecida");

            BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter writer = new PrintWriter(socket.getOutputStream(), true);

            // Leer mensaje del servidor
            String mensaje = reader.readLine();
            System.out.println("Mensaje del servidor: " + mensaje);

            BufferedReader leerInputUsuario = new BufferedReader(new InputStreamReader(System.in));
            String input;

            // Leer mensaje del usuario para desconectar al cliente
            do{
                System.out.println("Preciones q para desconectarse");
                input = leerInputUsuario.readLine();
            }while(!input.equalsIgnoreCase("q"));

            // Cerrar la conexion
            writer.close();
            reader.close();
            socket.close();
            System.out.println("Conexion cerrada");
        }catch(UnknownHostException e){
            System.out.println("Error al obtener la direccion IP del servidor" + e.getMessage());
        }catch(IOException e){
            System.out.println("Error al conectar con el servidor" + e.getMessage());
        }
    }
}