#include "statusbar.h"
#include "apps/icons/battery.h"
#include "apps/icons/battery_danger.h"
#include "apps/icons/battery_absent.h"
#include "apps/icons/wifi_disabled.h"
#include "apps/icons/wifi_offline.h"
#include "apps/icons/wifi_connecting.h"
#include "apps/icons/wifi_0.h"
#include "apps/icons/wifi_1.h"
#include "apps/icons/wifi_2.h"
#include "apps/icons/wifi_3.h"
#include "apps/icons/mem.h"
#include "keira/servicemanager.h"
#include "services/clock/clock.h"
#include "services/network/network.h"
#include "Preferences.h"

StatusBarApp::StatusBarApp() : App("StatusBar", 0, 0, lilka::display.width(), 24) {
    loadSettings();
    initSystemWidgets();
}

void StatusBarApp::initSystemWidgets() {
    systemWidgets.clear();
    if (clockMode > 0) {
        systemWidgets.push_back({[this](lilka::Canvas* canvas) { return drawClock(canvas); }, lilka::ALIGN_START, 100});
    }
    if (memMode > 0) {
        uint8_t memAlign = memMode == 1 ? lilka::ALIGN_END : lilka::ALIGN_START;
        systemWidgets.push_back({[this](lilka::Canvas* canvas) { return drawMem(canvas); }, memAlign, 150});
    }
    if (networkMode > 0) {
        systemWidgets.push_back({[this](lilka::Canvas* canvas) { return drawNetwork(canvas); }, lilka::ALIGN_END, 24});
    }
    if (batteryMode > 0) {
        systemWidgets.push_back({[this](lilka::Canvas* canvas) { return drawBattery(canvas); }, lilka::ALIGN_END, 60});
    }
}

uint8_t StatusBarApp::addWidget(std::function<int(lilka::Canvas*)> drawFn, uint8_t alignment, uint8_t maxWidth) {
    appWidgets.push_back({drawFn, alignment, maxWidth, ++appWidgetId});
    return appWidgetId;
}

bool StatusBarApp::removeWidget(uint8_t id) {
    auto it = std::remove_if(appWidgets.begin(), appWidgets.end(), [id](const StatusBarWidget& widget) {
        return widget.id == id;
    });
    if (it != appWidgets.end()) {
        appWidgets.erase(it, appWidgets.end());
        return true;
    }
    return false;
}

const uint16_t leftMargin = 24;
const uint16_t rightMargin = 24;
const uint16_t* wifiIcons[] = {wifi_0_img, wifi_1_img, wifi_2_img, wifi_3_img};

