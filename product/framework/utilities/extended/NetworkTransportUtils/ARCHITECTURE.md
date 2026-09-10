# NetworkTransportUtils architecture

## Component topology

```text
NetworkRuntime facade
└── shared RuntimeCore
    ├── io_context + workers
    ├── work guard
    └── active-lifecycle registry

TcpClient
└── connection actor strand

TcpServer
├── listener/control strand
├── public event strand
└── TcpServerSession [0..N]
    └── one socket actor strand per connection

UdpSocket
└── socket actor strand
```

There is intentionally no transport base class. The reusable pieces are small
and policy-free: `RuntimeCore`, callback/error helpers, and `QueueBudget`.
Protocol state machines remain explicit in their own implementations.

## Non-negotiable invariants

1. A socket, resolver, acceptor, or timer is initiated and mutated only by its
   owning actor strand.
2. Public server callbacks are invoked only by the server event strand. The
   callback strand never touches a session socket.
3. A successful lifecycle command registers exactly one runtime activity. While
   the public object remains alive, its single terminal event finishes that
   activity after the callback returns. Concurrent close/error paths atomically
   claim the terminal transition, so the first reason wins. Destruction
   suppresses only the callback, not actor cleanup or activity completion.
4. A successful send command transfers ownership and reserves capacity before
   returning. Capacity is released on confirmed write or terminal discard,
   including data accepted before its actor enqueue command gets to run.
5. At most one read and one write are in flight per TCP socket. At most one
   receive and one send are in flight per UDP socket. Receive storage belongs to
   one lifecycle generation, so stale canceled handlers cannot alias a restart.
6. User callback exceptions do not escape an Asio handler.
7. Object destruction disables subsequent user-callback dispatch but still
   schedules actor-owned cleanup. A callback already being dispatched may finish.

## Runtime shutdown

```text
Running --requestShutdown()--> Quiescing --last activity finishes--> Stopped
   |                              |
   | accepts user work            | rejects user work
   |                              | accepts internal cleanup/events
   +--unhandled handler fault-------------------------------> Faulted
```

The transition to `Quiescing`, participant snapshot, and new-activity admission
share one lock. A lifecycle therefore cannot be registered without its first
actor command while shutdown slips between the two.

Normal shutdown never calls `io_context::stop()`. It asks every registered
participant to close, permits internal posts while quiescing, and releases the
work guard only after the activity registry becomes empty. `stop()` is reserved
for a runtime fault where continued handler execution is unsafe.

Worker threads retain the execution state rather than the public facade.
Transports retain `RuntimeCore`; no transport stores a raw `NetworkRuntime&`.
This makes facade-first destruction safe and avoids a worker/core ownership
cycle.

## TCP server event ordering

An accepted socket starts paused:

```text
control strand: accept -> create/map paused session -> post onConnected
event strand:   onConnected -> post session.start()
session strand: queued send(s) from onConnected -> start -> read/write
```

If `onConnected` calls `send()`, both that send command and `start()` are posted
from the event strand to the same session strand in causal order. No socket
operation is initiated cross-strand.

Read delivery is an ownership handoff:

```text
session read completes
  -> pause reads
  -> post shared owning buffer to event strand
  -> invoke onDataReceived(ByteView)
  -> post buffer back to session strand
  -> start next read (or process terminal read error)
```

The event strand serializes callbacks from all sessions. A session terminal
event first returns to the control strand, which removes the session and posts
`onConnectionClosed`. During server stop, `onStopped` is posted only after the
last such event has already been posted, preserving the documented order.

## Backpressure

- TCP client: one `maxQueuedWriteBytes` budget.
- TCP server: one per-session budget plus one shared
  `maxTotalQueuedWriteBytes` budget.
- UDP: `maxQueuedSendBytes` plus `maxQueuedDatagrams`, so empty datagrams are
  bounded too.
- TCP server inbound: one handed-off read buffer per connection.
- UDP inbound: one reusable receive buffer; callback completion gates the next
  receive. The buffer and sender endpoint are replaced on each bind lifecycle.

Queue admission is synchronized at the public command boundary. Actor queues
contain only already-accounted buffers, which makes success/failure semantics
deterministic under concurrent callers.

## Extension guidance

- TLS should be a new protocol actor that reuses `RuntimeCore`, public errors,
  close information, and queue budgeting; it should not add TLS branches to the
  plain TCP actors.
- UDP multicast/broadcast belong in UDP-specific options and commands.
- Framing, reconnect policy, heartbeats, request correlation, and application
  retry are upper-layer concerns.
- New callbacks must be assigned an owning callback strand and included in the
  exactly-once/ordering tests before they become public API.
