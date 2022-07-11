import org.junit.Test;

import static org.junit.Assert.*;

public class SetTest {
    private Set<Integer> set;

    @Before
    public void prepare(){
        set = new SetImpl<>();
        set.add(1);
        set.add(3);
        set.add(5);
    }
    
    @Test
    public void checkAdd() {
        SetTest<Integer> set = new prepare();
        assertTrue(set.contains(1));
        assertFalse(set.contains(2));
        assertTrue(set.contains(3));
        assertFalse(set.contains(4));
        assertTrue(set.contains(5));
        assertFalse(set.contains(6));

        assertTrue(set.add(7));
        assertTrue(set.contains(7));
        assertFalse(set.add(7));
    }

    @Test
    public void checkRemove() {
        SetTest<Integer> set = new prepare();
        set.remove(1);
        assertFalse(set.contains(1));

        assertTrue(set.remove(3))
        assertFalse(set.remove(3))
    }

    @Test
    public void checkContains() {
        /// А зачем? При услоии того, что функционал проверяем выше? Разве что что-то такое:
        SetTest<Integer> set = new prepare();
        assertTrue(set.contains(1));
        set.remove(1);
        assertFalse(set.contains(1));
    }

    @Test
    public void checkIsEmpty() {
        SetTest<Integer> set = new prepare();
        assertFalse(set.isEmpty(1));
        set.remove(1);
        set.remove(3);
        set.remove(5);
        assertTrue(set.isEmpty(1));
    }
}
