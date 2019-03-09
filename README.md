## This is a very basic Multi-Paxos implementation in C.
### For building and running:
  - make clean
  - make
  - Or you can only make a specific role, e.g., make proposer
  
In order to have a working system, we need at least 1 proposer, 2 acceptor, 1 learner, and 1 client.

For example, to start a client with id 1:

./client.sh 1 paxos.config

and the same for other roles.

### Description
Paxos is a protocol used to solve consensus in asynchronous systems. Simply put, consensus can be used by a set of processes that need to agree on a single value. More commonly though, processes need to agree on a sequence of totally ordered values - a problem known as atomic broadcast. In this project, we’ll use the Paxos protocol to implement atomic broadcast.
In our implementation, four roles need to be provided:
  - clients: submit values to proposers
  - proposers: coordinate Paxos rounds to propose values to be decided • acceptors: Paxos acceptors
  - learners: learn about the sequence of values as they are decided
  
The protocol should always guarantee safety. To guarantee liveness, one of the proposers should be elected as the leader. A simple leader election oracle can be implemented using heartbeats.

### Assumptions
We assume that the number of acceptors is 3, that is, the failure of 1 acceptor can be tolerated. Note that more than 1 acceptor failing should not violate safety, only liveness. In general, proposing and learning values should be possible as long as a majority of acceptors and at least 1 of each client, proposer and learner are alive.
We assume fail-stop processes. That is, processes fail by halting and do not recover. This allows the following simplifications:
  - no need to implement a recovery procedure for acceptors or learners 
  - all state can be kept in memory - no need to use stable storage

### TODO 
  - Use stable storage
  - Recovery procedure for roles
  - Timeouts
