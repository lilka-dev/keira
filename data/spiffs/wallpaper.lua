-- Example Lua wallpaper for Keira home screen.
-- Copy to /sd/wallpaper.lua - it takes priority over image wallpapers.
-- Available modules: display, resources, math, geometry. Buttons are handled by the launcher.

local STAR_COUNT = 90
local SPEED = 0.35 -- depth units per second
local FOCAL = 120

local cx = display.width / 2
local cy = display.height / 2
local stars = {}
local time = 0

local function spawn(star, far)
    star.x = math.random() * 2 - 1
    star.y = math.random() * 2 - 1
    -- New stars appear far away, initial ones are spread over the whole depth
    star.z = far and 1 or (0.05 + math.random() * 0.95)
    star.px = nil
    star.py = nil
end

local function project(star)
    return cx + star.x / star.z * FOCAL, cy + star.y / star.z * FOCAL
end

function lilka.init()
    for i = 1, STAR_COUNT do
        stars[i] = {}
        spawn(stars[i], false)
    end
end

function lilka.update(delta)
    time = time + delta
    -- Slowly swirl the whole field around the center
    local angle = delta * 0.15
    local s = math.sin(angle)
    local c = math.cos(angle)

    for _, star in ipairs(stars) do
        star.px, star.py = project(star)
        star.x, star.y = star.x * c - star.y * s, star.x * s + star.y * c
        star.z = star.z - SPEED * delta
        if star.z <= 0.05 then
            spawn(star, true)
        else
            local x, y = project(star)
            if x < 0 or x >= display.width or y < 0 or y >= display.height then
                spawn(star, true)
            end
        end
    end
end

function lilka.draw()
    -- Background slowly shifts between deep blue and purple
    local shift = (math.sin(time * 0.2) + 1) / 2
    display.fill_screen(display.color565(math.floor(20 * shift), 0, 30))

    for _, star in ipairs(stars) do
        local x, y = project(star)
        -- Closer stars are brighter and bigger
        local brightness = math.floor(255 * (1 - star.z))
        -- Slight blue tint, stays within 0..255
        local color = display.color565(brightness, brightness, math.floor(60 + brightness * 0.75))
        if star.px ~= nil then
            display.draw_line(star.px, star.py, x, y, color)
        end
        if star.z < 0.3 then
            display.fill_rect(x - 1, y - 1, 2, 2, color)
        else
            display.draw_pixel(x, y, color)
        end
    end
end
