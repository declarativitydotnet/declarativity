package jol.types.exception;

public class UpdateException extends Exception {
	private static final long serialVersionUID = 1L;

	public UpdateException(String msg) {
		super(msg);
	}

	public UpdateException(Throwable cause) {
		super(cause);
	}

	public UpdateException(String msg, Throwable cause) {
		super(msg, cause);
	}
}
