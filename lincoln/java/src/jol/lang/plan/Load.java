package jol.lang.plan;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.List;

import jol.lang.plan.Fact.FactTable;
import jol.types.basic.Tuple;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import xtc.tree.Location;
import jol.core.Runtime;

public class Load extends Clause {
	private final static String DEFAULT_DELIM = ",";
	
	private File file;
	
	private Table table;
	
	private String delim;

	public Load(Location location, File file, Table table, String delim) {
		super(location);
		this.file = file;
		this.table = table;
		this.delim = delim == null ? DEFAULT_DELIM : delim;
	}

	@Override
	public void set(Runtime context, String program) throws UpdateException {
		try {
			BufferedReader in = new BufferedReader(new FileReader(this.file));
			for (String line = in.readLine(); line != null; line = in.readLine()) {
				if (line.startsWith("//")) continue;
				else if (line.length() == 0) continue;
				String[] values = line.split(this.delim);
				context.catalog().table(FactTable.TABLENAME).force(new Tuple(program, table.name(), tuple(values)));
			}
		} catch (FileNotFoundException e) {
			throw new UpdateException(e.toString());
		} catch (IOException e) {
			throw new UpdateException(e.toString());
		}

	}
	
	private Tuple tuple(String[] values) throws UpdateException {
		List<Comparable> attributes = new ArrayList<Comparable>();
		Class[] types = this.table.types();
		if (values.length != types.length) {
			throw new UpdateException("ERROR: file loader value mismatch for table " + this.table.name());
		}
		for (int i = 0; i < values.length; i++) {
			attributes.add(value(types[i], values[i]));
		}
		return new Tuple(attributes);
	}
	
	private Comparable value(Class type, String value) throws UpdateException {
		if (type == String.class) {
			return value.equals("null") ? null : value.trim();
		}
		else if (type == Number.class) {
			if (type == Integer.class) {
				return Integer.parseInt(value);
			}
			else if (type == Double.class) {
				return Double.parseDouble(value);
			}
			else if (type == Float.class) {
				return Float.parseFloat(value);
			}
			else if (type == Long.class) {
				return Long.parseLong(value);
			}
			else if (type == Short.class) {
				return Short.parseShort(value);
			}
			else {
				throw new UpdateException("ERROR: load unknown number type " + type);
			}
		}
		else {
			try {
				Constructor constructor = type.getConstructor(String.class);
				return (Comparable) constructor.newInstance(value);
			} catch (Exception e) {
				throw new UpdateException("ERROR: loader - " + e.toString());
			}
		}
	}

	@Override
	public String toString() {
		return "load(" + file + ", " + table + ", \"" + delim + "\")";
	}

}
