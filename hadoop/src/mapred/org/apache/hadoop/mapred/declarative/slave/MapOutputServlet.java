package org.apache.hadoop.mapred.declarative.slave;

import java.io.IOException;
import java.io.OutputStream;

import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.logging.Log;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.LocalDirAllocator;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MRConstants;
import org.apache.hadoop.mapred.MapTask;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskTracker;
import org.apache.hadoop.mapred.TaskTrackerImpl;
import org.apache.hadoop.util.StringUtils;

/**
 * This class is used in TaskTracker's Jetty to serve the map outputs
 * to other nodes.
 */
public class MapOutputServlet extends HttpServlet implements MRConstants {
  private static final int MAX_BYTES_TO_READ = 64 * 1024;
  @Override
  public void doGet(HttpServletRequest request, 
                    HttpServletResponse response
                    ) throws ServletException, IOException {
    String mapId = request.getParameter("map");
    String reduceId = request.getParameter("reduce");
    String jobId = request.getParameter("job");

    if (jobId == null) {
      throw new IOException("job parameter is required");
    }

    if (mapId == null || reduceId == null) {
      throw new IOException("map and reduce parameters are required");
    }
    ServletContext context = getServletContext();
    int reduce = Integer.parseInt(reduceId);
    byte[] buffer = new byte[MAX_BYTES_TO_READ];
    // true iff IOException was caused by attempt to access input
    boolean isInputException = true;
    OutputStream outStream = null;
    FSDataInputStream indexIn = null;
    FSDataInputStream mapOutputIn = null;
    
    try {
      outStream = response.getOutputStream();
      JobConf conf = (JobConf) context.getAttribute("conf");
      LocalDirAllocator lDirAlloc = 
        (LocalDirAllocator)context.getAttribute("localDirAllocator");
      FileSystem fileSys = 
        (FileSystem) context.getAttribute("local.file.system");

      // Index file
      Path indexFileName = lDirAlloc.getLocalPathToRead(
          TaskTracker.getJobCacheSubdir() + Path.SEPARATOR + 
          jobId + Path.SEPARATOR +
          mapId + "/output" + "/file.out.index", conf);
      
      // Map-output file
      Path mapOutputFileName = lDirAlloc.getLocalPathToRead(
          TaskTracker.getJobCacheSubdir() + Path.SEPARATOR + 
          jobId + Path.SEPARATOR +
          mapId + "/output" + "/file.out", conf);

      /**
       * Read the index file to get the information about where
       * the map-output for the given reducer is available. 
       */
      //open index file
      indexIn = fileSys.open(indexFileName);

      //seek to the correct offset for the given reduce
      indexIn.seek(reduce * MapTask.MAP_OUTPUT_INDEX_RECORD_LENGTH);
        
      //read the offset and length of the partition data
      final long startOffset = indexIn.readLong();
      final long rawPartLength = indexIn.readLong();
      final long partLength = indexIn.readLong();

      indexIn.close();
      indexIn = null;
        
      //set the custom "Raw-Map-Output-Length" http header to 
      //the raw (decompressed) length
      response.setHeader(RAW_MAP_OUTPUT_LENGTH, Long.toString(rawPartLength));

      //set the custom "Map-Output-Length" http header to 
      //the actual number of bytes being transferred
      response.setHeader(MAP_OUTPUT_LENGTH, 
                         Long.toString(partLength));

      //use the same buffersize as used for reading the data from disk
      response.setBufferSize(MAX_BYTES_TO_READ);
      
      /**
       * Read the data from the sigle map-output file and
       * send it to the reducer.
       */
      //open the map-output file
      mapOutputIn = fileSys.open(mapOutputFileName);
      
      // TODO: Remove this after a 'fix' for HADOOP-3647
      // The clever trick here to reduce the impact of the extra seek for
      // logging the first key/value lengths is to read the lengths before
      // the second seek for the actual shuffle. The second seek is almost
      // a no-op since it is very short (go back length of two VInts) and the 
      // data is almost guaranteed to be in the filesystem's buffers.
      // WARN: This won't work for compressed map-outputs!
      int firstKeyLength = 0;
      int firstValueLength = 0;
      if (partLength > 0) {
        mapOutputIn.seek(startOffset);
        firstKeyLength = WritableUtils.readVInt(mapOutputIn);
        firstValueLength = WritableUtils.readVInt(mapOutputIn);
      }
      

      //seek to the correct offset for the reduce
      mapOutputIn.seek(startOffset);
        
      long totalRead = 0;
      int len = mapOutputIn.read(buffer, 0,
                                 partLength < MAX_BYTES_TO_READ 
                                 ? (int)partLength : MAX_BYTES_TO_READ);
      while (len > 0) {
        try {
          outStream.write(buffer, 0, len);
          outStream.flush();
        } catch (IOException ie) {
          isInputException = false;
          throw ie;
        }
        totalRead += len;
        if (totalRead == partLength) break;
        len = mapOutputIn.read(buffer, 0, 
                               (partLength - totalRead) < MAX_BYTES_TO_READ
                               ? (int)(partLength - totalRead) : MAX_BYTES_TO_READ);
      }
      
    } catch (IOException ie) {
      TaskTrackerImpl tracker = 
        (TaskTrackerImpl) context.getAttribute("task.tracker");
      Log log = (Log) context.getAttribute("log");
      String errorMsg = ("getMapOutput(" + mapId + "," + reduceId + 
                         ") failed :\n"+
                         StringUtils.stringifyException(ie));
      log.warn(errorMsg);
      if (isInputException) {
        tracker.mapOutputLost(TaskAttemptID.forName(mapId), errorMsg);
      }
      response.sendError(HttpServletResponse.SC_GONE, errorMsg);
      throw ie;
    } finally {
      if (indexIn != null) {
        indexIn.close();
      }
      if (mapOutputIn != null) {
        mapOutputIn.close();
      }
    }
    outStream.close();
  }
}
