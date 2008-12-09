package gfs;

public interface DataProtocol {
    final byte READ_OPERATION = 1;
    final byte WRITE_OPERATION = 2;
    final byte DELETE_OPERATION = 3;
    final byte COPY_OPERATION = 4;
}
