package jol.lang;

import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import jol.lang.plan.Predicate;
import jol.types.table.Aggregation;
import jol.types.table.Table;

public class Debugger {
	
	   public static boolean toDot(String program, String definitions, String dependencies) {
	    	try {
	    		StringBuilder sb = new StringBuilder();
	    		sb.append("digraph " + program + "{\n");
	    		sb.append("\tcompound=true;\n");
	    		sb.append("\tranksep=1.25;\n");
	    		sb.append("\tlabel=\"Stratification Graph Program " + program + "\", fontsize=20;\n");
	    		sb.append("\tnode [shape=plaintext, fontname=\"verdana\", fontsize=16];\n");
	    		sb.append("\tbgcolor=white;\n");
	    		sb.append("\tedge [arrowsize=1.5];\n");
	    		sb.append(definitions);
	    		sb.append(dependencies);
	    		sb.append("\n}");
	    		
				FileWriter graphFile = new FileWriter(program + ".dot");
				graphFile.write(sb.toString().replaceAll("\\\\\"", "\""));

				graphFile.close();
			} catch (IOException e) {
				return false;
			}
			return true;
	    }
	    
	    public static String dotEdge(Predicate consumer, int consumerStratum, 
	    		                     Predicate producer, int producerStratum) {
	    	String edge = consumer.name().dotLabel() + " -> " + producer.name().dotLabel();
	    	Table table = producer.context.catalog().table(producer.name());
	    	if (producer.notin()) {
	    		if (consumerStratum <= producerStratum) {
	    			edge += "[color = red, label = \"notin\"]";
	    		}
	    		else {
	    			edge += "[label = \"notin\"]";
	    		}
	    	}
	    	else if (table instanceof Aggregation) {
	    		jol.types.table.Aggregation aggregation = (Aggregation) table;
	    		if (consumerStratum <= producerStratum) {
	    			edge += "[color = red, label = \"";
	    		}
	    		else {
	    			edge += "[label = \"";
	    		}
	    		List<jol.lang.plan.Aggregate> aggregates = aggregation.aggregates();
	    		for (jol.lang.plan.Aggregate aggregate : aggregates) {
	    			edge += aggregate.toString() + ", ";
	    		}
	    		edge = edge.substring(0, edge.lastIndexOf(", "));
	    		edge += "\"]";
	    	}
	    	
	    	return edge + ";\n";
	    }

}
