package org.apache.hadoop.mapred;

import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.CompressionCodecFactory;

public class InverseLineRecordReader implements RecordReader<LongWritable, Text> {
	private static final Log LOG = LogFactory.getLog(InverseLineRecordReader.class.getName());

	private CompressionCodecFactory compressionCodecs = null;
	private long start;
	private long pos;
	private long end;
	private InverseLineReader in;
	int maxLineLength;

	/**
	 * A class that provides a line reader from an input stream.
	 */
	public static class InverseLineReader {
		private static final int DEFAULT_BUFFER_SIZE = 64 * 1024;
		private int bufferSize = DEFAULT_BUFFER_SIZE;
		private FSDataInputStream in;
		private byte[] buffer;

		private int bufferPos;
		// the current position in the buffer
		private long position;

		private long start;

		private long end;

		/**
		 * Create a line reader that reads from the given stream using the 
		 * given buffer-size.
		 * @param in
		 * @throws IOException
		 */
		InverseLineReader(FSDataInputStream in, int bufferSize, long start, long end) {
			this.in = in;
			this.buffer     = new byte[bufferSize];
			this.bufferSize = bufferSize;
			this.start      = start;
			this.end        = end;
			this.position   = end; // Skip the final return/EOF marker
			this.bufferPos  = 0;
			System.err.println("INVERSE LINE READER: start = " + start + ", end = " + end);
		}

		/**
		 * Create a line reader that reads from the given stream using the
		 * <code>io.file.buffer.size</code> specified in the given
		 * <code>Configuration</code>.
		 * @param in input stream
		 * @param conf configuration
		 * @throws IOException
		 */
		public InverseLineReader(FSDataInputStream in, Configuration conf, long start, long end) throws IOException {
			this(in, conf.getInt("io.file.buffer.size", DEFAULT_BUFFER_SIZE), start, end);
		}

		/**
		 * Fill the buffer with more data.
		 * @return was there more data?
		 * @throws IOException
		 */
		boolean fillBuffer() throws IOException {
			if (this.start < this.position) {
				long length = (this.position - start) < bufferSize ? (this.position - start) : bufferSize;
				this.position -= length;
				in.seek(position);
				int bytesRead = 0;
				while (bytesRead < length) {
					bytesRead += in.read(buffer, bytesRead, (int) (length - bytesRead));
					if (bytesRead < length) {
						try { Thread.sleep(10);
						} catch (Throwable e) { }
					}
				}
				this.bufferPos = bytesRead;
				return bytesRead > 0;
			}
			return false;
		}

		/**
		 * Close the underlying stream.
		 * @throws IOException
		 */
		public void close() throws IOException {
			in.close();
		}

		/**
		 * Read from the InputStream into the given Text.
		 * @param str the object to store the given line
		 * @param maxLineLength the maximum number of bytes to store into str.
		 * @param maxBytesToConsume the maximum number of bytes to consume in this call.
		 * @return the number of bytes read including the newline
		 * @throws IOException if the underlying stream throws
		 */
		public int readLine(Text str, int maxLineLength, int maxBytesToConsume) throws IOException {
			str.clear();
			int totalBytes = 0;
			do {
				int bytes = 0;
				for ( ; bufferPos > 0; bytes++, totalBytes++) {
					bufferPos--; 
					if (buffer[bufferPos] == '\n' || buffer[bufferPos] == '\r') {
						if (totalBytes > 0) {
							if (bytes > 0) {
								str.prepend(buffer, bufferPos + 1, bytes);
							}
							return totalBytes + 1; // Count the current
						}
					}
					else if (totalBytes >= maxBytesToConsume) {
						str.prepend(buffer, bufferPos, bytes);
						return totalBytes + 1; // Count the current
					}
				}

				if (bytes > 0) {
					str.prepend(buffer, bufferPos, bytes);
				}
			} while (fillBuffer());
			return totalBytes;
		}

