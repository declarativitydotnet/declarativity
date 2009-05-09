package bfs.telemetry;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.regex.Pattern;

public class ProcessStats {
	public static enum SelfStatEntry {
		// auto-generated from linux's proc.txt:
		// perl -ne 'chomp;$i+=0; s/\s+/ /g; s/^\s+//g;s~^(\S+)\s*~($i, "~; print uc($1).$_."\"),\n"; $i++;'
		PID(0, "process id"),
		TCOMM(1, "filename of the executable"),
		STATE(2, "state (R is running, S is sleeping, D is sleeping in an uninterruptible wait, Z is zombie, T is traced or stopped)"),
		PPID(3, "process id of the parent process"),
		PGRP(4, "pgrp of the process"),
		SID(5, "session id"),
		TTY_NR(6, "tty the process uses"),
		TTY_PGRP(7, "pgrp of the tty"),
		FLAGS(8, "task flags"),
		MIN_FLT(9, "number of minor faults"),
		CMIN_FLT(10, "number of minor faults with child's"),
		MAJ_FLT(11, "number of major faults"),
		CMAJ_FLT(12, "number of major faults with child's"),
		UTIME(13, "user mode jiffies"),
		STIME(14, "kernel mode jiffies"),
		CUTIME(15, "user mode jiffies with child's"),
		CSTIME(16, "kernel mode jiffies with child's"),
		PRIORITY(17, "priority level"),
		NICE(18, "nice level"),
		NUM_THREADS(19, "number of threads"),
		IT_REAL_VALUE(20, "(obsolete, always 0)"),
		START_TIME(21, "time the process started after system boot"),
		VSIZE(22, "virtual memory size"),
		RSS(23, "resident set memory size"),
		RSSLIM(24, "current limit in bytes on the rss"),
		START_CODE(25, "address above which program text can run"),
		END_CODE(26, "address below which program text can run"),
		START_STACK(27, "address of the start of the stack"),
		ESP(28, "current value of ESP"),
		EIP(29, "current value of EIP"),
		PENDING(30, "bitmap of pending signals (obsolete)"),
		BLOCKED(31, "bitmap of blocked signals (obsolete)"),
		SIGIGN(32, "bitmap of ignored signals (obsolete)"),
		SIGCATCH(33, "bitmap of catched signals (obsolete)"),
		WCHAN(34, "address where process went to sleep"),
		RESERVED_1(35, "(place holder)"),
		RESERVED_2(36, "(place holder)"),
		EXIT_SIGNAL(37, "signal to send to parent thread on exit"),
		TASK_CPU(38, "which CPU the task is scheduled on"),
		RT_PRIORITY(39, "realtime priority"),
		POLICY(40, "scheduling policy (man sched_setscheduler)"),
		BLKIO_TICKS(41, "time spent waiting for block IO");
		
		public final int offset;
		public final String name;
		private SelfStatEntry(int o, String n) { offset = o; name = n;}
	}
	static final Pattern whiteSpace = Pattern.compile("\\s+");
	Object cols[];
	public ProcessStats(String name) throws IOException {
		File f = new File("/proc/" + name + "/stat");
		BufferedReader in = new BufferedReader(new FileReader(f));
		String info = in.readLine();
		System.out.println(info);
		int oparen = info.indexOf('(');
		int cparen = info.indexOf(')');
		
		Integer pid = Integer.parseInt(info.substring(0, oparen-1));
		String cmd = info.substring(oparen+1, cparen);
		String rest = info.substring(cparen+2);
		System.out.println(pid); System.out.println(cmd); System.out.println(rest);
		String tok[] = whiteSpace.split(rest);
		cols = new Object[3 + tok.length];
		cols[0] = pid;
		cols[1] = cmd;
		cols[2] = tok[0];
		for(int i = 1; i < tok.length; i++) {
			if(tok[i].length() > 19) {
				cols[i+3] = tok[i]; // don't try to parse this integer; it may be greater than maxint.
			} else {
				cols[i+3] = Long.parseLong(tok[i]);
			}
		}
		in.close();
	}
	boolean verbose = true;
	@Override
    public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("<" + super.toString() + ": ");
		for(SelfStatEntry e: SelfStatEntry.values()) {
			sb.append("\n\t(" + e + ", " + cols[e.offset] + ( verbose ? ", " + e.name  : "" )+ ")");
		}
		sb.append(">");
		return sb.toString();
	}
	public static void main(String[] arg) {
		try {
	        ProcessStats t = new ProcessStats(arg[0]);
	        System.out.println(t);
        } catch (IOException e) {
	        e.printStackTrace();
        }
	}
}
