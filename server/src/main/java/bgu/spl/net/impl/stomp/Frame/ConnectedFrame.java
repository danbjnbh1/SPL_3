package bgu.spl.net.impl.stomp.Frame;

public class ConnectedFrame extends Frame {
    public ConnectedFrame() {
        super("CONNECTED");
        headers.put("version", "1.2");
    }

}
