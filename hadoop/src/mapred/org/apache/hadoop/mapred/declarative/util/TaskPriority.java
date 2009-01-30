package org.apache.hadoop.mapred.declarative.util;

import org.apache.hadoop.mapred.JobPriority;

public class TaskPriority implements Comparable<TaskPriority> {
	
	private Integer catagory;
	
	private JobPriority jobPriority;
	
	private Long timestamp;
	
	public TaskPriority(Integer catagory, JobPriority jobPriority, Long timestamp) {
		this.catagory    = catagory;
		this.jobPriority = jobPriority;
		this.timestamp   = timestamp;
	}

	public boolean equals(Object o) {
		if (o instanceof TaskPriority) {
			return compareTo((TaskPriority)o) == 0;
		}
		return false;
	}
	
	public int compareTo(TaskPriority o) {
		if (this.catagory < o.catagory) 
			return -1;
		else if (this.catagory > o.catagory) 
			return 1;
		else if (this.jobPriority.compareTo(o.jobPriority) != 0)
			return this.jobPriority.compareTo(o.jobPriority);
		else if (this.timestamp < o.timestamp) 
			return -1;
		else if (this.timestamp > o.timestamp) 
			return 1;
		return 0;
	}

}
