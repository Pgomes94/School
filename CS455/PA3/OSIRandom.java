public class OSIRandom
{
    public static long seed;

    public static void init(int s)
    {
        seed = s;
	seed = (seed) & 0xFFFFFFFFL;
    }

    private static int nextInt()
    {
	seed = ((seed&0xFFFFFFFFL)*(1103515245&0xFFFFFFFFL)+12345)&0xFFFFFFFFL;
	return (int)(seed/65536)%32768;
    }

    public static double nextDouble()
    {
	return (double)nextInt()/32767;
    }
}