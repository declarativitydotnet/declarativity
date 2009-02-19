package org.apache.hadoop.fs.bfs;

import bfs.BFSClient;
import bfs.BFSFileInfo;
import bfs.Conf;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;

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
	public void initialize(URI uri, Configuration conf) throws IOException {
        setConf(conf);

        int clientPort = conf.getInt("fs.bfs.clientPort", 5015);
        System.out.println("BFS#initialize(): client port = " + clientPort);
        this.bfs = new BFSClient(clientPort);
        this.uri = URI.create(uri.getScheme() + "://" + uri.getAuthority());
	}

	@Override
	public FSDataOutputStream append(Path path, int bufferSize,
			Progressable progress) throws IOException {
		System.out.println("BFS#append() called for " + path);
		BFSOutputStream bos = new BFSOutputStream(getPathName(path), this.bfs);
		return new FSDataOutputStream(bos, statistics);
	}

	@Override
	public FSDataOutputStream create(Path path, FsPermission permission,
			boolean overwrite, int bufferSize, short replication,
			long blockSize, Progressable progress) throws IOException {
		System.out.println("BFS#create() called for " + path);
		if (overwrite)
			throw new RuntimeException("cannot overwrite files");

		if (this.bfs.createFile(getPathName(path)) == false)
			throw new IOException("failed to create file " + path);

		return append(path, bufferSize, progress);
	}

	@Override
	public FSDataInputStream open(Path path, int bufferSize) throws IOException {
		System.out.println("BFS#open() called for " + path);
		return new FSDataInputStream(new BFSInputStream(getPathName(path), this.bfs));
	}

	@Override
	public boolean delete(Path path) throws IOException {
		return delete(path, true);
	}

	@Override
	public boolean delete(Path path, boolean recursive) throws IOException {
		System.out.println("BFS#delete() called for " + path);

		if (!recursive)
			throw new RuntimeException("non-recursive delete not implemented");

		return this.bfs.delete(getPathName(path));
	}

	@Override
	public FileStatus getFileStatus(Path path) throws IOException {
		BFSFileInfo bfsInfo = this.bfs.getFileInfo(getPathName(path));
		if (bfsInfo == null)
			throw new FileNotFoundException("File does not exist: " + path);

		FileStatus result = new FileStatus(bfsInfo.getLength(),
				bfsInfo.isDirectory(),
                bfsInfo.getReplication(),
                bfsInfo.getChunkSize(),
                0,    // modification time
                null, // permissions
                null, // owner
                null, // group
                new Path(bfsInfo.getName()));
		return result;
	}

	@Override
	public FileStatus[] listStatus(Path path) throws IOException {
		System.out.println("BFS#listStatus() called for " + path);
		Set<BFSFileInfo> bfsListing = this.bfs.getDirListing(getPathName(path));

		// XXX: ugly. We need to convert the BFS data structure to the Hadoop
		// file info format manually
		FileStatus[] result = new FileStatus[bfsListing.size()];
		int i = 0;
		for (BFSFileInfo bfsInfo : bfsListing) {
			FileStatus fStatus = new FileStatus(bfsInfo.getLength(),
												bfsInfo.isDirectory(),
					                            bfsInfo.getReplication(),
					                            bfsInfo.getChunkSize(),
					                            0,    // modification time
					                            null, // permissions
					                            null, // owner
					                            null, // group
					                            new Path(path, bfsInfo.getName()));
			result[i++] = fStatus;
		}

		return result;
	}

	@Override
	public boolean mkdirs(Path path, FsPermission permission) throws IOException {
		System.out.println("BFS#mkdirs() called for " + path);
		// This command should create all the directories in the given path that
		// don't already exist (i.e. it should function like "mkdir -p"). Since
		// BFS requires that we create directories one at a time, we start at
		// the root path and iterate
		path = makeAbsolute(path);
		List<Path> pathHierarchy = new ArrayList<Path>();
		for (Path p = path; p != null; p = p.getParent()) {
			pathHierarchy.add(p);
		}
		Collections.reverse(pathHierarchy);

		for (Path p : pathHierarchy) {
			this.bfs.createDir(getPathName(p));
		}

		return true;
	}

	@Override
	public boolean rename(Path src, Path dst) throws IOException {
		System.out.println("BFS#rename() called for " + src + " => " + dst);
		return this.bfs.rename(getPathName(src), getPathName(dst));
	}

	@Override
	public URI getUri() {
		return this.uri;
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
