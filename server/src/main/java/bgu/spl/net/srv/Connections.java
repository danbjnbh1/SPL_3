package bgu.spl.net.srv;

import java.io.IOException;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    void addConnection(int connectionId, ConnectionHandler<T> handler);

    void addConnection(User user);

    void subscribeChannel(String channel, int connectionId, int subscriptionId);

    void unsubscribeChannel(int subscriptionId, int connectionId);

    boolean isSubscribed(int connectionId, String channe);

    boolean validVersion(String version);

    boolean validHost(String host);

    boolean isRegister(String login, String passcode);

    boolean usedLogin(String login);

    boolean usedPasscode(String passcode);

    void addUser(User user);

   //TODO void response(T response);

}
