package bgu.spl.net.srv;

import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArrayList;

public class ConnectionsImpl<T> implements Connections<T> {
    private final ConcurrentMap<Integer, ConnectionHandler<T>> connections;
    private final ConcurrentMap<String, List<Integer>> channels;
    private final ConcurrentMap<Integer, Map<String, Integer>> subscriptionIds;

    public ConnectionsImpl() {
        this.connections = new ConcurrentHashMap<>();
        this.channels = new ConcurrentHashMap<>();
        this.subscriptionIds = new ConcurrentHashMap<>();
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
}