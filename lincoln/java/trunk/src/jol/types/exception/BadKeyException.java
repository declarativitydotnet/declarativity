package jol.types.exception;

public class BadKeyException extends Exception {
	private static final long serialVersionUID = 1L;

	public BadKeyException() {
		super("Bad Key");
	}

	public BadKeyException(String message) {
		super(message);
	}

	public BadKeyException(Throwable cause) {
		super(cause);
	}

	public BadKeyException(String message, Throwable cause) {
		super(message, cause);
	}
}
