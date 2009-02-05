package org.apache.hadoop.fs.bfs;

import java.io.IOException;
import java.net.URI;

import jol.core.JolSystem;
import jol.types.exception.JolRuntimeException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.permission.FsPermission;
import org.apache.hadoop.util.Progressable;

public class BoomFileSystem extends FileSystem {
	private JolSystem context;

	@Override
	public FSDataOutputStream append(Path f, int bufferSize,
			Progressable progress) throws IOException {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public FSDataOutputStream create(Path f, FsPermission permission,
			boolean overwrite, int bufferSize, short replication,
			long blockSize, Progressable progress) throws IOException {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public boolean delete(Path f) throws IOException {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public boolean delete(Path f, boolean recursive) throws IOException {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public FileStatus getFileStatus(Path f) throws IOException {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public URI getUri() {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public Path getWorkingDirectory() {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public void initialize(URI name, Configuration conf) throws IOException {
        setConf(conf);
        int clientPort = conf.getInt("fs.bfs.clientPort", 5015);
        System.out.println("BFS#initialize(): client port = " + clientPort);

        try {
        	this.context = jol.core.Runtime.create(clientPort);
        } catch (JolRuntimeException e) {
        	throw new IOException(e);
        }
	}

	@Override
	public FileStatus[] listStatus(Path f) throws IOException {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public boolean mkdirs(Path f, FsPermission permission) throws IOException {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public FSDataInputStream open(Path f, int bufferSize) throws IOException {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public boolean rename(Path src, Path dst) throws IOException {
		throw new RuntimeException("not yet implemented");
	}

	@Override
	public void setWorkingDirectory(Path new_dir) {
		throw new RuntimeException("not yet implemented");
	}
}
