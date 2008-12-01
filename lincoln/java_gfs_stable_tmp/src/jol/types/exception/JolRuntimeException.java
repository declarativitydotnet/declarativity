package jol.types.exception;

public class JolRuntimeException extends Exception {
	private static final long serialVersionUID = 1L;

	public JolRuntimeException(String message) {
		super(message);
	}

	public JolRuntimeException(Throwable cause) {
	    super(cause);
	}

	public JolRuntimeException(String message, Throwable cause) {
		super(message, cause);
	}
}
