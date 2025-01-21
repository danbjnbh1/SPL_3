package bgu.spl.net.srv;

public class User {

    private int connectionId;
    private String username;
    private String password;
    private boolean isLoggedIn = false;

    public User(String username, String password, int connectionId) {
        this.username = username;
        this.password = password;
        this.connectionId = connectionId;
        this.isLoggedIn = true;
    }

    public String getUsername() {
        return username;
    }

    public String getPassword() {
        return password;
    }

    public int getConnectionId() {
        return connectionId;
    }

    public void login(int connectionId) {
        this.connectionId = connectionId;
        isLoggedIn = true;
    }

    public boolean isLoggedIn() {
        return isLoggedIn;
    }

    public void setLoggedIn(boolean loggedIn) {
        isLoggedIn = loggedIn;
    }
}
