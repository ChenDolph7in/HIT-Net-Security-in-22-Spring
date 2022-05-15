package Socket;

import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;

public class SonThread extends Thread{
    private String host;
    private int port;
    private ArrayList<IPPort> ports;

    public SonThread(String host, int port, ArrayList<IPPort> ports){
        this.host = host;
        this.port = port;
        this.ports = ports;
    }

    @Override
    public void run(){
        Socket socket = null;
        try {
            socket = new Socket(host, port);
            System.out.println("[host]"+host+" [port]"+port+"\t:succeed");
            IPPort ipport = new IPPort(host,port);
            ports.add(ipport);
            //return true;
        } catch (Exception e) {
            System.out.println("[host]"+host+" [port]"+port+"\t:failed");
            //return false;
        } finally {
            // Clean up
            if (socket != null) {
                try {
                    socket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
