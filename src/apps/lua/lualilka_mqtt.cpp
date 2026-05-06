#include "lualilka_mqtt.h"

#include <mqtt_client.h>
#include <esp_event.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <cstring>
#include <cstdlib>

#define MQTT_MT             "lualilka_mqtt_client"
#define MQTT_QUEUE_CAPACITY 16

struct MqttMsg {
    char* topic;
    char* payload;
    int payload_len;
};

struct LuaMqttClient {
    esp_mqtt_client_handle_t handle;
    QueueHandle_t queue;
    volatile bool connected;
};

static void mqtt_drain_queue(LuaMqttClient* ctx) {
    MqttMsg msg;
    while (ctx->queue && xQueueReceive(ctx->queue, &msg, 0) == pdTRUE) {
        free(msg.topic);
        free(msg.payload);
    }
}

static void mqtt_event_cb(void* arg, esp_event_base_t /*base*/, int32_t event_id, void* event_data) {
    LuaMqttClient* ctx = static_cast<LuaMqttClient*>(arg);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ctx->connected = true;
            break;

        case MQTT_EVENT_DISCONNECTED:
            ctx->connected = false;
            break;

        case MQTT_EVENT_DATA: {
            if (!event->topic || event->topic_len <= 0) break;

            MqttMsg msg = {};
            msg.topic = static_cast<char*>(malloc((size_t)event->topic_len + 1));
            if (!msg.topic) break;
            memcpy(msg.topic, event->topic, (size_t)event->topic_len);
            msg.topic[event->topic_len] = '\0';

            msg.payload_len = event->data_len;
            if (msg.payload_len > 0) {
                msg.payload = static_cast<char*>(malloc((size_t)msg.payload_len + 1));
                if (!msg.payload) {
                    free(msg.topic);
                    break;
                }
                memcpy(msg.payload, event->data, (size_t)msg.payload_len);
                msg.payload[msg.payload_len] = '\0';
            }

            if (xQueueSend(ctx->queue, &msg, 0) != pdTRUE) {
                // Queue full — drop the oldest message and enqueue new one
                MqttMsg old;
                if (xQueueReceive(ctx->queue, &old, 0) == pdTRUE) {
                    free(old.topic);
                    free(old.payload);
                }
                if (xQueueSend(ctx->queue, &msg, 0) != pdTRUE) {
                    free(msg.topic);
                    free(msg.payload);
                }
            }
            break;
        }

        default:
            break;
    }
}

// mqtt.connect(options) -> client | nil, errmsg
// options: { host, port?, client_id?, username?, password?, keepalive? }
static int lualilka_mqtt_connect(lua_State* L) {
    if (!lua_istable(L, 1)) {
        return luaL_error(L, "Expected table argument");
    }

    // Copy all strings into local char arrays to avoid Lua GC / longjmp issues
    char host[256] = {};
    char client_id[128] = {};
    char username[128] = {};
    char password[128] = {};
    int port = 1883;
    int keepalive = 60;
    bool has_client_id = false;
    bool has_username = false;
    bool has_password = false;

    lua_getfield(L, 1, "host");
    if (!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        return luaL_error(L, "host is required");
    }
    strncpy(host, lua_tostring(L, -1), sizeof(host) - 1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "port");
    if (lua_isnumber(L, -1)) port = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "client_id");
    if (lua_isstring(L, -1)) {
        strncpy(client_id, lua_tostring(L, -1), sizeof(client_id) - 1);
        has_client_id = true;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "username");
    if (lua_isstring(L, -1)) {
        strncpy(username, lua_tostring(L, -1), sizeof(username) - 1);
        has_username = true;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "password");
    if (lua_isstring(L, -1)) {
        strncpy(password, lua_tostring(L, -1), sizeof(password) - 1);
        has_password = true;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "keepalive");
    if (lua_isnumber(L, -1)) keepalive = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);

    LuaMqttClient* ctx = static_cast<LuaMqttClient*>(lua_newuserdata(L, sizeof(LuaMqttClient)));
    memset(ctx, 0, sizeof(LuaMqttClient));

    ctx->queue = xQueueCreate(MQTT_QUEUE_CAPACITY, sizeof(MqttMsg));
    if (!ctx->queue) {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to create message queue");
        return 2;
    }

    esp_mqtt_client_config_t config = {};
    config.host = host;
    config.port = (uint32_t)port;
    config.client_id = has_client_id ? client_id : nullptr;
    config.username = has_username ? username : nullptr;
    config.password = has_password ? password : nullptr;
    config.keepalive = keepalive;

    ctx->handle = esp_mqtt_client_init(&config);
    if (!ctx->handle) {
        vQueueDelete(ctx->queue);
        ctx->queue = nullptr;
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to init MQTT client");
        return 2;
    }

    esp_mqtt_client_register_event(ctx->handle, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_cb, ctx);

    esp_err_t err = esp_mqtt_client_start(ctx->handle);
    if (err != ESP_OK) {
        esp_mqtt_client_destroy(ctx->handle);
        ctx->handle = nullptr;
        vQueueDelete(ctx->queue);
        ctx->queue = nullptr;
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushfstring(L, "Failed to start MQTT client: esp_err %d", (int)err);
        return 2;
    }

    luaL_setmetatable(L, MQTT_MT);
    return 1;
}

