package bfs.telemetry;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.regex.Pattern;

public class SystemStats {
	public static enum SystemStatEntry {
		USER(0, "normal processes executing in user mode"),
		NICE(1, "niced processes executing in user mode"),
		SYSTEM(2, "processes executing in kernel mode"),
		IDLE(3, "twiddling thumbs"),
		IOWAIT(4, "waiting for I/O to complete"),
		IRQ(5, "servicing interrupts"),
		SOFTIRQ(6, "softirq: servicing softirqs"),
		STEAL(7, "involuntary wait (usually host virtual machine) time"),
		GUEST(8, "guest virtual machine time"),
		LOAD_1(9, "1 minute load average"),
		LOAD_5(10, "5 minute load average"),
		LOAD_15(11, "15 minute load average");
		public final int offset;
		public final String name;
		private SystemStatEntry(int o, String n) {
			this.offset = o; this.name = n;
		}
	}

	public int getInt(SystemStatEntry e) {
		return cols[e.offset].intValue();
	}

	public float getFloat(SystemStatEntry e) {
		return cols[e.offset].floatValue();
	}
	
	static final Pattern whiteSpace = Pattern.compile("\\s+");
	Number cols[];
	public SystemStats() throws IOException {
		File f = new File("/proc/stat");
		BufferedReader in = new BufferedReader(new FileReader(f));
		String info = in.readLine();
		if(!info.startsWith("cpu  ")) { throw new IllegalStateException("Can't parse /proc/stat!"); }
		info = info.substring(5);
		String[] tok = whiteSpace.split(info);
		cols = new Number[tok.length + 3];
		for(int i = 0; i < tok.length; i++) {
			try {
				cols[i] = Long.parseLong(tok[i]);
			} catch(Exception e) {
				// deal w/ index out of bound, number format, etc by ignoring them
			}
		}
		in.close();
		f = new File("/proc/loadavg");
		in = new BufferedReader(new FileReader(f));
		info = in.readLine();
		tok = whiteSpace.split(info);
		cols[cols.length-3] = Double.parseDouble(tok[0]);
		cols[cols.length-2] = Double.parseDouble(tok[1]);
		cols[cols.length-1] = Double.parseDouble(tok[2]);
	}
	boolean verbose = true;
	@Override
    public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("<" + super.toString() + ": ");
		for(SystemStatEntry e: SystemStatEntry.values()) {
			sb.append("\n\t(" + e + ", " + cols[e.offset] + ( verbose ? ", " + e.name  : "" )+ ")");
		}
		sb.append(">");
		return sb.toString();
	}

	public int totalJiffies() {
		int sum = 0;
		for (SystemStatEntry e: SystemStatEntry.values()) {
			if (e.offset < SystemStatEntry.LOAD_1.offset) {
				sum += cols[e.offset].intValue();
			}
		}
		return sum;
	}
	public static void main(String[] arg) {
		try {
	        SystemStats t = new SystemStats();
	        System.out.println(t);
        } catch (IOException e) {
	        e.printStackTrace();
        }
	}
}
