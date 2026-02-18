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
#include "apps/icons/ram.h"
#include "keira/servicemanager.h"
#include "services/clock/clock.h"
#include "services/network/network.h"

StatusBarApp::StatusBarApp() : App("StatusBar", 0, 0, lilka::display.width(), 24) {
    initSystemWidgets();
}

void StatusBarApp::initSystemWidgets() {
    systemWidgets.push_back({[this](lilka::Canvas* canvas) { return drawClock(canvas); }, lilka::ALIGN_START, 50});
#ifdef KEIRA_RAM_ICON
    systemWidgets.push_back({[this](lilka::Canvas* canvas) { return drawRamIcon(canvas); }, lilka::ALIGN_END, 24});
#else
    systemWidgets.push_back({[this](lilka::Canvas* canvas) { return drawRamText(canvas); }, lilka::ALIGN_START, 150});
#endif
    systemWidgets.push_back({[this](lilka::Canvas* canvas) { return drawNetwork(canvas); }, lilka::ALIGN_END, 24});
    systemWidgets.push_back({[this](lilka::Canvas* canvas) { return drawBattery(canvas); }, lilka::ALIGN_END, 60});
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
        int availableWidth = canvas->width() - leftMargin - rightMargin;
        int xOffsetStart = leftMargin;
        int xOffsetEnd = canvas->width() - rightMargin;

        drawLeftWidgets(systemWidgets, xOffsetStart, availableWidth);
        drawRightWidgets(systemWidgets, xOffsetEnd, availableWidth);
        drawLeftWidgets(appWidgets, xOffsetStart, availableWidth);
        drawRightWidgets(appWidgets, xOffsetEnd, availableWidth);

        canvas->fillRect(xOffsetStart, 0, xOffsetEnd - xOffsetStart, canvas->height(), lilka::colors::Black);

        // Draw everything
        queueDraw();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void StatusBarApp::drawLeftWidgets(std::vector<StatusBarWidget>& widgets, int& xOffset, int& availableWidth) {
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

void StatusBarApp::drawRightWidgets(std::vector<StatusBarWidget>& widgets, int& xOffset, int& availableWidth) {
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
    strftime(strftime_buf, sizeof(strftime_buf), "%H:%M", &timeinfo);
    canvas->setCursor(0, 17);
    canvas->print(strftime_buf);
    return canvas->getCursorX();
}

int StatusBarApp::drawRamText(lilka::Canvas* canvas) {
    char ram_str[15], psram_str[15];
    formatSize(ESP.getFreeHeap(), ram_str, sizeof(ram_str));
    formatSize(ESP.getFreePsram(), psram_str, sizeof(psram_str));

    char ram_buf[32];
    snprintf(ram_buf, sizeof(ram_buf), "%s/%s", ram_str, psram_str);
    canvas->setTextColor(lilka::colors::Green, lilka::colors::Black);
    canvas->setCursor(0, 17);
    canvas->print(ram_buf);
    return canvas->getCursorX();
}

int StatusBarApp::drawRamIcon(lilka::Canvas* canvas) {
    uint32_t freeRAM = ESP.getFreeHeap();
    uint32_t freePSRAM = ESP.getFreePsram();
    uint32_t totalRAM = ESP.getHeapSize();
    uint32_t totalPSRAM = ESP.getPsramSize();

    int16_t padding = 2;
    int16_t barWidth = 24 - padding * 2;
    int16_t barHeight = 10;
    int16_t barWidthRAM = barWidth * (totalRAM - freeRAM) / totalRAM;
    canvas->fillRect(padding, padding, barWidthRAM, barHeight / 2, lilka::colors::Red);
    int16_t barWidthPSRAM = barWidth * (totalPSRAM - freePSRAM) / totalPSRAM;
    canvas->fillRect(padding, padding + barHeight / 2, barWidthPSRAM, barHeight / 2, lilka::colors::Green);
    canvas->draw16bitRGBBitmapWithTranColor(0, 0, ram_img, lilka::colors::Black, 24, 24);
    return 24;
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
    int level = lilka::battery.readLevel();
    if (level == -1) {
        canvas->draw16bitRGBBitmapWithTranColor(0, 0, battery_absent_img, lilka::colors::Fuchsia, 16, 24);
        return 24;
    } else {
        int16_t x1 = 4, y1 = 6;
        int16_t width = 8, fullHeight = 14;
        int filledHeight = fullHeight * level / 100;
        if (filledHeight < 1) filledHeight = 1;
        int emptyHeight = fullHeight - filledHeight;
        int16_t color = level > 50 ? lilka::colors::Green : (level > 20 ? lilka::colors::Yellow : lilka::colors::Red);
        canvas->draw16bitRGBBitmapWithTranColor(
            0, 0, level > 10 ? battery_img : battery_danger_img, lilka::colors::Fuchsia, 16, 24
        );
        canvas->fillRect(0 + x1, y1 + emptyHeight, width, filledHeight, color);
        canvas->setCursor(20, 17);
        canvas->print(String(level) + "%");
        return canvas->getCursorX() + 2;
    }
}

void StatusBarApp::formatSize(uint32_t bytes, char* buf, size_t len) {
    if (bytes >= 1048576) snprintf(buf, len, "%.1fM", bytes / 1048576.0f);
    else if (bytes >= 1024) snprintf(buf, len, "%uk", bytes / 1024);
    else snprintf(buf, len, "%uB", bytes);
}