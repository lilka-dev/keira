---@meta

---@class net
net = {}

---Connects to a TCP server.
---@param host string Server hostname or IP address
---@param port integer Port number
---@param timeout_ms? integer Connect and I/O timeout in milliseconds (default: 5000)
---@return integer|nil fd Socket file descriptor, or nil on error
---@return string? errmsg Error message on failure
function net.connect(host, port, timeout_ms) end

---Sends data over a socket.
---@param fd integer Socket file descriptor
---@param data string Data to send
---@return integer|nil bytes_sent Number of bytes sent, or nil on error
---@return string? errmsg Error message on failure
function net.send(fd, data) end

---Receives data from a socket (blocks until data arrives or timeout).
---@param fd integer Socket file descriptor
---@param max_bytes? integer Maximum bytes to read (default: 1024)
---@param timeout_ms? integer Override receive timeout in ms; -1 keeps the current setting
---@return string|nil data Received data, or nil on timeout/close/error
---@return string? errmsg "timeout" | "connection closed" | error description
function net.receive(fd, max_bytes, timeout_ms) end

---Closes a socket.
---@param fd integer Socket file descriptor
function net.close(fd) end

---Sets the send and receive timeout for a socket.
---@param fd integer Socket file descriptor
---@param timeout_ms integer Timeout in milliseconds
function net.settimeout(fd, timeout_ms) end

---Creates a TCP server socket bound to the given port and starts listening.
---@param port integer Port to listen on
---@param backlog? integer Connection backlog (default: 5)
---@return integer|nil fd Server socket file descriptor, or nil on error
---@return string? errmsg Error message on failure
function net.listen(port, backlog) end

---Accepts an incoming connection on a server socket.
---Returns the client socket fd and the client's IP address string.
---@param server_fd integer Server socket file descriptor
---@param timeout_ms? integer How long to wait for a connection in ms; -1 = no timeout
---@return integer|nil client_fd Client socket fd, or nil on timeout/error
---@return string client_ip_or_errmsg Client IP address on success, or error message
function net.accept(server_fd, timeout_ms) end
