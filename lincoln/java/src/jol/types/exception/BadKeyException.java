package jol.types.exception;

public class BadKeyException extends Exception {

	public BadKeyException() {
		super("Bad Key");
	}

	public BadKeyException(String message) {
		super(message);
		// TODO Auto-generated constructor stub
	}

	public BadKeyException(Throwable cause) {
		super(cause);
		// TODO Auto-generated constructor stub
	}

	public BadKeyException(String message, Throwable cause) {
		super(message, cause);
		// TODO Auto-generated constructor stub
	}

}
