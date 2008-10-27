package gfs;

import jol.core.Runtime;
import jol.core.System;
import jol.types.exception.JolRuntimeException;

public class Main {
    private System system;

    Main() throws JolRuntimeException {
        system = Runtime.create(10000);
    }

    public static void main(String[] args) throws JolRuntimeException {
        new Main();
        java.lang.System.out.println("hello, world");
    }
}
