package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.stomp.Frame.Frame;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.User;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<Frame> {
    private int connectionId;
    private Connections<Frame> connections;
    private boolean shouldTerminate = false;

    @Override
    public void start(int connectionId, Connections<Frame> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    @Override
    public void process(Frame message) {
        switch (message.getCommand()) {
            case "CONNECT":
                handleConnect(message);
                break;

            case "SEND":
                handleSend(message);
                break;
            case "SUBSCRIBE":
                handleSubscribe(message);

                break;
            case "UNSUBSCRIBE":
                handleUnsubscribe(message);

                break;
            case "DISCONNECT":
                handleDisconnect();
                break;

            default:
                break;
        }

    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    void handleDisconnect() {
        connections.disconnect(connectionId);
        shouldTerminate = true;
    }

    void handleConnect(Frame frame) {

        Map<String, String> headers = frame.getHeaders();
        Frame errorFrame;
        if (!connections.validVersion(headers.get("accept-version")) || !connections.validHost(headers.get("host"))) {
            errorFrame = new Frame("ERROR", headers, "Invalid frame");
            // connections.response(errorFrame);
            return;
        }
        String login = headers.get("login");
        String passcode = headers.get("passcode");
        if (!connections.isRegister(login, passcode)) {

            if (connections.usedLogin(login)) {
                errorFrame = new Frame("ERROR", headers, "Password is not valid");
                // connections.response(errorFrame);
                return;
            }
            if (connections.usedPasscode(passcode)) {
                errorFrame = new Frame("ERROR", headers, "Login is not valid");
                // connections.response(errorFrame);
                return;
            }

        } else {
            Map<String, String> frameHeaders = new HashMap<String, String>();
            frameHeaders.put("version", "1.2");
            Frame connected = new Frame("CONNECTED", frameHeaders, "");
            User user = new User(login, passcode);
            connections.addUser(user);
            connections.addConnection(user);

            //connections.response(connected);
        }
    }

    void handleSend(Frame frame) {
        Map<String, String> headers = frame.getHeaders();
        String destination = headers.get("destination");
        if (connections.isSubscribed(connectionId, destination)) {
            Frame msgToSend = new Frame("MESSAGE", headers, frame.getBody());
            connections.send(connectionId, msgToSend);
        } else {
            Frame msgToSend = new Frame("ERROR", headers, "User is not subscribed to this channel");
            connections.send(connectionId, msgToSend);
            connections.disconnect(connectionId);
            shouldTerminate = true;
        }

    }

    void handleSubscribe(Frame frame) {
        Map<String, String> headers = frame.getHeaders();
        String destination = headers.get("destination");
        String id = headers.get("id");
        connections.subscribeChannel(destination, connectionId, Integer.parseInt(id));
    }

    void handleUnsubscribe(Frame frame) {
        Map<String, String> headers = frame.getHeaders();
        String id = headers.get("id");
        connections.unsubscribeChannel(Integer.parseInt(id), connectionId);
    }
}