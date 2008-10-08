package jol.core;

public class SystemFactory {
	public static System makeSystem()
	{
		return new Runtime();
	}
}
