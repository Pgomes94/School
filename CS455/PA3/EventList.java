public interface EventList
{
    public boolean add(Event e);
    public Event removeNext();
    public String toString();    
    public double getLastPacketTime(double time, int entity);
}