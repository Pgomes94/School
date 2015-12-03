import java.io.*;
import java.net.*;

/*
 * Basic Echo Client
 */
 
public class Client {

  public static void main(String[] args) throws Exception {
    Socket client;
    PrintWriter out;
    BufferedReader in;
    BufferedReader stdIn;
    
    // if hostname and port weren't given
    if (args.length != 2) {
      System.out.println("Required arguments: hostname  port");
      return;
    }
    String hostname = args[0];
    int port = Integer.parseInt(args[1]);

    try {
      // server to connect to
      client = new Socket(hostname, port);

      // to write to the server
      out = new PrintWriter(client.getOutputStream(), true);

      // to read what the server sends
      in = new BufferedReader(new InputStreamReader(client.getInputStream()));

      // to read what was typed into console
      stdIn = new BufferedReader(new InputStreamReader(System.in));

      // send to server whatever was typed into the console
      out.println(stdIn.readLine());

      // print to the console what the server sends back
      System.out.println(in.readLine());

      // close client
      client.close();
    } catch (IOException e) {
    }
  }
}