package p2.lang;

import p2.lang.plan.Assignment.AssignmentTable;
import p2.lang.plan.Predicate.PredicateTable;
import p2.lang.plan.Program.ProgramTable;
import p2.lang.plan.Rule.RuleTable;
import p2.lang.plan.Selection.SelectionTable;
import p2.types.element.Element;
import p2.types.exception.UpdateException;
import p2.types.table.Table;

public class Compile extends Element {
	
	private static ProgramTable program = null;
	
	private static RuleTable rule = null;
	
	private static PredicateTable predicate = null;
	
	private static SelectionTable selection = null;
	
	private static AssignmentTable assignment = null;
	

	public Compile(String id, String name) {
		super(id, name);
	}
	
	public static void initialize() {
		try {
			program = (ProgramTable) 
			          Table.create(new Table.Name("program", ProgramTable.class.getName()), 
				        		                       ProgramTable.SCHEMA, ProgramTable.PRIMARY_KEY);
			
			rule = (RuleTable) 
			          Table.create(new Table.Name("rule", RuleTable.class.getName()), 
				        		                       RuleTable.SCHEMA, RuleTable.PRIMARY_KEY);
				
			predicate = (PredicateTable) 
			          Table.create(new Table.Name("predicate", PredicateTable.class.getName()), 
				        		                       PredicateTable.SCHEMA, PredicateTable.PRIMARY_KEY);
			
			selection = (SelectionTable) 
			          Table.create(new Table.Name("selection", SelectionTable.class.getName()), 
				        		                       SelectionTable.SCHEMA, SelectionTable.PRIMARY_KEY);
			
			assignment = (AssignmentTable) 
			          Table.create(new Table.Name("assignment", AssignmentTable.class.getName()), 
				        		                       AssignmentTable.SCHEMA, AssignmentTable.PRIMARY_KEY);
		} catch (UpdateException e) {
			e.printStackTrace();
			java.lang.System.exit(0);
		}
	}

}
