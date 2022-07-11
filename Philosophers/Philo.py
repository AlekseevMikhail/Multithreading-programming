from threading import Lock

class DiningPhilosophers:
    
    def __init__(self):
        self.locks = [Lock() for i in range(5)]

    def wantsToEat(self,
                   philosopher: int,
                   pickLeftFork: 'Callable[[], None]',
                   pickRightFork: 'Callable[[], None]',
                   eat: 'Callable[[], None]',
                   putLeftFork: 'Callable[[], None]',
                   putRightFork: 'Callable[[], None]') -> None:
        
        def cycle():
            pickLeftFork()
            pickRightFork()
            eat()
            putLeftFork()
            putRightFork()
        
        left_f, right_f = philosopher, (philosopher + 1) % 5
        
        if philosopher % 2:
            left_f, right_f = right_f, left_f
            
        with self.locks[left_f], self.locks[right_f]:
            cycle()
