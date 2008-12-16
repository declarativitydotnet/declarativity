import java.sql.*;

public class sqlite {
  public static Connection connect(String filename) throws Exception {
    Class.forName("org.sqlite.JDBC");
    Connection conn = DriverManager.getConnection("jdbc:sqlite:"+filename);
    return conn;
  }
}
