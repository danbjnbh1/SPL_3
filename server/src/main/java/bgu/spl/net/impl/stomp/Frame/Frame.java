package bgu.spl.net.impl.stomp.Frame;

import java.util.Map;

public class Frame {
    private String command;
    private Map<String, String> headers;
    private String body;

    public Frame(String command, Map<String, String> headers, String body) {
        this.command = command;
        this.headers = headers;
        this.body = body;
    }

    public String getCommand() {
        return command;
    }

    public Map<String, String> getHeaders() {
        return headers;
    }

    public String getBody() {
        return body;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(command).append("\n");
        headers.forEach((key, value) -> sb.append(key).append(":").append(value).append("\n"));
        sb.append("\n").append(body).append("\u0000");
        return sb.toString();
    }

    public static Frame setErrorFrame(Frame frame, String message) {
        //TODO not accurate
        frame.command = "ERROR";
        frame.headers.put("receipt-id", "message-"+frame.headers.get("receipt-id"));
        frame.headers.put("message", "malformed frame received");
        frame.body = frame.toString();

        return frame;
        
        
    }
}