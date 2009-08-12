package jol.types.table;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;

public class StasisSerializer {
	private final ByteArrayOutputStream outArray = new ByteArrayOutputStream();
    private ObjectOutputStream serializer;
    public StasisSerializer() {
    	ObjectOutputStream s;
    	try {
    		s = new ObjectOutputStream(outArray);
    	} catch(IOException e) {
    		e.printStackTrace();
    		s = null;
    	}
    	serializer = s;
    }

    public byte[] toBytes(Object o) {
		appendObject(o);
		return objectBytes();
	}

	public void appendObject(Object o) {
		try {
			serializer.writeObject(o);
		} catch(IOException e) {
			e.printStackTrace();
			throw new RuntimeException(e);
		}
	}
	public byte[] objectBytes() {
		try {
			serializer.close(); // does not affect outArray...
		} catch (IOException e) {
			e.printStackTrace();
			throw new RuntimeException(e);
		}
		byte[] ret = outArray.toByteArray(); // makes copy
		outArray.reset();  // reset outArray to beginning of stream for next time.
		try {
			serializer = new ObjectOutputStream(outArray);
		} catch (IOException e) {
			e.printStackTrace();
			throw new RuntimeException(e);
		}
		if (ret == null) {
			throw new RuntimeException("Impossible!!!");
		}
		return ret;
	}
}
