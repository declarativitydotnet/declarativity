/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.hadoop.mapred;

import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.LocalDirAllocator;
import org.apache.hadoop.fs.Path;

/**
 * Manipulate the working area for the transient store for maps and reduces.
 */ 
public class FileHandle {

  protected JobConf conf;
  protected JobID jobId;
  
  public FileHandle() {
  }

  public FileHandle(JobID jobId) {
    this.jobId = jobId;
  }

  protected LocalDirAllocator lDirAlloc = 
                            new LocalDirAllocator("mapred.local.dir");
  
  /** Return the path to local task output file created earlier
   * @param taskid a task id
   */
  public Path getOutputFile(TaskAttemptID taskid)
    throws IOException {
    return lDirAlloc.getLocalPathToRead(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), taskid.toString()) + "/file.out", conf);
  }

  /** Create a local task output file name.
   * @param taskid a task id
   * @param size the size of the file
   */
  public Path getOutputFileForWrite(TaskAttemptID taskid, long size)
    throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), taskid.toString()) + "/file.out", size, conf);
  }

  /** Return the path to a local task output index file created earlier
   * @param taskid a task id
   */
  public Path getOutputIndexFile(TaskAttemptID taskid)
    throws IOException {
    return lDirAlloc.getLocalPathToRead(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), taskid.toString()) + "/file.out.index", conf);
  }

  /** Create a local output index file name.
   * @param taskid a task id
   * @param size the size of the file
   */
  public Path getOutputIndexFileForWrite(TaskAttemptID taskid, long size)
    throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), taskid.toString()) + "/file.out.index", 
                       size, conf);
  }

  /** Return a local spill file created earlier.
   * @param taskid a map task id
   * @param spillNumber the number
   */
  public Path getSpillFile(TaskAttemptID taskid, int spillNumber)
    throws IOException {
    return lDirAlloc.getLocalPathToRead(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), taskid.toString())
                       + "/spill"  + spillNumber + ".out", conf);
  }

  /** Create a local spill file name.
   * @param taskid a task id
   * @param spillNumber the number
   * @param size the size of the file
   */
  public Path getSpillFileForWrite(TaskAttemptID taskid, int spillNumber, long size) 
  throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), taskid.toString())
                       + "/spill" +  spillNumber + ".out", size, conf);
  }

  /** Return a local task spill index file created earlier
   * @param taskid a task id
   * @param spillNumber the number
   */
  public Path getSpillIndexFile(TaskAttemptID taskid, int spillNumber)
    throws IOException {
    return lDirAlloc.getLocalPathToRead(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), taskid.toString())
                       + "/spill" +  spillNumber + ".out.index", conf);
  }

  /** Create a local task spill index file name.
   * @param taskid a task id
   * @param spillNumber the number
   * @param size the size of the file
   */
  public Path getSpillIndexFileForWrite(TaskAttemptID taskid, int spillNumber, long size) 
  throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), taskid.toString())
                       + "/spill" + spillNumber +  ".out.index", size, conf);
  }
  
  /**
   * Used by JBufferSink when accepting new input data that needs to be spilled to disk.
   * @param attempt The taskid that owns the sink.
   * @param task The taskid that is sending the data.
   * @param spillNumber The spill number
   * @param size The size of the spill file.
   * @return A path to the spill file.
   * @throws IOException
   */
  public Path getInputSpillFileForWrite(TaskAttemptID attempt, TaskID task, int spillNumber, long size) 
  throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), attempt.toString())
                       + "/spill_" + task + "_" + spillNumber + ".in", size, conf);
  }
  
  /**
   * Used by JBufferSink when accepting new input data that needs to be spilled to disk.
   * @param attempt The taskid that owns the sink.
   * @param task The taskid that is sending the data.
   * @param spillNumber The spill number
   * @param size The size of the spill file.
   * @return A path to the spill file.
   * @throws IOException
   */
  public Path getInputSpillIndexFileForWrite(TaskAttemptID attempt, TaskID task, int spillNumber, long size) 
  throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), attempt.toString())
                       + "/spill_" + task + "_" + spillNumber +  ".in.index", size, conf);
  }
  
  /** 
   * Used by the JBuffer class when generating a new output snapshot.
   * @param taskid The taskid that owns the JBuffer
   * @param snapNumber The snapshot number.
   * @param size The size of the output.
   * @return A path to the snapshot file.
   * @throws IOException
   */
  public Path getOutputSnapshotFileForWrite(TaskAttemptID taskid, int snapNumber, long size) 
  throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), taskid.toString())
                       + "/snapshot_" +  snapNumber + ".out", size, conf);
  }

  /** 
   * Used by the JBuffer class when generating a new output snapshot.
   * @param taskid The taskid that owns the JBuffer
   * @param snapNumber The snapshot number.
   * @param size The size of the output.
   * @return A path to the snapshot file.
   * @throws IOException
   */
  public Path getOutputSnapshotIndexFileForWrite(TaskAttemptID taskid, int snapNumber, long size) 
  throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), taskid.toString())
                       + "/snapshot_" + snapNumber +  ".out.index", size, conf);
  }

  
  /** 
   * Used by the JBufferSink class when receiving a new input snapshot.
   * @param taskid The taskid that owns the JBufferSink
   * @param task The taskid that sent the snapshot.
   * @param snapNumber The snapshot number.
   * @param size The size of the output.
   * @return A path to the snapshot file.
   * @throws IOException
   */
  public Path getInputSnapshotFileForWrite(TaskAttemptID attempt, TaskID task, int snapNumber, long size) 
  throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), attempt.toString())
                       + "/snapshot_" + task + "_" + snapNumber + ".in", size, conf);
  }
  
  /** 
   * Used by the JBufferSink class when receiving a new input snapshot.
   * @param taskid The taskid that owns the JBufferSink
   * @param task The taskid that sent the snapshot.
   * @param snapNumber The snapshot number.
   * @param size The size of the output.
   * @return A path to the snapshot index file.
   * @throws IOException
   */
  public Path getInputSnapshotIndexFileForWrite(TaskAttemptID attempt, TaskID task, int snapNumber, long size) 
  throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), attempt.toString())
                       + "/snapshot_" + task + "_" + snapNumber +  ".in.index", size, conf);
  }

  /** Return a local reduce input file created earlier
   * @param mapTaskId a map task id
   * @param reduceTaskId a reduce task id
   */
  public Path getInputFile(int mapId, TaskAttemptID reduceTaskId)
    throws IOException {
    return lDirAlloc.getLocalPathToRead(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), reduceTaskId.toString())
                       + "/map_" + mapId + ".out", conf);
  }

  /** Create a local reduce input file name.
   * @param mapTaskId a map task id
   * @param reduceTaskId a reduce task id
   * @param size the size of the file
   */
  public Path getInputFileForWrite(TaskID mapId, TaskAttemptID reduceTaskId, long size)
    throws IOException {
    return lDirAlloc.getLocalPathForWrite(TaskTracker.getIntermediateOutputDir(
                       jobId.toString(), reduceTaskId.toString())
                       + "/map_" + mapId.getId() + ".out",  size, conf);
  }

  /** Removes all of the files related to a task. */
  public void removeAll(TaskAttemptID taskId) throws IOException {
    conf.deleteLocalFiles(TaskTracker.getIntermediateOutputDir(
                          jobId.toString(), taskId.toString())
);
  }

  public void setConf(Configuration conf) {
    if (conf instanceof JobConf) {
      this.conf = (JobConf) conf;
    } else {
      this.conf = new JobConf(conf);
    }
  }
  
  public void setJobId(JobID jobId) {
    this.jobId = jobId;
  }

}
