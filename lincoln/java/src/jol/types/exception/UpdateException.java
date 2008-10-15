package jol.types.exception;


public class UpdateException extends Exception {
	private static final long serialVersionUID = 1L;

	public UpdateException(String msg) {
		super(msg);
	}
	public UpdateException(String msg, Exception e) {
		super(msg, e);
	}
}
