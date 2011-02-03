package jol.net.plaintext;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

import jol.core.Runtime;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.table.TableName;

public class Server {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// open a listener socket.
		ServerSocket sock;
		jol.core.Runtime j;

		try {
			sock = new ServerSocket(12345);
			j  = (jol.core.Runtime) jol.core.Runtime.create(Runtime.DEBUG_WATCH, System.err);
			j.install("user", new File(args[0]).toURI().toURL());
			j.evaluate(); // Install program arguments.
		} catch(Exception e) {
			e.printStackTrace();
			return;
		}
		
		while(true) {
			try {
				Socket ss = sock.accept();
				BufferedReader ins = new BufferedReader(new InputStreamReader(ss.getInputStream()));
				String in;

				while(null != (in = ins.readLine())) {
					// TODO Call the jol parser, instead of using this crazy hack?
					String[] tok = in.split("\\s*[()]\\s*");
					String[] tok2 = tok[1].split("\\s*[,]\\s*");
					
					TableName name = new TableName("trace", tok[0]);
					@SuppressWarnings("rawtypes")
					Class[] c = j.catalog().table(name).types();
					List<Object> l = new ArrayList<Object>();
					
					for (int i = 0; i < tok2.length; i++) {
						if(c[i] == String.class) {
							// XXX what about strings that contain ','?
							l.add(tok2[i]);
						} else if(c[i] == Integer.class) {
							l.add(Integer.parseInt(tok2[i]));
						} else if(c[i] == Long.class) {
							l.add(Long.parseLong(tok2[i]));
						} else if(c[i] == Short.class) {
							l.add(Short.parseShort(tok2[i]));
						} else if(c[i] == Boolean.class) {
							l.add(Boolean.parseBoolean(tok2[i]));
						} else if(c[i] == Character.class) {
							if(tok2[i].length() == 1) {
								l.add(tok2[i].charAt(0));
							} else {
								throw new UnsupportedOperationException("Could not convert '" + tok2[i] + "' to a char, as it is more than one character long");
							}
						} else if(c[i] == Byte.class) {
							if(tok2[i].length() == 1) {
								l.add((byte)tok2[i].charAt(0));
							} else {
								// XXX what about UTF16 chars greater than 255?
								throw new UnsupportedOperationException("Could not convert '" + tok2[i] + "' to a byte, as it is more than one character long");
							}
						} else if(c[i] == Float.class) {
							l.add(Float.parseFloat(tok2[i]));
						} else if(c[i] == Double.class) {
							l.add(Double.parseDouble(tok2[i]));
						} else {
							throw new UnsupportedOperationException("Unknown column type encountered in schema.  I only understand the JOL basic types (String, Integer, Long, Short, Boolean, Character, Byte, Float, and Double).  Also, String and Byte support is buggy.");
						}
					}
					Tuple tup = new Tuple(l);
					TupleSet insertions = new BasicTupleSet(tup);
					
					j.schedule("trace", name, insertions, null);
					j.evaluate();
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
}
