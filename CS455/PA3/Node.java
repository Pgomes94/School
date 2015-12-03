import java.io.*;

/**
 * This is the class that students need to implement. The code skeleton is provided.
 * Students need to implement rtinit(), rtupdate() and linkhandler().
 * printdt() is provided to pretty print a table of the current costs for reaching
 * other nodes in the network.
 */ 
public class Node { 
    
    public static final int INFINITY = 9999;
    
    int[] lkcost;		/*The link cost between node 0 and other nodes*/
    int[][] costs;  		/*Define distance table*/
    int nodename;               /*Name of this node*/
    
    /* Class constructor */
    public Node() { }
    
    /* students to write the following two routines, and maybe some others */
    void rtinit(int nodename, int[] initial_lkcost) { 
      costs[0][0] = 0;
      costs[0][1] = 1;
      costs[0][2] = 10;
      costs[0][3] = 7;
      
      costs[1][0] = 1;
      costs[1][1] = 0;
      costs[1][2] = 1;
      costs[1][3] = INFINITY;
      
      costs[2][0] = 10;
      costs[2][1] = 1;
      costs[2][2] = 0;
      costs[2][3] = 2;
      
      costs[3][0] = 7;
      costs[3][1] = INFINITY;
      costs[3][2] = 2;
      costs[3][3] = 0;
    }    
    
    void rtupdate(Packet rcvdpkt) {  }
    
    
    /* called when cost from the node to linkid changes from current value to newcost*/
    void linkhandler(int linkid, int newcost) {  }    


    /* Prints the current costs to reaching other nodes in the network */
    void printdt() {
        switch(nodename) {
	case 0:
	    System.out.printf("                via     \n");
	    System.out.printf("   D0 |    1     2    3 \n");
	    System.out.printf("  ----|-----------------\n");
	    System.out.printf("     1|  %3d   %3d   %3d\n",costs[1][1], costs[1][2],costs[1][3]);
	    System.out.printf("dest 2|  %3d   %3d   %3d\n",costs[2][1], costs[2][2],costs[2][3]);
	    System.out.printf("     3|  %3d   %3d   %3d\n",costs[3][1], costs[3][2],costs[3][3]);
	    break;
	case 1:
	    System.out.printf("                via     \n");
	    System.out.printf("   D1 |    0     2 \n");
	    System.out.printf("  ----|-----------------\n");
	    System.out.printf("     0|  %3d   %3d \n",costs[0][0], costs[0][2]);
	    System.out.printf("dest 2|  %3d   %3d \n",costs[2][0], costs[2][2]);
	    System.out.printf("     3|  %3d   %3d \n",costs[3][0], costs[3][2]);
	    break;
	    
	case 2:
	    System.out.printf("                via     \n");
	    System.out.printf("   D2 |    0     1    3 \n");
	    System.out.printf("  ----|-----------------\n");
	    System.out.printf("     0|  %3d   %3d   %3d\n",costs[0][0], costs[0][1],costs[0][3]);
	    System.out.printf("dest 1|  %3d   %3d   %3d\n",costs[1][0], costs[1][1],costs[1][3]);
	    System.out.printf("     3|  %3d   %3d   %3d\n",costs[3][0], costs[3][1],costs[3][3]);
	    break;
	case 3:
	    System.out.printf("                via     \n");
	    System.out.printf("   D3 |    0     2 \n");
	    System.out.printf("  ----|-----------------\n");
	    System.out.printf("     0|  %3d   %3d\n",costs[0][0],costs[0][2]);
	    System.out.printf("dest 1|  %3d   %3d\n",costs[1][0],costs[1][2]);
	    System.out.printf("     2|  %3d   %3d\n",costs[2][0],costs[2][2]);
	    break;
        }
    }
}