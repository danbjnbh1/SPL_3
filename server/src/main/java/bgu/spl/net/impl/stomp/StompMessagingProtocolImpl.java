package bgu.spl.net.impl.stomp;

import java.util.Map;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.stomp.Frame.ConnectedFrame;
import bgu.spl.net.impl.stomp.Frame.ErrorFrame;
import bgu.spl.net.impl.stomp.Frame.Frame;
import bgu.spl.net.impl.stomp.Frame.MessageFrame;
import bgu.spl.net.impl.stomp.Frame.ReceiptFrame;
import bgu.spl.net.srv.Connections;

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
                handleDisconnect(message);
                break;

            default:
                break;
        }

    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    void handleDisconnect(Frame message) {
        String receiptId = message.getHeaders().get("receipt");
        sendReceiptIfNeeded(receiptId);
        connections.disconnect(connectionId);
    }

    void closeConnection() {
        connections.disconnect(connectionId);
        shouldTerminate = true;
    }

    void sendReceiptIfNeeded(String receiptId) {
        if (receiptId != null) {
            Frame receiptFrame = new ReceiptFrame(receiptId);
            System.out.println(receiptFrame.toString());
            connections.send(connectionId, receiptFrame);
        }
    }

    boolean validateLogin() {
        if (!connections.isLoggedIn(connectionId)) {
            connections.send(connectionId, new ErrorFrame("Please login first", null, ""));
            return false;
        }

        return true;
    }

    void handleConnect(Frame frame) {
        Map<String, String> headers = frame.getHeaders();
        String receiptId = headers.get("receipt");
        if (!connections.validVersion(headers.get("accept-version")) || !connections.validHost(headers.get("host"))) {
            connections.send(connectionId,
                    new ErrorFrame("Invalid accept-version or host", receiptId, frame.toString()));
            closeConnection(); // ! check if needed
            return;
        }
        String username = headers.get("login");
        String passcode = headers.get("passcode");
        if (connections.isRegister(username, passcode)) {
            if (connections.isUserLoggedIn(username)) {
                connections.send(connectionId, new ErrorFrame(
                        "The client is already logged in, log out before trying again", receiptId,
                        frame.toString()));

                return;
            }
            connections.login(username, connectionId);
            connections.send(connectionId, new ConnectedFrame());
            sendReceiptIfNeeded(receiptId);
            return;
        }
        if (connections.usedLogin(username)) {
            connections.send(connectionId, new ErrorFrame("Wrong password", receiptId, frame.toString()));
            closeConnection();
            return;
        }
        connections.register(username, passcode, connectionId);
        connections.send(connectionId, new ConnectedFrame());
    }

    void handleSend(Frame frame) {
        if (!validateLogin()) {
            return;
        }
        Map<String, String> headers = frame.getHeaders();
        String destination = headers.get("destination").substring(1);
        String receiptId = headers.get("receipt");

        if (!connections.isSubscribed(connectionId, destination)) {
            connections.send(connectionId,
                    new ErrorFrame("User is not subscribed to the destination", receiptId, frame.toString()));
            return;
        }

        int messageId = connections.getMessageIdCounter();
        int subscriptionId = connections.getSubscriptionId(connectionId, destination);
        Frame response = new MessageFrame(subscriptionId, messageId, headers.get("destination"),
                headers.get("body"));
        connections.send(destination, response);
        sendReceiptIfNeeded(receiptId);
    }

    void handleSubscribe(Frame frame) {
        if (!validateLogin()) {
            return;
        }
        Map<String, String> headers = frame.getHeaders();
        String destination = headers.get("destination");
        String id = headers.get("id");
        connections.subscribeChannel(destination, connectionId, Integer.parseInt(id));
    }

    void handleUnsubscribe(Frame frame) {
        if (!validateLogin()) {
            return;
        }
        Map<String, String> headers = frame.getHeaders();
        String id = headers.get("id");
        connections.unsubscribeChannel(Integer.parseInt(id), connectionId);
    }
}