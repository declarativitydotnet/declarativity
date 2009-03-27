package jol.types.exception;

import xtc.tree.Node;

public class CompileException extends RuntimeException {
    private static final long serialVersionUID = 1L;

	private Node node;

	public CompileException(String msg, Node n) {
		super(msg);
		this.node = n;
	}

	public Node node() {
		return this.node;
	}
}
