package bgu.spl.net.impl.stomp.Frame;

public class ReceiptFrame extends Frame {
    private final String receiptId;
    public ReceiptFrame(String receiptId) {
        super("RECEIPT");
        this.receiptId = receiptId;
    }

    public String getReceiptId() {
        return receiptId;
    }
}
