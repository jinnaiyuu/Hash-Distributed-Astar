This is the source code for the paper:
Jinnai Y, Fukunaga A. 2016. Abstract Zobrist Hashing: An Efficient Work Distribution Method for Parallel Best-First Search. Proc. 30th AAAI Conference on Artificial Intelligence (AAAI-16)

### Hash Distributed A*

Each thread has its own open list and closed list. Created nodes will be pushed to its owner. The owner is decided by hash function shared with all threads. As the pushing is asynchronous task, it takes no synchonization overhead.


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