		/**
		 * Read from the InputStream into the given Text.
		 * @param str the object to store the given line
		 * @param maxLineLength the maximum number of bytes to store into str.
		 * @return the number of bytes read including the newline
		 * @throws IOException if the underlying stream throws
		 */
		public int readLine(Text str, int maxLineLength) throws IOException {
			return readLine(str, maxLineLength, Integer.MAX_VALUE);
		}

		/**
		 * Read from the InputStream into the given Text.
		 * @param str the object to store the given line
		 * @return the number of bytes read including the newline
		 * @throws IOException if the underlying stream throws
		 */
		public int readLine(Text str) throws IOException {
			return readLine(str, Integer.MAX_VALUE, Integer.MAX_VALUE);
		}

	}

	public InverseLineRecordReader(Configuration job, FileSplit split) throws IOException {
		this.maxLineLength = job.getInt("mapred.linerecordreader.maxlength", Integer.MAX_VALUE);
		start = split.getStart();
		end = start + split.getLength();
		final Path file = split.getPath();
		compressionCodecs = new CompressionCodecFactory(job);
		final CompressionCodec codec = compressionCodecs.getCodec(file);

		// open the file and seek to the start of the split
		FileSystem fs = file.getFileSystem(job);
		FSDataInputStream fileIn = fs.open(split.getPath());
		boolean skipFirstLine = false;
		if (codec != null) {
			throw new IOException("Inverse coded net yet supported");
			// in = new InverseLineReader(codec.createInputStream(fileIn), job, start, end);
			// TODO FIGURE OUT WTF end = Long.MAX_VALUE;
		} else {
			if (start != 0) {
				skipFirstLine = true;
				--start;
				fileIn.seek(start);
			}
		}
		if (skipFirstLine) {  // skip first line and re-establish "start".
			LineRecordReader.LineReader lr = new LineRecordReader.LineReader(fileIn, job);
			start += lr.readLine(new Text(), 0,
					(int)Math.min((long)Integer.MAX_VALUE, end - start));
		}

		in = new InverseLineReader(fileIn, job, start, end);
		this.pos = end;
	}

	public InverseLineRecordReader(FSDataInputStream in, long offset, long endOffset, int maxLineLength) {
		this.maxLineLength = maxLineLength;
		this.start = offset;
		this.pos = endOffset;
		this.end = endOffset;    
		this.in = new InverseLineReader(in, InverseLineReader.DEFAULT_BUFFER_SIZE, start, end);
	}

	public InverseLineRecordReader(FSDataInputStream in, long offset, long endOffset, Configuration job)  throws IOException{
		this.maxLineLength = job.getInt("mapred.linerecordreader.maxlength", Integer.MAX_VALUE);
		this.start = offset;
		this.pos = endOffset;
		this.end = endOffset;    
		this.in = new InverseLineReader(in, job, start, end);
	}

	public LongWritable createKey() {
		return new LongWritable();
	}

	public Text createValue() {
		return new Text();
	}

	/** Read a line. */
	public synchronized boolean next(LongWritable key, Text value)
	throws IOException {

		while (start < pos) {
			key.set(pos);

			int newSize = in.readLine(value, maxLineLength,
					Math.max((int)Math.min(Integer.MAX_VALUE, pos-start),
							maxLineLength));
			if (newSize == 0) {
				return false;
			}
			pos -= newSize;
			key.set(pos);
			if (newSize < maxLineLength) {
				return true;
			}

			// line too long. try again
			LOG.info("Skipped line of size " + newSize + " at pos " + pos);
		}

		return false;
	}

	/**
	 * Get the progress within the split
	 */
	public float getProgress() {
		if (start == end) {
			return 0.0f;
		} else {
			return Math.min(1.0f, (end - pos) / (float)(end - start));
		}
	}

	public  synchronized long getPos() throws IOException {
		return pos;
	}

	public synchronized void close() throws IOException {
		if (in != null) {
			in.close(); 
		}
	}

}
