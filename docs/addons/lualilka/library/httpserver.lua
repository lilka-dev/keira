---@meta

---@class HttpRequest
---@field fd integer Client socket fd — pass to httpserver.respond() or net.close()
---@field client_ip string Remote client IP address
---@field method string HTTP method (GET, POST, PUT, DELETE, …)
---@field path string URL path without query string
---@field query string Raw query string (may be empty)
---@field headers table<string, string> Request headers (keys lowercased)
---@field body string Request body (empty string when none)

---@class httpserver
httpserver = {}

---Creates a TCP server socket and starts listening for HTTP connections.
---@param port integer Port to listen on (e.g. 80)
---@param backlog? integer Connection backlog (default: 5)
---@return integer|nil fd Server socket file descriptor, or nil on error
---@return string? errmsg Error message on failure
function httpserver.listen(port, backlog) end

---Waits for and accepts the next HTTP connection, parses the request.
---The returned `fd` must be passed to `httpserver.respond()` or `net.close()`.
---@param server_fd integer Server socket from httpserver.listen()
---@param timeout_ms? integer How long to wait in ms; -1 = block indefinitely
---@return HttpRequest|nil request Parsed request table, or nil on timeout/error
---@return string? errmsg "timeout" | error description
function httpserver.accept(server_fd, timeout_ms) end

---Sends a complete HTTP/1.1 response and closes the client connection.
---@param fd integer Client socket fd from request.fd
---@param status integer HTTP status code (e.g. 200, 404)
---@param headers? table<string, string> Extra response headers
---@param body? string Response body (default: empty)
function httpserver.respond(fd, status, headers, body) end

---Closes a server socket (stops accepting new connections).
---@param fd integer Server socket file descriptor
function httpserver.close(fd) end
