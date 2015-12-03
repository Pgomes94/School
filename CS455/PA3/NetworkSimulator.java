import java.util.Vector;
import java.util.Enumeration;
import java.io.*;

public class NetworkSimulator {
    
    // possible events
    static final int FROM_LAYER2 = 2;
    static final int LINK_CHANGE = 10;
    
    // link changes flag
    private int LINKCHANGES = 0;
    
    // Variables and data structures
    private static EventList eventList;
    public static int TRACE;
    public static double clocktime;
    public static int seed;
    
    // Nodes
    private Node n0;
    private Node n1;
    private Node n2;
    private Node n3;
    
    // Connection costs between routers used for sanity checks
    private static int[][] connectcosts = new int[4][4];
    
    
    /**
     * Constructor
     */
    public NetworkSimulator(int t, int s) {
        // grab input arguments
        TRACE = t;
        seed = s;
        
        // initialize clocktime
        clocktime = 0.0;
        
        // create event list
        eventList = new EventListImpl();
    }
    
    
    /* initialize the simulator */
    void init() {
        
        
        /************************************************
         * Initialize random number generator
         *************************************************/
        //RNG.init();
        OSIRandom.init(seed);
        
        /***************************
         * Initialize time to 0.0
         ***************************/
        clocktime = 0.0;
        
        /*****************************
         * Create routers
         *****************************/
        n0 = new Node();
        n1 = new Node();
        n2 = new Node();
        n3 = new Node();

        
        /***************************
         * Initialize routers
         ***************************/
        // Initialize router 0
        int[] initialCosts0 = new int[4];
        initialCosts0[0] = 0; initialCosts0[1] = 1; initialCosts0[2] = 10; initialCosts0[3] = 7;
        n0.rtinit(0,initialCosts0);
        
        // Initialize router 1
        int[] initialCosts1 = new int[4];
        initialCosts1[0] = 1; initialCosts1[1] = 0; initialCosts1[2] = 1; initialCosts1[3] = n1.INFINITY;
        n1.rtinit(1,initialCosts1);
        
        // Initialize router 2
        int[] initialCosts2 = new int[4];
        initialCosts2[0] = 10; initialCosts2[1] = 1; initialCosts2[2] = 0; initialCosts2[3] = 2;
        n2.rtinit(2,initialCosts2);
        
        // Initialize router 3
        int[] initialCosts3 = new int[4];
        initialCosts3[0] = 7; initialCosts3[1] = n3.INFINITY; initialCosts3[2] = 2; initialCosts3[3] = 0;
        n3.rtinit(3,initialCosts3);
        
        /***************************
         * Initialize connection costs between routers
         * Used for performing some sanity checks in the tolayer2() method
         ***************************/
        connectcosts = new int[4][4];
        connectcosts[0][0]=0;  connectcosts[0][1]=1;  connectcosts[0][2]=3; connectcosts[0][3]=7;
        connectcosts[1][0]=1;  connectcosts[1][1]=0;  connectcosts[1][2]=1; connectcosts[1][3]=999;
        connectcosts[2][0]=3;  connectcosts[2][1]=1;  connectcosts[2][2]=0; connectcosts[2][3]=2;
        connectcosts[3][0]=7;  connectcosts[3][1]=999;  connectcosts[3][2]=2; connectcosts[3][3]=0;
        
        /***************************************
         * initialize future link changes
         ****************************************/
        if (LINKCHANGES==1)   {
            Event ev1 = new Event(10000.0, LINK_CHANGE, -1, null);
            eventList.add(ev1);
            Event ev2 = new Event(20000.0, LINK_CHANGE, -1, null);
            eventList.add(ev2);
        }
    }
    
    
    /**
     * Run simulator
     */
    public void runSimulator() {
        
        // initialize simulator
        init();
        
        // Begin the main loop
        while (true) {
            // Get our next event
            Event nextEvent = eventList.removeNext();
            if (nextEvent == null) {
                System.out.printf("\nSimulator terminated at t=%f, no packets in medium\n", clocktime);
                break;
            }
            
            if (TRACE >= 2) {
                System.out.printf("MAIN: rcv event, t=%.3f, at %d", nextEvent.getTime(),nextEvent.getEntity());
                if (nextEvent.getType() == FROM_LAYER2 ) {
                    System.out.printf(" src:%2d,",nextEvent.getPacket().sourceid);
                    System.out.printf(" dest:%2d,",nextEvent.getPacket().destid);
                    System.out.printf(" contents: %3d %3d %3d %3d\n",
                            nextEvent.getPacket().mincost[0], nextEvent.getPacket().mincost[1],
                            nextEvent.getPacket().mincost[2], nextEvent.getPacket().mincost[3]);
                }
                
            }
            
            // Advance the simulator's time
            clocktime = nextEvent.getTime();
            
            // Perform the appropriate action based on the event
            switch (nextEvent.getType()) {
                case FROM_LAYER2:
                    if (nextEvent.getEntity() == 0) {
                        n0.rtupdate( nextEvent.getPacket() );
                    } else if (nextEvent.getEntity() == 1) {
                        n1.rtupdate( nextEvent.getPacket() );
                    } else if (nextEvent.getEntity() == 2) {
                        n2.rtupdate( nextEvent.getPacket() );
                      } else if (nextEvent.getEntity() == 3) {
                        n3.rtupdate( nextEvent.getPacket() );
                      } else {
                        System.out.printf("Panic: unknown event entity\n"); System.exit(0);
                    }
                    break;
                    
                case LINK_CHANGE:
                    if (clocktime<10001.0) {
                        n0.linkhandler(1,20);
                        n1.linkhandler(0,20);
                    } else {
                        n0.linkhandler(1,1);
                        n1.linkhandler(0,1);
                    }
                    
                    break;
                    
                default:
                    System.out.printf("Panic: unknown event entity\n");
                    System.exit(0);
            }
        }
        System.out.println("Simulator terminated at time " + getTime());
    }
    
    
    /************************** TOLAYER2 ***************/
    public static void tolayer2(Packet packet) {
        
        /***************************************
         * be nice: check if source and destination id's are reasonable
         ****************************************/
        if (packet.sourceid<0 || packet.sourceid >3) {
            System.out.printf("WARNING: illegal source id in your packet, ignoring packet!\n");
            return;
        }
        if (packet.destid<0 || packet.destid >3) {
            System.out.printf("WARNING: illegal dest id in your packet, ignoring packet!\n");
            return;
        }
        if (packet.sourceid == packet.destid)  {
            System.out.printf("WARNING: source and destination id's the same, ignoring packet!\n");
            return;
        }
        if (connectcosts[packet.sourceid][packet.destid] == 999)  {
            System.out.printf("WARNING: source and destination not connected, ignoring packet!\n");
            return;
        }
        
        /************************************************
         * make a copy of the packet student just gave me since he/she may decide
         * to do something with the packet after we return back to him/her
         *************************************************/
        Packet mypktcopy = new Packet(packet);
        if (TRACE>2)  {
            System.out.printf("    TOLAYER2: source: %d, dest: %d\n              costs:", mypktcopy.sourceid, mypktcopy.destid);
            for (int i=0; i<4; i++)
                System.out.printf("%d  ",mypktcopy.mincost[i]);
            System.out.printf("\n");
        }
        
        /*****************************************************************
         * create future event for arrival of packet at the other side
         ******************************************************************/
        // compute the arrival time of packet at the other end.
        // medium can not reorder, so make sure packet arrives between 1 and 10
        // time units after the latest arrival time of packets
        // currently in the medium on their way to the destination
        double t = clocktime;
        t = eventList.getLastPacketTime(t, packet.destid);
        t = t + 2.0*OSIRandom.nextDouble();
        
        // Create event
        Event e = new Event(t, FROM_LAYER2, packet.destid, mypktcopy);
        
        if (TRACE>2)
            System.out.printf("    TOLAYER2: scheduling arrival on other side\n");
        
        // Add event to list of events
        eventList.add(e);
    }
    
    
    protected double getTime() {
        return clocktime;
    }
    
    protected void printEventList() {
        System.out.println(eventList.toString());
    }
    
}