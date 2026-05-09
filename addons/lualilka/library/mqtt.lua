---@meta

---@class MqttClient Opaque MQTT client handle

---@class MqttMessage
---@field topic string Topic the message was published on
---@field payload string Message payload (may be empty)

---@class mqtt
mqtt = {}

---Creates an MQTT client and connects to the broker.
---
--- Options:
--- - `host` (string, required) — broker hostname or IP
--- - `port` (integer, optional) — broker port (default: 1883)
--- - `client_id` (string, optional)
--- - `username` (string, optional)
--- - `password` (string, optional)
--- - `keepalive` (integer, optional) — keepalive interval in seconds (default: 60)
---@param options table
---@return MqttClient|nil client MQTT client handle, or nil on error
---@return string? errmsg Error message on failure
function mqtt.connect(options) end

---Returns true if the client is currently connected to the broker.
---@param client MqttClient
---@return boolean
function mqtt.connected(client) end

---Subscribes to a topic.
---@param client MqttClient
---@param topic string MQTT topic filter (wildcards # and + are supported)
---@param qos? integer QoS level 0, 1, or 2 (default: 0)
---@return boolean|nil ok true on success, or nil on error
---@return string? errmsg Error message on failure
function mqtt.subscribe(client, topic, qos) end

---Unsubscribes from a topic.
---@param client MqttClient
---@param topic string
function mqtt.unsubscribe(client, topic) end

---Publishes a message.
---@param client MqttClient
---@param topic string
---@param payload string Message payload
---@param qos? integer QoS level 0, 1, or 2 (default: 0)
---@param retain? boolean Retain flag (default: false)
---@return boolean|nil ok true on success, or nil on error
---@return string? errmsg Error message on failure
function mqtt.publish(client, topic, payload, qos, retain) end

---Returns the next pending incoming message without blocking.
---Returns nil when the receive queue is empty.
---@param client MqttClient
---@return MqttMessage|nil message
function mqtt.receive(client) end

---Disconnects from the broker and releases all resources.
---@param client MqttClient
function mqtt.disconnect(client) end
