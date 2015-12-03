import java.io.*;
import java.net.*;

/*
 * Basic Echo Server
 */
 
public class Server {

  public static void main(String[] args) throws Exception {
    Socket request;
    BufferedReader in;
    PrintWriter out;

    // if port wasn't given
    if (args.length != 1) {
      System.out.println("Required arguments: port");
      return;
    }
    int port = Integer.parseInt(args[0]);

    try {
      // initialize SocketServer at port
      @SuppressWarnings("resource")
      ServerSocket server = new ServerSocket(port);

      // processing loop
      // accepts message (string) from client server repeatedly
      // when message is received echoes back the message
      while (true) {

        // block until client connects
        // give clients IP address
        request = server.accept();
        System.err.println("Accepted connection from client: " + request.getInetAddress());

        // reader for messages from the client connected
        in = new BufferedReader(new InputStreamReader(request.getInputStream()));

        // writer to send messages back to client
        out = new PrintWriter(request.getOutputStream(), true);

        // get line from client and send it back to client
        out.println(in.readLine());
      }
    } catch (IOException e) {
    }
  }
}