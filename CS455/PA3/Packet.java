import java.io.*;

/**
 * This is the packet that is sent from one routing update process to another
 * via the call tolayer2() in the network simulator entitled
 * NetworkSimulator.java
 */
public class Packet {
    
    // ID of router sending this packet
    public int sourceid;
    
    // ID of router to which packet is being sent
    public int destid;
    
    // Min cost to router 0 ... 3
    public int[] mincost;
    
    /**
     * Class constructor with all attributes set
     */
    public Packet(int sourceid, int destid, int[] mincost) {
        this.sourceid = sourceid;
        this.destid = destid;
        if (mincost.length != 4) {
            System.out.printf("mincost array is invalid\n");
            System.out.printf("Unable to construct new packet properly\n");
            this.mincost = new int[4];
            this.mincost[0] = -1;
            this.mincost[1] = -1;
            this.mincost[2] = -1;
            this.mincost[3] = -1;
        } else {
            this.mincost = new int[4];
            this.mincost[0] = mincost[0];
            this.mincost[1] = mincost[1];
            this.mincost[2] = mincost[2];
            this.mincost[3] = mincost[3];
        }
    }


/**
 * Class constructor with no attributes set
 */
public Packet() {
    this.sourceid = -1;
    this.destid = -1;
    this.mincost = new int[4];
    this.mincost[0] = -1;
    this.mincost[1] = -1;
    this.mincost[2] = -1;
    this.mincost[3] = -1;
}

/**
 * Constructor that takes in a Packet as an input argument
 */
public Packet(Packet p) {
    if (p == null) {
        new Packet();
    } else {
        this.sourceid = p.sourceid;
        this.destid = p.destid;
        if (p.mincost.length != 4) {
            System.out.printf("mincost array is invalid\n");
            System.out.printf("Unable to construct new packet properly\n");
            this.mincost = new int[4];
            this.mincost[0] = -1;
            this.mincost[1] = -1;
            this.mincost[2] = -1;
            this.mincost[3] = -1;
        } else {
            this.mincost = new int[4];
            this.mincost[0] = p.mincost[0];
            this.mincost[1] = p.mincost[1];
            this.mincost[2] = p.mincost[2];
            this.mincost[3] = p.mincost[3];
        }
    }
}
}