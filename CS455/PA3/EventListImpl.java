import java.util.Vector;

public class EventListImpl implements EventList
{
    private Vector<Event> data;
    
    /**
     * Class constructor
     */
    public EventListImpl()
    {
        data = new Vector<Event>();
    }
    
    /**
     * Adds an event to the list of events
     */
    public boolean add(Event e)
    {
	data.addElement(e);
        return true;
    }
    
    /**
     * Returns the next event that needs to be processed from the events list
     **/
    public Event removeNext()
    {
        if (data.isEmpty())
        {
            return null;
        }
    
        int firstIndex = 0;
        double first = ((Event)data.elementAt(firstIndex)).getTime();
        for (int i = 0; i < data.size(); i++)
        {
            if (((Event)data.elementAt(i)).getTime() < first)
            {
                first = ((Event)data.elementAt(i)).getTime();
                firstIndex = i;
            }
        }
        
        Event next = (Event)data.elementAt(firstIndex);
        data.removeElement(next);
    
        return next;
    }
        
    /**
     * Converts the events list to a string
     */
    public String toString()
    {
        return data.toString();
    }

    public double getLastPacketTime(double time, int entity)
    {        
        double t = time;
        for (int i = 0; i < data.size(); i++)
        {
            if ((((Event)(data.elementAt(i))).getType() ==  NetworkSimulator.FROM_LAYER2) &&
                (((Event)(data.elementAt(i))).getEntity() == entity)) {
                    t = ((Event)(data.elementAt(i))).getTime();
            }
        }    
        return t;
    }
    
    /**
     * Prints the events in the events list one after the other for debugging
     */
    public void printEventList() {
        System.out.printf("-------------\nEvent List Follows:\n");
        for (int i=0; i<data.size(); i++) {
            Event e = (Event) data.elementAt(i);
            System.out.printf("Event time: %f, type: %d, entity: %d\n", e.getTime(), e.getType(), e.getEntity());
        }
        System.out.printf("-------------\n");    
    }    
}