// mqtt.connected(client) -> bool
static int lualilka_mqtt_connected(lua_State* L) {
    LuaMqttClient* ctx = static_cast<LuaMqttClient*>(luaL_checkudata(L, 1, MQTT_MT));
    lua_pushboolean(L, ctx->connected ? 1 : 0);
    return 1;
}

// mqtt.subscribe(client, topic [, qos]) -> true | nil, errmsg
static int lualilka_mqtt_subscribe(lua_State* L) {
    LuaMqttClient* ctx = static_cast<LuaMqttClient*>(luaL_checkudata(L, 1, MQTT_MT));
    const char* topic = luaL_checkstring(L, 2);
    int qos = (int)luaL_optinteger(L, 3, 0);

    int msg_id = esp_mqtt_client_subscribe(ctx->handle, topic, qos);
    if (msg_id < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "subscribe failed");
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

// mqtt.unsubscribe(client, topic)
static int lualilka_mqtt_unsubscribe(lua_State* L) {
    LuaMqttClient* ctx = static_cast<LuaMqttClient*>(luaL_checkudata(L, 1, MQTT_MT));
    const char* topic = luaL_checkstring(L, 2);
    esp_mqtt_client_unsubscribe(ctx->handle, topic);
    return 0;
}

// mqtt.publish(client, topic, payload [, qos [, retain]]) -> true | nil, errmsg
static int lualilka_mqtt_publish(lua_State* L) {
    LuaMqttClient* ctx = static_cast<LuaMqttClient*>(luaL_checkudata(L, 1, MQTT_MT));
    const char* topic = luaL_checkstring(L, 2);
    size_t len;
    const char* payload = luaL_checklstring(L, 3, &len);
    int qos = (int)luaL_optinteger(L, 4, 0);
    int retain = lua_toboolean(L, 5);

    int msg_id = esp_mqtt_client_publish(ctx->handle, topic, payload, (int)len, qos, retain);
    if (msg_id < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "publish failed");
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

// mqtt.receive(client) -> {topic, payload} | nil
static int lualilka_mqtt_receive(lua_State* L) {
    LuaMqttClient* ctx = static_cast<LuaMqttClient*>(luaL_checkudata(L, 1, MQTT_MT));
    MqttMsg msg;
    if (xQueueReceive(ctx->queue, &msg, 0) != pdTRUE) {
        lua_pushnil(L);
        return 1;
    }

    lua_createtable(L, 0, 2);
    lua_pushstring(L, msg.topic);
    lua_setfield(L, -2, "topic");
    if (msg.payload && msg.payload_len > 0) {
        lua_pushlstring(L, msg.payload, (size_t)msg.payload_len);
    } else {
        lua_pushstring(L, "");
    }
    lua_setfield(L, -2, "payload");

    free(msg.topic);
    free(msg.payload);
    return 1;
}

// mqtt.disconnect(client)
static int lualilka_mqtt_disconnect(lua_State* L) {
    LuaMqttClient* ctx = static_cast<LuaMqttClient*>(luaL_checkudata(L, 1, MQTT_MT));
    if (ctx->handle) {
        esp_mqtt_client_stop(ctx->handle);
        esp_mqtt_client_destroy(ctx->handle);
        ctx->handle = nullptr;
    }
    if (ctx->queue) {
        mqtt_drain_queue(ctx);
        vQueueDelete(ctx->queue);
        ctx->queue = nullptr;
    }
    ctx->connected = false;
    return 0;
}

static int lualilka_mqtt_gc(lua_State* L) {
    LuaMqttClient* ctx = static_cast<LuaMqttClient*>(luaL_checkudata(L, 1, MQTT_MT));
    if (ctx->handle) {
        esp_mqtt_client_stop(ctx->handle);
        esp_mqtt_client_destroy(ctx->handle);
        ctx->handle = nullptr;
    }
    if (ctx->queue) {
        mqtt_drain_queue(ctx);
        vQueueDelete(ctx->queue);
        ctx->queue = nullptr;
    }
    return 0;
}

static const struct luaL_Reg lualilka_mqtt_funcs[] = {
    {"connect", lualilka_mqtt_connect},
    {"connected", lualilka_mqtt_connected},
    {"subscribe", lualilka_mqtt_subscribe},
    {"unsubscribe", lualilka_mqtt_unsubscribe},
    {"publish", lualilka_mqtt_publish},
    {"receive", lualilka_mqtt_receive},
    {"disconnect", lualilka_mqtt_disconnect},
    {NULL, NULL},
};

int lualilka_mqtt_register(lua_State* L) {
    luaL_newmetatable(L, MQTT_MT);
    lua_pushcfunction(L, lualilka_mqtt_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_newlib(L, lualilka_mqtt_funcs);
    lua_setglobal(L, "mqtt");
    return 0;
}
