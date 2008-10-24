package org.apache.hadoop.mapred.declarative.table;

import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.declarative.util.Wrapper;

import jol.core.Runtime;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

/**
 *  The heartbeat table maintains the previous heartbeat
 *  response sent to each tracker. This allows the job
 *  tracker to replay a past response.
 */
public class HeartbeatTable extends ObjectTable {

	/** The table name */
	public static final TableName TABLENAME = new TableName("mapred", "heartbeat");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0);
	
	/** An enumeration of all clock table fields. */
	public enum Field{TRACKERNAME, ID, INTERVAL, RESPONSE};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,   // Tracker name
		Short.class,    // Tracker identifier
		Integer.class,  // Heartbeat interval
		Wrapper.class   // Tracker response
	};
	
	public HeartbeatTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
	}

}
