package bgu.spl.net.impl.stomp;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.impl.stomp.Frame.Frame;

public class StompMessageEncoderDecoder implements MessageEncoderDecoder<Frame> {
    private byte[] bytes = new byte[1 << 10]; // start with 1k
    private int len = 0;

    @Override
    public Frame decodeNextByte(byte nextByte) {
        if (nextByte == '\u0000') {
            String result = new String(bytes, 0, len);
            len = 0;
            return parseFrame(result);
        }

        pushByte(nextByte);
        return null; // not a complete frame yet
    }

    @Override
    public byte[] encode(Frame message) {
        return message.toString().getBytes(); // uses UTF-8 by default
    }

    private void pushByte(byte nextByte) {
        if (len >= bytes.length) {
            bytes = Arrays.copyOf(bytes, len * 2);
        }

        bytes[len++] = nextByte;
    }

    private Frame parseFrame(String frameString) {
        String[] parts = frameString.split("\n\n", 2);
        String[] lines = parts[0].split("\n");
        String command = lines[0];
        Map<String, String> headers = new HashMap<>();
        for (int i = 1; i < lines.length; i++) {
            String[] header = lines[i].split(":", 2);
            headers.put(header[0], header[1]);
        }
        String body = parts.length > 1 ? parts[1] : "";
        return new Frame(command, headers, body);
    }
}