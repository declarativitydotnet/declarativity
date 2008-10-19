package stasis.jni;

public class Stasis {
	// Stasis operations
	public static native void			init();
	public static native void			deinit();

	// Transaction initiation
	public static native long			begin();
	public static native void			commit(long xid);
	public static native void			abort(long xid);
	public static native void			prepare(long xid);
	
	// Record operations
	public static native long			record_type_read(long xid, long page, long slot);

	// LinearHashNTA
	public static native long[]		hash_create(long xid);
	public static native void			hash_delete(long xid, long page, long slot); // TODO need way to drop tables from lincoln...
	public static native long			hash_cardinality(long xid, long page, long slot);
	public static native byte[]		hash_insert(long xid, long page, long slot, byte[] keybytes, byte[] valbytes);
	public static native byte[]		hash_remove(long xid, long page, long slot, byte[] keybytes);
	public static native byte[]		hash_lookup(long xid, long page, long slot, byte[] keybytes);
	public static native byte[]		hash_iterator(long xid, long page, long slot);

	// Generic iterator interface 
	public static native void			iterator_close(byte[] it);
	public static native boolean	iterator_next(byte[] it);
	public static native byte[]		iterator_key(byte[] it);
	public static native byte[]		iterator_value(byte[] it);
	public static native void			iterator_tuple_done(byte[] it);

	static {
	    System.loadLibrary("stasisjni");
    }

	public static void main(String[] arg) {
		System.out.println("Tinit()");
		init();
		System.out.println("Stasis is running");
		System.out.println("Tbegin()");
		long xid = begin();
		System.out.println("Root record type is " + record_type_read(xid,1, 1));
		System.out.println("Tcommit()");
		commit(xid);
		deinit();
		System.out.println("Successfully shut down.  Exiting.");
	}
}
