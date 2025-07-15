So Let’s summarize the difference between select, poll, and epoll for I/O multiplexing. The select and poll should pass all the descriptors to the kernel to know what file descriptors the process can handle. On the other hand, the process only asks for the epoll instance without full file descriptors once the process enrolled them before. So kernel doesn’t need to scan all the file descriptors whenever the request comes from the process with an epoll instance.


Here’s a practical checklist of edge cases for a robust multiplexer/server, with steps to handle each. This will help you audit and improve your code.

---

## Multiplexer/Server Robustness Checklist

### 1. File Descriptor (FD) Exhaustion  
- **Checklist:**  
  - Do you close all sockets/FDS when done?
  - Do you check for errors from `socket()`, `accept()`, etc.?
- **Steps:**  
  - Always call `close(fd)` on unused/finished sockets.
  - Log and handle errors from FD-creating functions.
  - Consider setting a max connection limit.

//									DONE!!!!!!!!!!
---

### 2. Partial Reads/Writes
- **Checklist:**  
  - Do you loop on `recv()`/`send()` until all data is processed?
- **Steps:**  
  - Use a loop to read/write until the full buffer is handled.
  - Handle cases where `recv()`/`send()` returns less than requested.


//									DONE!!!!!!!!!!
---

### 3. Client Disconnects
- **Checklist:**  
  - Do you handle `recv()` returning 0 or errors like `ECONNRESET`?
- **Steps:**  
  - On `recv() == 0` or error, close the socket and remove it from your multiplexer.


//									DONE!!!!!!!!!!
---

### 4. Non-blocking I/O
- **Checklist:**  
  - Are your sockets set to non-blocking mode?
  - Do you handle `EAGAIN`/`EWOULDBLOCK`?
- **Steps:**  
  - Use `fcntl(fd, F_SETFL, O_NONBLOCK)` for non-blocking.
  - If you get `EAGAIN`/`EWOULDBLOCK`, wait for the next event.

//									DONE!!!!!!!!!!
---

### 5. Signal Handling
- **Checklist:**  
  - Do you handle `SIGPIPE`, `SIGINT`, `SIGTERM`, `SIGCHLD`?
- **Steps:**  
  - Ignore `SIGPIPE` (`signal(SIGPIPE, SIG_IGN)`).
  - Catch `SIGINT`/`SIGTERM` for graceful shutdown.
  - Handle `SIGCHLD` if you fork.


//									DONE!!!!!!!!!!
---

### 6. Resource Leaks
- **Checklist:**  
  - Do you free all memory and close all FDs on shutdown or error?
- **Steps:**  
  - Use RAII or cleanup functions.
  - Audit all error paths for leaks.

//									DONE!!!!!!!!!!
---

### 7. Large or Malformed Requests
- **Checklist:**  
  - Do you limit request size and validate input?
- **Steps:**  
  - Set a max request size.
  - Validate headers and body before processing.

//									DONE!!!!!!!!!!
---

### 8. Multiple Listeners/Ports
- **Checklist:**  
  - Can your multiplexer handle events on all listening sockets?
- **Steps:**  
  - Add all listening sockets to your poll/select/epoll set.
  - On event, check which socket is ready and accept accordingly.

//									DONE!!!!!!!!!!
---

### 9. Timeouts
- **Checklist:**  
  - Do you disconnect idle clients?
- **Steps:**  
  - Track last activity time per connection.
  - Periodically check and close idle connections.

//									DONE!!!!!!!!!!
---

### 10. Error Propagation
- **Checklist:**  
  - Do you check and log all return values?
- **Steps:**  
  - Always check return values and log errors.
  - Propagate errors up and handle them at a central point.

---

### 11. IPv4/IPv6 Support
- **Checklist:**  
  - Do you support both address families if needed?
- **Steps:**  
  - Use `getaddrinfo()` and create sockets for both families if required.

---

### 12. Thread Safety (if multithreaded)
- **Checklist:**  
  - Are shared resources protected?
- **Steps:**  
  - Use mutexes or thread-safe containers for shared data.

---

### 13. Graceful Shutdown
- **Checklist:**  
  - Can you shut down without dropping in-flight requests?
- **Steps:**  
  - On shutdown signal, stop accepting new connections, finish current ones, then close.

---

### 14. Handling Many Connections
- **Checklist:**  
  - Have you tested with many clients?
- **Steps:**  
  - Use tools like `ab` or `wrk` to test.
  - Monitor for leaks and performance issues.

---

### 15. Properly Remove Closed FDs
- **Checklist:**  
  - Do you remove closed FDs from your multiplexer set?
- **Steps:**  
  - After closing a socket, remove it from your poll/select/epoll set.

---