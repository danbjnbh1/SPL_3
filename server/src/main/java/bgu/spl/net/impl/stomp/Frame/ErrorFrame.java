package bgu.spl.net.impl.stomp.Frame;

import java.util.HashMap;
import java.util.Map;

public class ErrorFrame extends Frame {
    public ErrorFrame(String message, String receiptId, String body) {
        super("ERROR", body);
        Map<String, String> headers = new HashMap<>();
        headers.put("message", message);
        if (receiptId != null) {
            headers.put("receipt-id", "message-" + receiptId);
        }
        this.headers = headers;
    }
}
