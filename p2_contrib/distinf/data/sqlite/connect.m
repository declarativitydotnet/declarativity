function conn = connect(filename)
  
import java.sql.*;
import java.lang.*;

Class.forName('org.sqlite.JDBC');
conn = DriverManager.getConnection(['jdbc:sqlite:' filename]);

