package bfs;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;

/**
 * This class provides the BoomFS client API. It communicates with both master
 * nodes, to obtain metadata, and data nodes, to read and write data.
 */
public class BfsClient {
	private JolSystem system;
	private int currentMaster;

	public BfsClient(int port) {
		try {
	        /* this shouldn't be a static member at all... */
	        Conf.setSelfAddress("tcp:localhost:" + port);

			this.system = Runtime.create(port);

	        this.system.install("gfs_global", ClassLoader.getSystemResource("gfs/gfs_global.olg"));
	        this.system.evaluate();
	        this.system.install("gfs", ClassLoader.getSystemResource("gfs/gfs.olg"));
	        this.system.evaluate();

	        updateMasterAddr();
	        this.system.start();
		} catch (JolRuntimeException e) {
			throw new RuntimeException(e);
		} catch (UpdateException e) {
			throw new RuntimeException(e);
		}
	}

    private void updateMasterAddr() throws JolRuntimeException {
        TupleSet master = new TupleSet();
        master.add(new Tuple(Conf.getSelfAddress(),
                             Conf.getMasterAddress(this.currentMaster)));
        try {
            this.system.schedule("gfs", MasterTable.TABLENAME, master, null);
        } catch (UpdateException e) {
            throw new JolRuntimeException(e);
        }
        this.system.evaluate();
    }

	public void createFile() {
		;
	}

	public boolean delete(String path) {
		return false;
	}

	public boolean rename(String oldPath, String newPath) {
		return false;
	}
}
