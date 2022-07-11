import java.util.concurrent.atomic.AtomicMarkableReference;

public class SetImpl<T extends Comparable<T>> implements Set<T> {

    class Node<T extends Comparable<T>> {

        T item;
        AtomicMarkableReference<Node<T>> next;

        public Node(T item, Node<T> next) {
            this.item = item;
            this.next = new AtomicMarkableReference<>(next, false);
        }
    }

    class Pair<T extends Comparable<T>> {
        Node<T> left;
        Node<T> right;

        public Pair(Node<T> left, Node<T> right) {
            this.left = left;
            this.right = right;
        }
    }

    private final Node<T> head;
    private final Node<T> tail;

    SetImpl() {
        tail = new Node<>(null, null);
        head = new Node<>(null, tail);
    }

    private Pair<T> find(T item) {
        find_again: while (true) {
            Node<T> pred = head;
            Node<T> curr = head.next.getReference();
            while (true) {
                boolean[] flag = { false };
                Node<T> next = curr.next.get(flag);
                if (flag[0]) {
                    if (pred.next.compareAndSet(curr, next, false, false)) {
                        continue find_again;
                    }
                    curr = next;
                }
                if (curr.item == null ? true : curr.item.compareTo(item) >= 0) {
                    return new Pair<>(pred, curr);
                }
                pred = curr;
                curr = next;
            }
        }
    }

    @Override
    public boolean add(T value) {
        Node<T> newNode = new Node<>(value, null);
        while (true) {
            Pair<T> pos = find(value);
            if (pos.right.item != null && pos.right.item.compareTo(value) == 0) {
                return false;
            }
            newNode.next.set(pos.right, false);
            if (pos.left.next.compareAndSet(pos.right, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            Pair<T> findRes = find(value);
            if (findRes.right.item == null || findRes.right.item.compareTo(value) != 0) {
                return false;
            }
            Node<T> successor = findRes.right.next.getReference();
            if (!findRes.right.next.compareAndSet(successor, successor, false, true)) {
                continue;
            }
            findRes.left.next.compareAndSet(findRes.right, successor, false, false);
            return true;
        }
    }

    @Override
    public boolean contains(T value) {
        Node<T> current = head.next.getReference();
        while (current.item != null && current.item.compareTo(value) < 0) {
            current = current.next.getReference();
        }
        return value == current.item && !current.next.isMarked();
    }

    @Override
    public boolean isEmpty() {
        while (head.next.getReference() != tail) {
            Node<T> current = head.next.getReference();
            if (!current.next.isMarked()) {
                return false;
            }
            head.next.compareAndSet(current, current.next.getReference(), false, false);
        }
        return true;
    }
}
