package bgu.spl.net.impl.stomp.Frame;

import java.util.HashMap;
import java.util.Map;

public class MessageFrame extends Frame {
    public MessageFrame(int subscription, int messageId, String destination, String body) {
        super("MESSAGE", body);
        Map<String, String> headers = new HashMap<>();
        headers.put("subscription", Integer.toString(subscription));
        headers.put("message-id", Integer.toString(messageId));
        headers.put("destination", destination);
        this.headers = headers;
    }
}
