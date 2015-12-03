import java.io.*;
import java.net.*;

/*
 * Run code through terminal javac measurementServer.java compiles the code java measurementServer
 * port_number
 * 
 * No changes needed for this to work Will edit values automatically based on input from
 * measurementClient
 */

public class measurementServer {

  public static void main(String[] args) throws Exception {
    Socket request;
    BufferedReader in;
    PrintWriter out;

    int NUM_PROBES = 0; // how many times to expect a message
    String TO_COMPUTE; // rtt = round trip time, tput = throughput
    int SERVER_DELAY = 0;; // theoretical delay in ms
    int MESSAGE_SIZE = 0;; // in bytes

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

      // block until client connects
      // give clients IP address to make sure right client
      request = server.accept();
      System.err.println("Accepted connection from client: " + request.getInetAddress());

      // reader for messages from the client connected
      in = new BufferedReader(new InputStreamReader(request.getInputStream()));

      // writer to send messages back to client
      out = new PrintWriter(request.getOutputStream(), true);

      // get line from client
      // input should be "s String(rtt or tput) NumberOfProbes Message_Size Server_Delay"
      String input = in.readLine();

      // input should be setup string, split, check, and assign variables
      String[] S = input.split(" ");

      try {

        // setup phase
        if (S[0].equals("s")) {

          // get values
          TO_COMPUTE = S[1];
          // make sure S[1] is either rtt or tput, nothing else
          if (!(TO_COMPUTE.equals("rtt") || TO_COMPUTE.equals("tput"))) {
            out.println("404 ERROR: Invalid Connection Serup Message");
            server.close();
          }
          NUM_PROBES = Integer.parseInt(S[2]);
          MESSAGE_SIZE = Integer.parseInt(S[3]);
          SERVER_DELAY = Integer.parseInt(S[4]);

          // setup was correct
          out.println("200 OK: Ready");
        }

        // some error in the setup string, didn't follow guidelines
      } catch (Exception e) {
        out.println("404 ERROR: Invalid Connection Setup Message");
        server.close();
      }

      // setup phase worked properly
      // get ready for measurement phase
      int current = 1; // probe expected
      while (current <= NUM_PROBES) {

        // input should be "m CurrentProbe Payload"
        input = in.readLine();
        S = input.split(" ");
        // delay by time SERVER_DELAY
        try{
          Thread.sleep(SERVER_DELAY);
        } catch (InterruptedException ex){
          Thread.currentThread().interrupt();
        }
        if (S[0].equals("m")) {
          int cur = Integer.parseInt(S[1]); // probe sent by client
          // make sure the expected probe is the actual probe
          if (current != cur) {
            out.println("404 ERROR: Invalid Measurement Message");
            server.close();
          } else {
            // make sure payload size is the size specified earlier.
            if (S[2].length() == MESSAGE_SIZE) {
              current++;
              out.println(input);
            } else {
              out.println("404 ERROR: Invalid Measurement Message");
              server.close();
            }
          }
        }
      }
      // got through measurement phase okay
      // termination phase

      // input should be "t"
      input = in.readLine();
      if (input.equals("t")) {
        out.println("200 OK: Closing Connection");
        server.close();
      } else {
        out.println("404 ERROR: Invalid Connection Termination Messsage");
        server.close();
      }
    } catch (IOException e) {
    }
  }
}
