package jol.test;

import java.net.URL;

import jol.core.Runtime;
import jol.core.JolSystem;
import jol.core.Runtime.DebugLevel;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;

import org.junit.After;
import org.junit.Before;

public abstract class LocalTestBase {
    protected JolSystem sys;
    @Before
    abstract public void setup() throws JolRuntimeException, UpdateException;
    
    public void setup(String filename) throws JolRuntimeException, UpdateException {
        try { 
        	sys = Runtime.create(DebugLevel.ALL, System.err, 5000);

            URL u = ClassLoader.getSystemResource(filename);
            sys.install("unit-test", u);
            sys.evaluate();    // XXX: temporary workaround for runtime bug
        } catch(JolRuntimeException e) {
        	e.printStackTrace();
        	throw e;
        } catch(UpdateException e) {
        	e.printStackTrace();
        	throw e;
        }
    }

    @After
    public void shutdown() {
        sys.shutdown();
    }
}
