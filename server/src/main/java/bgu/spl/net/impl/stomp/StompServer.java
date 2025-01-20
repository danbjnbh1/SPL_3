package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("Please provide two arguments: <port> and <server_type> (tpc or reactor)");
            System.exit(1);
        }

        int port = Integer.parseInt(args[0]);
        String serverType = args[1];
        if (serverType.equals("tpc")) {
            Server.threadPerClient(port, () -> new StompMessagingProtocolImpl(), () -> new StompMessageEncoderDecoder())
                    .serve();
        } else if (serverType.equals("reactor")) {
            Server.reactor(Runtime.getRuntime().availableProcessors(), port,
                    () -> new StompMessagingProtocolImpl(),
                    () -> new StompMessageEncoderDecoder()).serve();
        } else {
            System.out.println("unsupported server type provider");
            System.exit(1);
        }

    }
}
