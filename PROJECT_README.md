# MLFQ Scheduler for xv6-RISC-V

This project implements a Multi-Level Feedback Queue (MLFQ) scheduler to replace xv6's default round-robin scheduler.

## Project Structure

```
xv6-labs-2025/
├── kernel/
│   ├── proc.c          # MLFQ implementation
│   ├── proc.h          # MLFQ data structures
│   ├── trap.c          # Timer interrupt & boosting
│   ├── param.h         # MLFQ constants
│   ├── sysproc.c       # getprocinfo syscall
│   └── defs.h          # Function declarations
├── user/
│   ├── mlfqtest.c      # Test program
│   └── user.h          # Syscall declarations
├── MLFQ Design Document.pdf  # Design specification
├── MLFQ Final Report.pdf     # Implementation report
└── README.md           # This file
```

## Features

✅ **4-Level Priority Queues** (0=highest, 3=lowest)  
✅ **Dynamic Priority Adjustment** based on CPU usage  
✅ **Starvation Prevention** via periodic boosting (every 200 ticks)  
✅ **Time Quanta:** 4, 8, 16, 32 ticks for levels 0-3  
✅ **getprocinfo() Syscall** for monitoring process state  
✅ **I/O-aware Scheduling** - keeps interactive processes responsive

## Building and Running

### Prerequisites
- RISC-V toolchain
- QEMU RISC-V emulator

### Build
```bash
cd xv6-labs-2025
make clean
make qemu
```

### Run Tests
```bash
# In xv6 shell:
$ mlfqtest
```

Expected output shows:
- CPU-bound process demoting from queue 0→1→2→3
- I/O-bound process staying at queue 0
- Both processes completing due to boosting

## MLFQ Configuration

| Queue Level | Priority | Time Quantum | Purpose |
|-------------|----------|--------------|---------|
| 0 | Highest | 4 ticks | Interactive/I/O-bound |
| 1 | High | 8 ticks | Mixed workload |
| 2 | Low | 16 ticks | CPU-bound |
| 3 | Lowest | 32 ticks | Heavy CPU-bound |

**Boost Interval:** 200 ticks (prevents starvation)

## System Call: getprocinfo()

```c
struct procinfo {
  int pid;
  int queue_level;      // Current queue (0-3)
  int ticks_used;       // Ticks consumed in current queue
  int quantum;          // Time quantum for current level
  char name[16];
  enum procstate state;
};

int getprocinfo(int pid, struct procinfo *info);
```

**Usage Example:**
```c
struct procinfo info;
if (getprocinfo(getpid(), &info) == 0) {
  printf("Process %d: queue=%d, ticks=%d\n",
         info.pid, info.queue_level, info.ticks_used);
}
```

## How It Works

### Process Lifecycle
1. **New process:** Starts at queue 0 with 4-tick quantum
2. **CPU usage:** Timer increments `ticks_used` every tick
3. **Demotion:** If `ticks_used >= quantum`, demote to next lower queue
4. **I/O blocking:** When waking from sleep, stay at current queue
5. **Boosting:** Every 200 ticks, all processes reset to queue 0

### Scheduling Algorithm
```
scheduler():
  Loop forever:
    Get next runnable process from highest priority non-empty queue
    If process found:
      Run it until it yields or blocks
    Else:
      Wait for interrupt

yield():
  If ticks_used >= quantum:
    Demote to next lower queue
  Re-queue at current level
  Switch to scheduler
```

## Testing

### Test Programs
- **mlfqtest.c** - Demonstrates CPU-bound vs I/O-bound behavior


## Implementation Details

### Key Functions
- `get_quantum(level)` - Returns time quantum for queue level
- `mlfq_enqueue(p, level)` - Add process to queue
- `mlfq_dequeue(level)` - Remove first process from queue
- `mlfq_remove(p)` - Remove process from current queue
- `mlfq_next()` - Get next runnable process
- `mlfq_demote(p)` - Move process to lower priority
- `mlfq_boost()` - Reset all processes to queue 0

### Modified xv6 Functions
- `scheduler()` - Uses MLFQ instead of round-robin
- `yield()` - Checks for demotion before re-queuing
- `userinit()` - Enqueues first process
- `kfork()` - Enqueues new child process
- `wakeup()` - Re-queues woken processes
- `usertrap()` - Increments ticks and triggers boost

## Performance Characteristics

- **Scheduling Time:** O(1) best case, O(n) worst case
- **Memory Overhead:** 4 pointers + 3 ints per process
- **Context Switch Overhead:** Same as original scheduler
- **Responsiveness:** Excellent for I/O-bound processes
- **Fairness:** Long-term fair due to boosting

## Troubleshooting

### Process not demoting?
- Check that timer interrupts are firing (`p->ticks_used` incrementing)
- Verify quantum values in `param.h`
- Ensure `yield()` is called on timer interrupt

### Processes starving?
- Verify `mlfq_boost()` is called every 200 ticks
- Check `boost_counter` in `trap.c`
- Ensure all processes are boosted to queue 0

### getprocinfo() returning -1?
- Verify process exists (check PID)
- Ensure syscall is registered in `syscall.c`
- Check `usys.pl` includes entry


## Authors

Saad Inam - 29068
Hassan Jabbar - 29060

## License

Same as xv6 (MIT License)
