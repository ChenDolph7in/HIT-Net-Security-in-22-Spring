package Socket;

public class IPPort {
    private String host;//记录IP
    private int port;//记录端口

    public IPPort(String host, int port) {
        this.host = host;
        this.port = port;
    }

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }
}
