import java.io.*;
import java.lang.Byte;
import java.net.*;

/*
 * Run code through terminal
 * javac measurementClient.java compiles the code
 * java measurementClient host_ip_address port_number
 * Before running measurementClient make sure that measurementServer is running
 * 
 * Change the final variables below for different cases
 * 
 * No other changes necessary.
 */

public class measurementClient {

  public static void main(String[] args) throws Exception {
    Socket client;
    PrintWriter out;
    BufferedReader in;

    final int NUM_PROBES = 10; // how many times to expect a message
    final String TO_COMPUTE = "rtt"; // rtt = round trip time, tput = throughput
    final int SERVER_DELAY = 0; // theoretical delay
    final int MESSAGE_SIZE = 10; // in bytes

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

      // send to server initial setup values
      String input =
          "s " + TO_COMPUTE + " " + Integer.toString(NUM_PROBES) + " "
              + Integer.toString(MESSAGE_SIZE) + " " + Integer.toString(SERVER_DELAY) + '\n';
      
      System.out.println("Starting Setup");
      
      out.println(input);

      // if OK message received, send probes
      if (in.readLine().equals("200 OK: Ready")) {
        
        System.out.println("Setup Complete.");
        
        int current = 1; // current probe being sent.
        long time = 0; // current time in nano secs
        double diffTime = 0.0; // used to calc RTT/TPUT
        Byte[] b = new Byte[MESSAGE_SIZE]; // b is size MESSAGE_SIZE bytes
        String toSend = fillToString(b); // fill b with 0's and turn it into a string for passing to
                                         // server
        double[] timeTaken = new double[NUM_PROBES]; // array to store values

        System.out.println("Starting Measurements");
        
        // loop to send all probes and record time
        while (current <= NUM_PROBES) {
          input = "m " + current + " " + toSend;
          time = System.nanoTime();
          out.println(input);
          in.readLine();
          diffTime = System.nanoTime() - time;
          if (TO_COMPUTE.equals("rtt")) {
            timeTaken[current - 1] = diffTime;
          } else if (TO_COMPUTE.equals("tput")) {
            timeTaken[current - 1] = MESSAGE_SIZE / diffTime;
          }
          current++;
        }

        System.out.println("Time taken on average for " + TO_COMPUTE + " was " + average(timeTaken)
            + " NanoSeconds");

      }

      System.out.println("Measurements Complete");
      System.out.println("Starting Termination");
      
      // termination phase
      input = "t\n";
      out.println(input);
      if (in.readLine().equals("200 OK: Closing Connection")) {
        System.out.println("Connection being closed.");
        client.close();
      } else {
        System.out.println("404 ERROR: Invalid Connection Termination Message");
        client.close();
      }

    } catch (IOException e) {
    }
  }

  //fills a byte[] with 0s and converts into a string.
  public static String fillToString(Byte[] a) {
    String s = "";
    for (int i = 0; i < a.length; i++) {
      a[i] = 0;
      s = s + a[i].toString();
    }
    return s;
  }

  // computes average
  public static double average(double[] a) {
    double sum = 0;
    for (int i = 0; i < a.length; i++) {
      sum = sum + a[i];
    }
    return sum / a.length;
  }
}
