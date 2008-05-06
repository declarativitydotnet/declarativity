package p2.core;

import p2.exec.Query.QueryTable;
import p2.types.exception.UpdateException;
import p2.types.table.Index;
import p2.types.table.Table;
import p2.types.table.Index.IndexTable;
import p2.types.table.Table.Catalog;

public class System {
	
	private static Catalog catalog;
	
	private static IndexTable index;
	
	private static QueryTable query;

	public static void initialize() {
		catalog = new Catalog();
		index   = new IndexTable();
		query   = new QueryTable();
	}
	
	public static Catalog catalog() {
		return catalog;
	}
	
	public static IndexTable index() {
		return index;
	}
	
	public static QueryTable query() {
		return query;
	}
}
