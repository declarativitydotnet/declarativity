package bfs.telemetry;

import jol.core.JolSystem;
import jol.core.Runtime;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Callback;
import jol.lang.plan.Predicate;

import bfs.Conf;

public class Dump {
    public static void main(String[] args) throws JolRuntimeException, UpdateException {
    	if (args.length != 1)
    		usage();

		JolSystem system = Runtime.create(Runtime.DEBUG_ALL, System.err, 12345);
        Dump dn = new Dump(system);
        dn.startSink("tcp:localhost:321321");
		system.start();
    }

    public static void dump(Runtime rt) {
        ddl(rt);


       for (Tuple t : rt.catalog().tuples()) {
            TableName tn = (TableName)t.value(0);

            if (tn.scope.equals("global") || tn.scope.equals("runtime") ||
                tn.scope.equals("network") || tn.scope.equals("compile")
            ) {
                continue;
            } 
            Table tab = rt.catalog().table(tn);
            
            //System.out.println("ok TAB: "+ tab.toString());
            if (tab.tuples() == null) {
                //System.out.println("oh dear, didn't find "+ t.value(0).toString() + " in the catalog\n");
                continue;
            }

            String nm = tab.name().toString().replace(":","_");
            for (Tuple r : tab.tuples()) {
               // System.out.println(nm + " : " + r.toString());
                 System.out.println("insert into " + nm + " values " + pTup(r) +";\n");
            }
        }
        // not a breakpoint; we simply exit  
        try {
            System.out.println("BREAK\n");
            System.in.read();
        } catch (Exception e) {
             
        }   

    }

    private static void ddl(Runtime r) {
        TableName tn = new TableName("global", "predicate");
        Table preds = r.catalog().table(tn);

        //HashMap map = new HashMap();


        for (Tuple t : preds.tuples()) {
            if ((Integer)t.value(2) == 0) {

                StringBuilder sb = new StringBuilder();
                Predicate p = (Predicate)t.value(4);
                sb.append("CREATE TABLE " + p.name().toString().replace(":","_") + " ( ");
                for (int i=0; i < p.arguments().size(); i++) {
                    //System.out.println(p.arguments().get(i).toString() + " " + p.schema().types()[i].toString() + ", ");
                    String attr = p.arguments().get(i).toString().replace("().","_").replace("\"","").replace("@","").replace("<","").replace(">","") + "_";
                    sb.append(attr + "  String ");
                    if (i < p.arguments().size()-1) { 
                        sb.append(",\n");
                    }
                }
                sb.append(");");
                System.out.println(sb.toString());
            }
        }
    }

    private static String pTup(Tuple t) {
        StringBuilder sb = new StringBuilder();
        sb.append("(");
        if (t.size() > 0) {
            sb.append("'" + t.value(0) + "'" );
            for (int i = 1; i < t.size(); i++) {
                Object element = t.value(i);
                sb.append(", ");
                sb.append(element == null ? "null" : "'" + element.toString() +"'");
            }
        }
        sb.append(")");
        return sb.toString();
    }


    private static void usage() {
        System.err.println("Usage: bfs.Dump dir_root");
        System.exit(1);
    }

    private JolSystem system;
	private String sink;
	private String identifier;

    public Dump(JolSystem s) {
		this.system = s;
		this.sink = sink;
		this.identifier = identifier;
    }

    public static Table.Type tt() {
        return Table.Type.EVENT;
    }


    public void startSink(String sink) throws JolRuntimeException, UpdateException {

        Callback copyCallback = new Callback() {
            @Override
            public void deletion(TupleSet tuples) {}

            @Override
            public void insertion(TupleSet tuples) {
                for (Tuple t : tuples) {
                    System.out.println("YO\n");
                    //Integer chunkId = (Integer) t.value(2);
                }
            }
        };

        
        this.system.install("breakpoint", ClassLoader.getSystemResource("tap/bp2.olg"));

        this.system.evaluate();
/*
        TableName tblName = new TableName("telemetry", "identity");
        TupleSet ident = new BasicTupleSet();
		ident.add(new Tuple("non-leaf", sink));
        this.system.schedule("telemetry", tblName, ident, null);
        this.system.evaluate();

        Table table = this.system.catalog().table(new TableName("telemetry", "messages"));
        table.register(copyCallback);
*/
	}

	public void startSource(String sink, String identifier) throws JolRuntimeException {

        this.system.install("telemetry", ClassLoader.getSystemResource("telemetry/telemetry.olg"));
        this.system.evaluate();

        /* Identify the data directory */
        TableName tblName = new TableName("telemetry", "identity");
        TupleSet ident = new BasicTupleSet();
		ident.add(new Tuple(identifier, sink));
        this.system.schedule("telemetry", tblName, ident, null);

    }

    public void shutdown() {
        this.system.shutdown();
    }

}
