package jol.lang.plan;

public class DontCare extends Variable {
	public final static String DONTCARE = "_";
	public static Long ids = 0L;

	public DontCare(Class type) {
		super("DC" + Long.toString(ids++), type);
	}
}
