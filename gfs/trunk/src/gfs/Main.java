package gfs;

import jol.core.Runtime;
import jol.core.System;
import jol.types.exception.P2RuntimeException;

public class Main {
    private System system;

    Main() throws P2RuntimeException {
        system = Runtime.create(10000);
    }

    public static void main(String[] args) throws P2RuntimeException {
        new Main();
        java.lang.System.out.println("hello, world");
    }
}
