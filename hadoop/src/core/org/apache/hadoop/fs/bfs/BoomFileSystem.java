package org.apache.hadoop.fs.bfs;

import bfs.BFSClient;

import java.io.IOException;
import java.net.URI;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.permission.FsPermission;
import org.apache.hadoop.util.Progressable;

public class BoomFileSystem extends FileSystem {
	private BFSClient bfs;
    private URI uri;
	private Path workingDir = new Path("/");

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
		return this.bfs.delete(getPathName(f));
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
		return this.uri;
	}

	@Override
	public void initialize(URI uri, Configuration conf) throws IOException {
        setConf(conf);

        int clientPort = conf.getInt("fs.bfs.clientPort", 5015);
        System.out.println("BFS#initialize(): client port = " + clientPort);
        this.bfs = new BFSClient(clientPort);
        this.uri = URI.create(uri.getScheme() + "://" + uri.getAuthority());
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
		return this.bfs.rename(getPathName(src), getPathName(dst));
	}

	@Override
	public Path getWorkingDirectory() {
		return this.workingDir;
	}

	@Override
	public void setWorkingDirectory(Path new_dir) {
		this.workingDir = makeAbsolute(new_dir);
	}

	private String getPathName(Path path) {
		checkPath(path);
		String result = makeAbsolute(path).toUri().getPath();
		return result;
	}

	private Path makeAbsolute(Path path) {
		if (path.isAbsolute())
			return path;

		return new Path(this.workingDir, path);
	}
}
