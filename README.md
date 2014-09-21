### 15 puzzle solver for multicore machine


# 1. 'Naive' parallel A*
 The approach is to share open list and closed list for all threads. It requires a lock for these shared data structures. Actually, naive approach to parallelize 15 puzzle will NOT outperform serial A* (Ethan Burns et al. 2010). Threads spend most of time waiting & acquiring the mutex.
 According to Ethan, even with wait-free data structures (using compare&swap), it fails to outperform serial A*. Wait-free data structures enable to access open & closed lists concurrently. However, they need compare-and-swap atomic operator, which takes thousand times longer to execute. Therefore the wait-free implementation will not outperform serial A*.

# 2. Hash distribution A* (HDA*)
 In Hash distribution A* (HDA*), each thread has its own open list and closed list. Created nodes will be pushed to its owner. The owner is decided by hash function shared with all threads. As the pushing is asynchronous task, it takes no synchonization overhead. In fact, HDA* outperforms serial.

# 3. Expand-out-souring HDA*
 In some cases, HDA* suffers from unbalanced work distribution. There are many casues for uneveness.

---

### HDA* algorithm in Pseudo code

```
HDA*(start, goal)

  incomebuffer[start.zobrist] = {start}
  terminate = {false, false, false,...}
  incumbent = very big atomic integer
  
  initiate threads

    id = getThreadId
    closedlist  = empty
    openlist    = empty
    outgobuffer = {empty, empty, empty,...}

    while true

      if incomebuffer[id] is not empty
        lock(incomebuffer)
        tmp = incomebuffer.retrieveAll
	incomebuffer.clear
	unlock()
      
      if (openlist is empty) or (openlist.f < incumbent)
        terminate[id] = true
        if terminate is all true
	  break
        else
	  continue

      n = open.pop

      duplicate = closed.find(n)      
      if duplicate exist
        if duplicate.f <= n.f
	  discard n as duplicate
	  continue
      
      if n is goal state
        newPath = getPath(n)
	if newPath.length < incumbent
	  incumbent = newPath.length
	  path = newPath
	continue

      closed.add(n)

      for all possible operation op for n
        if op == n.pastop
	  continue
        
	nextEdge = apply(n, op)
	
	zobrist = nextEdge.zobrist
	if zobrist == id
	  open.push(next)
	else if incomebuffer[zobrist].trylock
	  lock(incomebuffer[zobrist])
	  incomebuffer[zobrist].push(nextEdge)
	  incomebuffer[zobrist].push(outgobuffer[zobrist])
	  unlock(incomebuffer[zobrist])
	  outgobuffer[zobrist].clear
	else 
	  outgobuffer[zobrist].push(nextEdge)
       
  return path

```
---

### Optimizations

TODO:

