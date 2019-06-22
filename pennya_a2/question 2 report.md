**Question 2**
After taking an hour or two to figure out what exactly the question was asking, I think I finally figured it out.
I initially had it sending the same signal handler to both the parent and child processes and was wondering
why everything was working normally.

So, what I did was I created parent and child processes. In the parent I called sleep for 0 seconds to get
the child working. The parent then woke up and called the signal handler, and at the same time the child
sent the same signal handler to the parent that had already received the signal. The handler function also
sleeps for like 5 seconds to ensure it's still working while the next signal is sent. What I noticed was
that the parent finished the original signal it called, while ignoring the child's signal. So, it seems
that while a process is handling a signal of a certain type, it cannot handle the exact same signal again
so it blocks the signal completely, and just finishes it's original call. The same results occur even if I
set the parent to sleep for 1 second and the child sends the signal first. That is, the signal the child
sends works and the parent's signal gets blocked.

The man page for signal confirms what I experienced while running my program. According to the man page,
"The handled signal is unblocked when the function returns and the process continues from where it left off
when the signal occured." Because the handled signal is unblocked when the function returns, it's safe to 
say that the handled signal IS blocked while the function is currently active. This makes sense considering
what I stated above. When the signal was already called, the second signal was completely ignored.

The man page for sigaction suggests that this function could potentially fix the problem. The man page
says, "A signal may also be blocked, in which case its delivery is postponed until it is unblocked". This
is unlike what happens with regular ol' signal which doesn't get delivered after the fact. So, if
sigaction is used instead of signal, I think it's safe to hypothesize that the outcome will be that the
original call to the signal will complete, then the second call will start.

Looking at [signal.c](https://github.com/torvalds/linux/blob/master/kernel/signal.c).

The first thing I noticed is that task_struct appears in a lot of functions. task_struct is declared in
[sched.h](https://github.com/torvalds/linux/blob/master/include/linux/sched.h) and contains a whole bunch
of other structs, most importantly a signal handler struct and a signal struct. Looking at the 
[signal struct](https://elixir.bootlin.com/linux/latest/source/include/linux/sched/signal.h#L85), it appears that there exists a linked list of some sort (list_head) and a queue (wait_queue_head_t). So it 
looks like signals have some sort of priority queue. There's also a sigpending struct that is used for
shared signal handling, and a hlist_head struct that is for multiprocess signals. hlist_head is probably
what is responsible for signals being sent when I called fork in my program. There's not much else of note
in this struct, so lets look at the [signal handler struct](https://elixir.bootlin.com/linux/latest/source/include/linux/sched/signal.h#L17). This struct also has a queue, but includes a sigaction and a siglock
as well. So, the sigaction is most likely what holds the handle part of signal(signal type, handler), and 
siglock must occur when a signal is interfered with. I think that when the kill function is called in my
program, the handle stored in sighand_struct is invoked. But, when I send a signal to the process that is
currently already handling that signal, the siglock is triggered, causing the signal to not occur. 