void StatusBarApp::run() {
    while (1) {
        canvas->fillScreen(lilka::colors::Black);
        int availableWidth = canvas->width() - leftMargin - rightMargin;
        int xOffsetStart = leftMargin;
        int xOffsetEnd = canvas->width() - rightMargin;

        drawLeftWidgets(systemWidgets, xOffsetStart, availableWidth);
        drawRightWidgets(systemWidgets, xOffsetEnd, availableWidth);
        drawLeftWidgets(appWidgets, xOffsetStart, availableWidth);
        drawRightWidgets(appWidgets, xOffsetEnd, availableWidth);

        // Draw everything
        queueDraw();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void StatusBarApp::drawLeftWidgets(const std::vector<StatusBarWidget>& widgets, int& xOffset, int& availableWidth) {
    for (int i = 0; i < widgets.size(); i++) {
        auto widget = widgets[i];
        if (widget.alignment == lilka::ALIGN_START) {
            auto widgetWidth = drawWidget(&widget, xOffset, availableWidth);
            if (widgetWidth < 0) {
                break;
            }
            xOffset += widgetWidth;
            availableWidth -= widgetWidth;
        }
    }
}

void StatusBarApp::drawRightWidgets(const std::vector<StatusBarWidget>& widgets, int& xOffset, int& availableWidth) {
    // малюємо системні віджети праворуч
    for (int i = widgets.size() - 1; i >= 0; i--) {
        auto widget = widgets[i];
        if (widget.alignment == lilka::ALIGN_END) {
            xOffset -= widget.maxWidth;
            auto widgetWidth = drawWidget(&widget, xOffset, availableWidth);
            if (widgetWidth < 0) {
                break;
            }
            xOffset += widget.maxWidth - widgetWidth;
            availableWidth -= widgetWidth;
        }
    }
}

int StatusBarApp::drawWidget(StatusBarWidget* widget, int x, int availableWidth) {
    // створюємо тимчасовий canvas та малюємо віджет
    lilka::Canvas widgetCanvas(widget->maxWidth, 24);
    widgetCanvas.fillScreen(lilka::colors::Black);
    widgetCanvas.setTextColor(lilka::colors::White, lilka::colors::Black);
    widgetCanvas.setFont(FONT_9x15);
    auto widgetWidth = widget->drawFn(&widgetCanvas);

    if (widgetWidth <= 0) {
        return 0;
    }

    if (widgetWidth > widget->maxWidth) {
        widgetWidth = widget->maxWidth;
    }

    if (widgetWidth > availableWidth) {
        return -1;
    }

    if (widget->alignment == lilka::ALIGN_END) {
        x += widget->maxWidth - widgetWidth;
    }
    if (widgetWidth == widget->maxWidth) {
        canvas->draw16bitRGBBitmap(x, 0, widgetCanvas.getFramebuffer(), widget->maxWidth, 24);
    } else {
        //певно що так повільніше, але нам треба не весь canvas, а лише ту частину, що займає віджет
        uint16_t* buf = widgetCanvas.getFramebuffer();
        for (int row = 0; row < 24; row++) {
            canvas->draw16bitRGBBitmap(x, row, buf + row * widget->maxWidth, widgetWidth, 1);
        }
    }
    return widgetWidth + 2;
}

int StatusBarApp::drawClock(lilka::Canvas* canvas) {
    ClockService* clockService = ServiceManager::getInstance()->getService<ClockService>("clock");
    struct tm timeinfo = clockService->getTime();
    char strftime_buf[16];
    strftime(strftime_buf, sizeof(strftime_buf), clockMode == 1 ? "%H:%M:%S" : "%H:%M", &timeinfo);
    canvas->setCursor(0, 17);
    canvas->print(strftime_buf);
    return canvas->getCursorX();
}

int StatusBarApp::drawMem(lilka::Canvas* canvas) {
    auto colorRAM = lilka::colors::Red;
    auto colorPSRAM = lilka::colors::Green;

    auto freeRAM = ESP.getFreeHeap();
    auto freePSRAM = ESP.getFreePsram();
    if (memMode == 1) {
        auto totalRAM = ESP.getHeapSize();
        auto totalPSRAM = ESP.getPsramSize();

        int16_t padding = 2;
        int16_t barWidth = 24 - padding * 2;
        int16_t barHeight = 10;
        int16_t barWidthRAM = barWidth * (totalRAM - freeRAM) / totalRAM;
        canvas->fillRect(padding, padding, barWidthRAM, barHeight / 2, colorRAM);
        int16_t barWidthPSRAM = barWidth * (totalPSRAM - freePSRAM) / totalPSRAM;
        canvas->fillRect(padding, padding + barHeight / 2, barWidthPSRAM, barHeight / 2, colorPSRAM);
        canvas->draw16bitRGBBitmapWithTranColor(0, 0, mem_img, lilka::colors::Black, 24, 24);
        return 24;
    } else {
        canvas->setCursor(0, 17);

        char ram_str[15];
        formatSize(freeRAM, ram_str, sizeof(ram_str));
        canvas->setTextColor(colorRAM, lilka::colors::Black);
        canvas->print(ram_str);
        char psram_str[15];
        formatSize(freePSRAM, psram_str, sizeof(psram_str));
        canvas->setTextColor(colorPSRAM, lilka::colors::Black);
        canvas->print(psram_str);
        return canvas->getCursorX();
    }
}

int StatusBarApp::drawNetwork(lilka::Canvas* canvas) {
    NetworkService* networkService = ServiceManager::getInstance()->getService<NetworkService>("network");
    if (networkService != NULL) {
        if (networkService->getNetworkState() == NETWORK_STATE_DISABLED) {
            canvas->draw16bitRGBBitmapWithTranColor(0, 0, wifi_disabled_img, 0, 24, 24);
        } else if (networkService->getNetworkState() == NETWORK_STATE_OFFLINE) {
            canvas->draw16bitRGBBitmapWithTranColor(0, 0, wifi_offline_img, 0, 24, 24);
        } else if (networkService->getNetworkState() == NETWORK_STATE_CONNECTING) {
            canvas->draw16bitRGBBitmapWithTranColor(0, 0, wifi_connecting_img, 0, 24, 24);
        } else if (networkService->getNetworkState() == NETWORK_STATE_ONLINE) {
            canvas->draw16bitRGBBitmapWithTranColor(0, 0, wifiIcons[networkService->getSignalStrength()], 0, 24, 24);
        }
    }
    return 24;
}

int StatusBarApp::drawBattery(lilka::Canvas* canvas) {
    auto level = lilka::battery.readLevel();
    auto xOffset = 0;
    const uint16_t* icon = nullptr;
    if (batteryMode == 1 || batteryMode == 2) {
        if (level == -1) {
            icon = battery_absent_img;
        } else if (level <= 10) {
            icon = battery_danger_img;
        } else {
            icon = battery_img;
        }
    }

    if (icon) {
        canvas->draw16bitRGBBitmapWithTranColor(xOffset, 0, icon, lilka::colors::Fuchsia, 16, 24);
        if (level > -1) {
            // clang-format off
            auto fullHeight = 14;
            auto filledHeight = std::max(1, fullHeight * level / 100);
            auto color = level > 50
                ? lilka::colors::Green
                : level > 20
                    ? lilka::colors::Yellow
                    : lilka::colors::Red;
            canvas->fillRect(xOffset + 4, fullHeight - filledHeight + 6, 8, filledHeight, color);
            // clang-format on
        }
        xOffset += 18;
    }

    if (batteryMode == 1 || batteryMode == 3) {
        canvas->setCursor(xOffset, 17);
        if (level > -1) {
            canvas->print(String(level) + "%");
        } else {
            canvas->print("N/A");
        }
        xOffset = canvas->getCursorX() + 2;
    }
    return xOffset;
}

void StatusBarApp::formatSize(uint32_t bytes, char* buf, size_t len) {
    if (bytes >= 1048576) snprintf(buf, len, "%.1fM", bytes / 1048576.0f);
    else if (bytes >= 1024) snprintf(buf, len, "%uk", bytes / 1024);
    else snprintf(buf, len, "%uB", bytes);
}

void StatusBarApp::loadSettings() {
    Preferences prefs;
    prefs.begin(STATUSBAR_KEIRA_NAMESPACE, true);
    clockMode = prefs.getInt("clock", 1);
    memMode = prefs.getInt("mem", 1);
    networkMode = prefs.getInt("network", 1);
    batteryMode = prefs.getInt("battery", 1);
    prefs.end();
}

void StatusBarApp::setMode(const char* key, uint8_t& mode, uint8_t newMode, uint8_t maxMode) {
    mode = newMode % (maxMode + 1);
    Preferences prefs;
    prefs.begin(STATUSBAR_KEIRA_NAMESPACE, false);
    prefs.putInt(key, mode);
    prefs.end();

    initSystemWidgets();
}

uint8_t StatusBarApp::getClockMode() {
    return clockMode;
}
uint8_t StatusBarApp::getMemMode() {
    return memMode;
}
uint8_t StatusBarApp::getNetworkMode() {
    return networkMode;
}
uint8_t StatusBarApp::getBatteryMode() {
    return batteryMode;
}

void StatusBarApp::setClockMode(uint8_t mode) {
    setMode("clock", clockMode, mode, 2);
}
void StatusBarApp::setMemMode(uint8_t mode) {
    setMode("mem", memMode, mode, 2);
}
void StatusBarApp::setNetworkMode(uint8_t mode) {
    setMode("network", networkMode, mode, 1);
}
void StatusBarApp::setBatteryMode(uint8_t mode) {
    setMode("battery", batteryMode, mode, 3);
}