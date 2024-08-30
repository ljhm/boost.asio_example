
captured variables become part of the lambda state (closure),
so their lifetimes and access are determined by how they are captured.

Calling do_read() or do_write() without an explicit pointer will use this by default.
Therefore, the calls are valid, and you can omit this when invoking those methods.

If you wanted to call them using self,
you would need to explicitly write self->do_read() and self->do_write().


The primary purpose of capturing self is to keep the session object
alive for the duration of the asynchronous callbacks.


Even if the captured variable is not explicitly used inside the lambda,
its presence in the capture list guarantees that the shared_ptr to the
object is kept alive for the lifetime of the lambda. This prevents the
object from being destroyed while the asynchronous
operation is pending.
