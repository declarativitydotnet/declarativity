package bfs.telemetry;

import java.io.BufferedReader;
import java.io.CharArrayReader;
import java.io.File;
import java.io.FileReader;

import jol.core.JolSystem;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.BasicTupleSet;
import jol.types.table.TableName;

public class LogEvents {
	private final File name;
	private class LogThread extends Thread {
		public boolean done = false;
		@Override
        public void run() {
//			System.err.println("Started log thread" + this);
			final TableName tableName =  new TableName("bfs_global", "error_log");
			int oldlen = 0;
			int lastNewline = -1;
			try {
				while(!done) {
					try{
						int curlen = (int)name.length();
						if(curlen > oldlen) {
							char[] buf = new char[(int)curlen - (lastNewline+1)];
							FileReader in = new FileReader(name);
							in.skip(lastNewline+1);
							in.read(buf, 0, buf.length);
							in.close();

							int i = buf.length-1;
							while(i >= 0 && buf[i] != '\n') { i--; }
							if(i >= 0 && buf[i] == '\n') lastNewline = lastNewline + 1 + i;

							if(i >= 0) {
								BufferedReader r = new BufferedReader(new CharArrayReader(buf, 0, i));
								String line;
								while(null != (line = r.readLine())) {

									if(runtime != null) {
										synchronized(runtime) {
//											System.err.println("Line: " + line);
											TupleSet s = new BasicTupleSet();
											s.add(new Tuple(line));
											runtime.schedule("bfs_global", tableName, new BasicTupleSet(tableName, s), null);
											runtime.evaluate();
										}
									}
								}
							}
							oldlen = curlen;
						} else if(curlen < oldlen) {
							// file was truncated?!?
							oldlen = 0;
							lastNewline = -1;
						} else {
							Thread.sleep(250);
						}
					}catch (InterruptedException e) {
						// ignore.
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		public LogThread() {
			super("Log monitor for " + name);
		}
	}
	private final LogThread logThread;
	private final JolSystem runtime;
	final boolean done = false;
	public LogEvents(JolSystem runtime, File name) {
		this.runtime = runtime;
		this.name = name;
		logThread = new LogThread();
		logThread.start();
	}
	public void shutdown() {
		logThread.done = true;
	}
	public static void main(String [] args) {
		LogEvents e = new LogEvents(null, new File(args[0]));
		while (e != null) {
			try {
				Thread.sleep(1000);
			} catch(InterruptedException ex) {}
		}
	}
}
