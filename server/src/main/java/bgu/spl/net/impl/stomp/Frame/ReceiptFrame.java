package bgu.spl.net.impl.stomp.Frame;

import java.util.Map;

public class ReceiptFrame extends Frame {

    public ReceiptFrame(String receiptId) {
        super("RECEIPT");
        Map<String, String> headers = this.getHeaders();
        headers.put("receipt-id", receiptId);
    }
}
