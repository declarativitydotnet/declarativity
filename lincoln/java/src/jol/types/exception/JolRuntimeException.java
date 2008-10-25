package jol.types.exception;

public class JolRuntimeException extends Exception {
	private static final long serialVersionUID = 1L;

	public JolRuntimeException(String error) {
		super(error);
	}

	public JolRuntimeException(String error, Exception e) {
		super(error, e);
	}
}
