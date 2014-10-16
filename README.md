# 15 puzzle solver for multicore machine


### 1. 'Naive' parallel A*
 The approach is to share open list and closed list for all threads. It requires a lock for these shared data structures. Actually, naive approach to parallelize 15 puzzle will NOT outperform serial A* (Ethan Burns et al. 2010). Threads spend most of time waiting & acquiring the mutex.
 According to Ethan, even with wait-free data structures (using compare&swap), it fails to outperform serial A*. Wait-free data structures enable to access open & closed lists concurrently. However, they need compare-and-swap atomic operator, which takes thousand times longer to execute. Therefore the wait-free implementation will not outperform serial A*.

### 2. Hash distribution A* (HDA*)
 In Hash distribution A* (HDA*), each thread has its own open list and closed list. Created nodes will be pushed to its owner. The owner is decided by hash function shared with all threads. As the pushing is asynchronous task, it takes no synchonization overhead. In fact, HDA* outperforms serial.

### 3. Expand-outsouring HDA*
 In some cases, HDA* suffers from unbalanced work distribution. There are many casues for uneveness.



---

### HDA* algorithm in Simplest Pseudo code

```
HDA*(start, goal)

  while true
    pull nodes from incomebuffer and put them into openlist
    pop a node from openlist
    check duplicate with closedlist
    if duplicate then continue
    expand the node and push new nodes to its distination

```

---

### Job Outsourcing HDA* algorithm in Simplest Pseudo code

```
JOHDA*(start, goal)

  spawin threads
    while true
      open.pushall(incomebuffer)
      n := open.pop
      if closed has find n.state with lesser or equal f value
        continue
      if n is goal
        incumbent := n
      if there is a thread, offshore with no significant job
        incomebuffer[offshore].push(n)
      for each neighbor of n
        incomebuffer[n.hash].push(neighbor)
     
```

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

### Expand-Outsourcing HDA* algorithm in Pseudo code

```
HDA*(start, goal)

  incomebuffer[start.zobrist] = {start}
  fvalues = {0, 0, 0,...}
  terminate = {false, false, false,...}
  incumbent = +infinitie
  
  initiate threads
    id = thread id
    closedlist  = empty
    openlist    = empty
    outgobuffer = {empty, empty, empty,...}

    while true
      if trylock(incomebuffer[id])
        openlist.addall(incomebuffer[id])
        unlock(incomebuffer[id])
      if openlist is empty OR openlist.f < incumbent
          terminate[id] = true
          if terminate is all true
  	    exit
          else  
            continue

      n = open.pop
      duplicate = closed.find(n)      
      if duplicate exist and duplicate.f <= n.f
        continue

      if n is goal state
        newPath = getPath(n)
        if newPath.length < incumbent
	  incumbent = newPath.length
          path = newPath
        continue

      if n.outsourcing == 0
        closed.add(n)

      if there is a thread, offshore, with no significant job 
        // The judge depends on each implementation
        
        if incomebuffer[offshore].trylock
          incomebuffer[offshore].push(n)
          incomebuffer[offshore].push(outgobuffer[offshore])
          unlock(incomebuffer[offshore])
          outgobuffer[offshore].clear
        else 
          outgobuffer[offshore].push(n)
      else
        for each neighbor to n
          zobrist = neighbor.zobrist
          if incomebuffer[zobrist].trylock
            incomebuffer[zobrist].push(neighbor)
            incomebuffer[zobrist].push(outgobuffer[zobrist])
            unlock(incomebuffer[zobrist])
            outgobuffer[zobrist].clear
          else 
            outgobuffer[zobrist].push(neighbor)
  
  return path

```


---

# Optimizations

TODO:

