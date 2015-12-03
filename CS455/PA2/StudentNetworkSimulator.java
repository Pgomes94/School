import java.util.*;
import java.io.*;

/*
 * RUN NETWORKSIMULATOR TO CHECK CODE
 */

public class StudentNetworkSimulator extends NetworkSimulator {
  /*
   * Predefined Constants (static member variables):
   * 
   * int MAXDATASIZE : the maximum size of the Message data and Packet payload
   * 
   * int A 0 : a predefined integer that represents entity A int B 1 : a predefined integer that
   * represents entity B
   * 
   * Predefined Member Methods:
   * 
   * void stopTimer(int entity): Stops the timer running at "entity" [A or B] void startTimer(int
   * entity, double increment): Starts a timer running at "entity" [A or B], which will expire in
   * "increment" time units, causing the interrupt handler to be called. You should only call this
   * with A. void toLayer3(int callingEntity, Packet p) Puts the packet "p" into the network from
   * "callingEntity" [A or B] void toLayer5(String dataSent) Passes "dataSent" up to layer 5 double
   * getTime() Returns the current time in the simulator. Might be useful for debugging. int
   * getTraceLevel() Returns TraceLevel void printEventList() Prints the current event list to
   * stdout. Might be useful for debugging, but probably not.
   * 
   * 
   * Predefined Classes:
   * 
   * Message: Used to encapsulate a message coming from layer 5 Constructor: Message(String
   * inputData): creates a new Message containing "inputData" Methods: boolean setData(String
   * inputData): sets an existing Message's data to "inputData" returns true on success, false
   * otherwise String getData(): returns the data contained in the message Packet: Used to
   * encapsulate a packet Constructors: Packet (Packet p): creates a new Packet that is a copy of
   * "p" Packet (int seq, int ack, int check, String newPayload) creates a new Packet with a
   * sequence field of "seq", an ack field of "ack", a checksum field of "check", and a payload of
   * "newPayload" Packet (int seq, int ack, int check) chreate a new Packet with a sequence field of
   * "seq", an ack field of "ack", a checksum field of "check", and an empty payload Methods:
   * boolean setSeqnum(int n) sets the Packet's sequence field to "n" returns true on success, false
   * otherwise boolean setAcknum(int n) sets the Packet's ack field to "n" returns true on success,
   * false otherwise boolean setChecksum(int n) sets the Packet's checksum to "n" returns true on
   * success, false otherwise boolean setPayload(String newPayload) sets the Packet's payload to
   * "newPayload" returns true on success, false otherwise int getSeqnum() returns the contents of
   * the Packet's sequence field int getAcknum() returns the contents of the Packet's ack field int
   * getChecksum() returns the checksum of the Packet int getPayload() returns the Packet's payload
   */

  /*
   * Please use the following variables in your routines. int WindowSize : the window size double
   * RxmtInterval : the retransmission timeout int LimitSeqNo : when sequence number reaches this
   * value, it wraps around
   */

  public static final int FirstSeqNo = 0;
  private int WindowSize;
  private double RxmtInterval;
  private int LimitSeqNo; // mod by to wrap around

  // Add any necessary class variables here. Remember, you cannot use
  // these variables to send messages error free! They can only hold
  // state information for A or B.
  // Also add any necessary methods (e.g. checksum of a String)
  private int base = FirstSeqNo; // where window starts
  private int nextSeqExpected = FirstSeqNo; // next sequence number expected
  private int bExpectedSeq; // sequence number b is waiting for
  private Packet b; // ack packet b will send
  private int bAck; // ack number b sends back
  private Packet[] beingSent; // packets being sent across medium
  private List<Message> toBeSent; // buffer packets not sent yet
  private static final int bufferLimit = 50; // limits amount of packets waiting to be sent
  private boolean inserted = false;

  // variables for Simulaton_Done
  private int origDataPackets = 0;
  private int retransmissions = 0;
  private int ackPackets = 0;
  private boolean corrupted = false;
  private int lostPackets = 0;
  private int corruptedPackets = 0;
  private double avgRTT = 0.0;
  private double startTime = 0;
  private double endTime = 0;
  private int timeCounter = 0;
  private double timeSum = 0;

  // calculates checksum given packet info
  protected int checkSum(int seqNum, int ackNum, String payload) {
    int sum = seqNum + ackNum + stringToInt(payload);
    return sum;
  }

  // used by check sum, converts string to unique int
  protected int stringToInt(String payload) {
    int sum = 0;
    Character c;
    for (int i = 0; i < payload.length(); i++) {
      c = payload.charAt(i);
      sum += c.hashCode();
    }
    return sum;
  }

  // This is the constructor. Don't touch!
  public StudentNetworkSimulator(int numMessages, double loss, double corrupt, double avgDelay,
      int trace, int seed, int winsize, double delay) {
    super(numMessages, loss, corrupt, avgDelay, trace, seed);
    WindowSize = winsize;
    LimitSeqNo = winsize + 1;
    RxmtInterval = delay;
  }

  // This routine will be called whenever the upper layer at the sender [A]
  // has a message to send. It is the job of your protocol to insure that
  // the data in such a message is delivered in-order, and correctly, to
  // the receiving upper layer.
  protected void aOutput(Message message) {
    System.out.println("Message from upper layer send to A");
    if (nextSeqExpected < base + WindowSize) { // still space to send more packets
      Packet p =
          new Packet(nextSeqExpected, 0, checkSum(nextSeqExpected, 0, message.getData()),
              message.getData());
      beingSent[nextSeqExpected] = p;
      toLayer3(A, p);
      if (base == nextSeqExpected) {
        startTimer(A, RxmtInterval);
        startTime = getTime();
      }
      nextSeqExpected = (nextSeqExpected + 1) % LimitSeqNo;
    } else { // if the number of packets out equals the window size put it in toBeSent
      if (toBeSent.size() < bufferLimit) {
        toBeSent.add(message);
      }
    }
  }

  // This routine will be called whenever a packet sent from the B-side
  // (i.e. as a result of a toLayer3() being done by a B-side procedure)
  // arrives at the A-side. "packet" is the (possibly corrupted) packet
  // sent from the B-side.
  protected void aInput(Packet packet) {
    if (packet != null) {
      System.out.println("Packet Received by A");
      if (packet.getChecksum() == checkSum(packet.getSeqnum(), packet.getAcknum(),
          packet.getPayload())) {
        System.out.println("Packet was not corrupted");
        if (packet.getAcknum() >= base && packet.getAcknum() < nextSeqExpected) { // makes sure we
                                                                                  // were expecting
                                                                                  // this packet
          stopTimer(A);
          endTime = getTime();
          timeCounter++;
          timeSum += endTime - startTime;
          base = (packet.getAcknum() + 1) % LimitSeqNo;
          beingSent[packet.getSeqnum()] = null;
          // if toBeSent has elements and more elements can be sent, keep sending elements.
          while (toBeSent.size() > 0 && nextSeqExpected < base + WindowSize) {
            if (inserted == false) {
              inserted = true;
            }
            Message m = toBeSent.remove(0);
            Packet p =
                new Packet(packet.getSeqnum() + 1, base, checkSum(packet.getSeqnum() + 1, base,
                    m.getData()), m.getData());
            beingSent[p.getSeqnum()] = p;
            toLayer3(A, p);
            nextSeqExpected = (nextSeqExpected + 1) % LimitSeqNo;
          }
          if (inserted) {
            startTimer(A, RxmtInterval);
            startTime = getTime();
            inserted = false;
          }

        }
      }
    } else {
      corruptedPackets++;
      System.out.println("Corruption detected: Wait for timeout to resend Packet");
    }
  }


  // This routine will be called when A's timer expires (thus generating a
  // timer interrupt). You'll probably want to use this routine to control
  // the retransmission of packets. See startTimer() and stopTimer(), above,
  // for how the timer is started and stopped.
  protected void aTimerInterrupt() {
    lostPackets++;
    System.out.println("Timer Interrupted: Resend all packets who have not been Acknowledged");
    for (Packet p : beingSent) { // resends all packets that were being sent.
      if (p != null) {
        toLayer3(A, p);
        retransmissions++;
      }
    }
    startTimer(A, RxmtInterval);
    startTime = getTime();
  }

  // This routine will be called once, before any of your other A-side
  // routines are called. It can be used to do any required
  // initialization (e.g. of member variables you add to control the state
  // of entity A).
  protected void aInit() {
    System.out.println("A is being initialized");
    beingSent = new Packet[LimitSeqNo]; // packets being sent across medium
    toBeSent = new ArrayList<Message>(); // buffer packets not sent yet
  }

  // This routine will be called whenever a packet sent from the B-side
  // (i.e. as a result of a toLayer3() being done by an A-side procedure)
  // arrives at the B-side. "packet" is the (possibly corrupted) packet
  // sent from the A-side.
  protected void bInput(Packet packet) {
    System.out.println("Packet received by B");
    if (packet != null) {
      int checksum = checkSum(packet.getSeqnum(), packet.getAcknum(), packet.getPayload());
      if (packet.getChecksum() == checksum) {
        if (packet.getSeqnum() == bExpectedSeq) { // checkSum right and expected seq found
                                                  // send ACK packet back
          toLayer5(packet.getPayload());
          origDataPackets++;
          System.out.println("Sending Acknowledgement back to A");
          b.setAcknum(packet.getSeqnum());
          b.setSeqnum(bExpectedSeq);
          b.setChecksum(checkSum(b.getSeqnum(), b.getAcknum(), ""));;
          beingSent[bExpectedSeq] = b;
          toLayer3(B, b);
          ackPackets++;
          bExpectedSeq = (bExpectedSeq + 1) % LimitSeqNo;
        }
      } else {
        corruptedPackets++;
        System.out.println("Corruption detected: Wait for timeout to resend Packet");
      }
    }
  }

  // This routine will be called once, before any of your other B-side
  // routines are called. It can be used to do any required
  // initialization (e.g. of member variables you add to control the state
  // of entity B).
  protected void bInit() {
    System.out.println("B is being initialized");
    bExpectedSeq = 1;
    bAck = -1;
    b = new Packet(0, bAck, checkSum(0, bAck, ""), "");
  }

  // Use to print final statistics
  protected void Simulation_done() {
    avgRTT = timeSum / timeCounter;

    System.out.println("+==========================+");
    System.out.println("|=======    DATA   ========|");
    System.out.println("+==========================+");
    System.out.println("Original Data Packets: " + origDataPackets);
    System.out.println("Retransmissions: " + retransmissions);
    System.out.println("ACK Packet: " + ackPackets);
    System.out.println("Avg RTT: " + avgRTT);
    System.out.println("Corrupted Packets: " + corruptedPackets);
    System.out.println("Lost Packets: " + lostPackets);
  }

}
