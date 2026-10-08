-- Example background service for Keira.
-- Copy to /sd/autorun.lua - it's started in the background on every boot.
-- Services have no screen and no buttons: display, controller, resources, UI and audio are not available.
-- Output of print() and errors go to serial.
--
-- This one writes a heartbeat line with uptime and free RAM to /sd/autorun.log every minute
-- and counts boots using state, which is saved to /sd/autorun.state when the service stops.

local LOG_PATH = "/sd/autorun.log"
local LOG_MAX_SIZE = 64 * 1024 -- log is cleared when it grows bigger than this

local sinceLog = 0

local function log(message)
    local line = string.format("[%8.1f] %s", util.time(), message)
    print(service.name .. ": " .. line)

    local file = io.open(LOG_PATH, "a")
    if file == nil then return end
    -- "a" mode puts position at the end, so seek returns current file size
    if file:seek("end") > LOG_MAX_SIZE then
        file:close()
        file = io.open(LOG_PATH, "w")
        if file == nil then return end
    end
    file:write(line, "\n")
    file:close()
end

function lilka.init()
    state = state or {}
    state.boots = (state.boots or 0) + 1
    -- Save right away: state is saved on stop, but a power-off skips that
    state.save()

    log(string.format("boot #%d, running services: %s", state.boots, table.concat(service.list(), ", ")))
end

-- Nothing here needs to be fast, wake up once per second
service.interval = 1

function lilka.update(delta)
    sinceLog = sinceLog + delta
    if sinceLog < 60 then return end
    sinceLog = 0

    log(string.format("alive, free RAM: %d bytes", util.free_ram()))
end
