package bgu.spl.net.srv;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicInteger;

import bgu.spl.net.impl.stomp.Frame.Frame;

public class ConnectionsImpl<T> implements Connections<T> {
    private final ConcurrentMap<Integer, ConnectionHandler<T>> connections;
    private final ConcurrentMap<String, List<Integer>> channels;
    private final ConcurrentMap<Integer, Map<String, Integer>> subscriptionIds;
    private final ConcurrentMap<String, User> users;
    private AtomicInteger messageIdCounter = new AtomicInteger();

    public ConnectionsImpl() {
        this.connections = new ConcurrentHashMap<>();
        this.channels = new ConcurrentHashMap<>();
        this.subscriptionIds = new ConcurrentHashMap<>();
        this.users = new ConcurrentHashMap<>();
    }

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> connectionHandler = connections.get(connectionId);
        if (connectionHandler != null) {
            connectionHandler.send(msg);
            return true;
        }
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        List<Integer> channelIds = channels.get(channel);
        if (channelIds != null) {
            for (Integer connectionId : channelIds) {
                send(connectionId, msg);
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        for (User user : users.values()) {
            if (user.getConnectionId() == connectionId) {
                user.setLoggedIn(false);
            }
        }
        connections.remove(connectionId);
        for (List<Integer> channelIds : channels.values()) {
            channelIds.remove(Integer.valueOf(connectionId));
        }
        subscriptionIds.remove(connectionId);

    }

    public void addConnection(int connectionId, ConnectionHandler<T> handler) {
        connections.put(connectionId, handler);
    }

    public void subscribeChannel(String channel, int connectionId, int subscriptionId) {

        channels.computeIfAbsent(channel, k -> new CopyOnWriteArrayList<>()).add(connectionId);
        subscriptionIds.computeIfAbsent(connectionId, k -> new ConcurrentHashMap<>()).put(channel, subscriptionId);

    }

    public void unsubscribeChannel(int subscriptionId, int connectionId) {
        Map<String, Integer> subscriptions = subscriptionIds.get(connectionId);
        if (subscriptions != null) {
            for (Map.Entry<String, Integer> entry : subscriptions.entrySet()) {
                if (entry.getValue() == subscriptionId) {
                    channels.get(entry.getKey()).remove(connections.get(connectionId));
                    break;
                }
            }
        }
    }

    public boolean isSubscribed(int connectionId, String channel) {
        Map<String, Integer> subscriptions = subscriptionIds.get(connectionId);
        return subscriptions != null && subscriptions.containsKey(channel);
    }

    public boolean validVersion(String version) {
        return version.equals("1.2");
    }

    public boolean validHost(String host) {
        return host.equals("stomp.cs.bgu.ac.il");
    }

    public boolean usedLogin(String login) {
        if (users.get(login) != null) {
            return true;
        }
        return false;
    }

    public boolean isRegister(String username, String passcode) {
        User user = users.get(username);
        return user != null && user.getPassword().equals(passcode);
    }

    public void register(String username, String passcode, int connectionId) {
        User user = new User(username, passcode, connectionId);
        users.put(username, user);
    }

    public void login(String username, int connectionId) {
        User user = users.get(username);
        user.login(connectionId);
    }

    public boolean isUserLoggedIn(String username) {
        User user = users.get(username);
        return user.isLoggedIn();
    }

    public boolean isLoggedIn(int connectionId) {
        for (User user : users.values()) {
            if (user.getConnectionId() == connectionId) {
                return user.isLoggedIn();
            }
        }
        return false;
    }

    public int getMessageIdCounter() {
        return messageIdCounter.incrementAndGet();
    }

    public int getSubscriptionId(int connectionId, String channel) {
        return subscriptionIds.get(connectionId).get(channel);
    }

}