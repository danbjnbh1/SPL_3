package bgu.spl.net.srv;

import java.io.IOException;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    void addConnection(int connectionId, ConnectionHandler<T> handler);

    void subscribeChannel(String channel, int connectionId, int subscriptionId);

    void unsubscribeChannel(int subscriptionId, int connectionId);

    boolean isSubscribed(int connectionId, String channe);

    boolean validVersion(String version);

    boolean validHost(String host);

    boolean isRegister(String username, String passcode);

    boolean isUsedUsername(String username);

    void register(String username, String passcode, int connectionId);

    void login(String username, int connectionId);

    public boolean isUserLoggedIn(String username);

    public int getMessageIdCounter();

    public int getSubscriptionId(int connectionId, String channel);

    public boolean isLoggedIn(int connectionId);
}
