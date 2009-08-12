package jol.lang.plan;

import xtc.tree.Node;

public class DontCare extends Variable {
	public final static String DONTCARE = "_";
	public static Long ids = 0L;

	public DontCare(Node node, Class type) {
		super(node, "DC" + Long.toString(ids++), type);
	}
}
