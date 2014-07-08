15 puzzle solver for multicore machine


1. 'Naive' parallel A*
 The approach is to share open list and closed list for all threads. It requires a lock for these shared data structures. Actually, naive approach to parallelize 15 puzzle will NOT outperform serial A* (Ethan Burns et al. 2010). Threads spend most of time waiting & acquiring the mutex.
 According to Ethan, even with wait-free data structures (using compare&swap), it fails to outperform serial A*. Wait-free data structures enable to access open & closed lists concurrently. However, they need compare-and-swap atomic operator, which takes thousand times longer to execute. Therefore the wait-free implementation will not outperform serial A*.

2. Hash distribution A*
 In Hash distribution A* (HDA*), each thread has its own open list and closed list. Created nodes will be pushed to its owner. The owner is decided by hash function shared with all threads. As the pushing is asynchronous task, it takes no synchonization overhead. In fact, HDA* outperforms serial.

3. AMAZING NEW ALGORITHM (hopefully)
 UNDER CONSTRUCTION


