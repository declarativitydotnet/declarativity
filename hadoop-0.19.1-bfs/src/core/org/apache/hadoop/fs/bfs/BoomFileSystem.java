package org.apache.hadoop.fs.bfs;

import bfs.BFSClient;
import bfs.BFSFileInfo;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;
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

		/**
		 * XXX: JOL currently requires a fixed local port to bind to. Since
		 * BoomFileSystem might be instanciated many times on the same machine,
		 * we can't just pick a single port. For now, randomly pick ports from
		 * within a range until we find a port we can bind to.
		 */
        int minPort = conf.getInt("fs.bfs.minPort", 12000);
        int maxPort = conf.getInt("fs.bfs.maxPort", 32000);
        if (maxPort <= minPort)
        	throw new IllegalStateException();

        Random r = new Random();
        while (true) {
        	int tryPort = r.nextInt(maxPort - minPort) + minPort;
        	System.out.println("BFS#initialize(): trying to create JOL @ port " + tryPort);
        	try {
        		// XXX: make the JOL driver thread a daemon thread. This is to
        		// workaround the apparent fact that the Hadoop task tracker code
        		// does not always invoke FileSystem#close().
        		this.bfs = new BFSClient(tryPort, true);
        	} catch (RuntimeException e) {
        		continue;
        	}
        	break;
        }

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

		/* XXX: not very efficient/clean, and not atomic either */
		if (overwrite)
			delete(path, false);

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

	/*
	 * XXX: we currently ignore "recursive"; we should instead raise an error if
	 * recursive is false and the target path is a non-empty directory.
	 */
	@Override
	public boolean delete(Path path, boolean recursive) throws IOException {
		System.out.println("BFS#delete() called for " + path);
		return this.bfs.delete(getPathName(path));
	}

	@Override
	public FileStatus getFileStatus(Path path) throws IOException {
		System.out.println("BFS#getFileStatus() called for " + path);
		BFSFileInfo bfsInfo = this.bfs.getFileInfo(getPathName(path));
		if (bfsInfo == null)
			throw new FileNotFoundException("File does not exist: " + path);

		FileStatus result = new FileStatus(bfsInfo.getLength(),
										   bfsInfo.isDirectory(),
										   bfsInfo.getReplication(),
										   bfsInfo.getChunkSize(),
										   0,    // modification time
										   0,    // access time
										   FsPermission.getDefault(),
										   bfsInfo.getOwner(),
										   bfsInfo.getGroup(),
										   new Path(bfsInfo.getPath()));
		return result;
	}

	@Override
	public FileStatus[] listStatus(Path path) throws IOException {
		System.out.println("BFS#listStatus() called for " + path);
		Set<BFSFileInfo> bfsListing = this.bfs.getDirListing(getPathName(path));
		if (bfsListing == null)
			return null;

		// XXX: ugly. We need to convert the BFS data structure to the Hadoop
		// file info format manually
		FileStatus[] result = new FileStatus[bfsListing.size()];
		int i = 0;
		for (BFSFileInfo bfsInfo : bfsListing) {
			// XXX: terrible hack. "Ls" file sizes are broken right now.
			FileStatus fStatus = getFileStatus(new Path(bfsInfo.getPath()));
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
			// We assume that the root directory ("/") always exists
			if (p.getParent() == null)
				continue;

			this.bfs.createDir(getPathName(p));
		}

		return true;
	}

	@Override
	public boolean rename(Path src, Path dst) throws IOException {
		System.out.println("BFS#rename() called for " + src + " => " + dst);
		if (getPathName(src).equals(getPathName(dst)))
			return true;

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

	@Override
	public void close() throws IOException {
		System.out.println("BoomFileSystem#close() called");
		super.close();
		this.bfs.shutdown();
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
