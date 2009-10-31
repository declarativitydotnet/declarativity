public class Main {
	private static final int BASE_PORT = 7000;

    public static void main(String[] args) {
    	if (args.length != 1)
    		usage();

    	int nClients = Integer.parseInt(args[0]);
    	if (nClients <= 0)
    		usage();

    	PerfRunner pr = new PerfRunner(nClients);
    	pr.run();
    }

	private static void usage() {
		System.out.println("Usage: java -jar paxos.jar nClients");
		System.exit(1);
	}

	public static int makePort(int i) {
		return BASE_PORT + i;
	}
}
