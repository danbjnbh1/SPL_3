package bgu.spl.net.srv;

import java.util.Map;

public class User {
    
    private static int counterID = 0;
    private int connectionId;
    private String username;
    private String password;

    public User(String username, String password){
        this.username = username;
        this.password = password;
        this.connectionId = counterID++;
    }

    public String getUsername(){
        return username;
    }

    public String getPassword(){
        return password;
    }   

    public int getConnectionId(){
        return connectionId;
    }
    

}
