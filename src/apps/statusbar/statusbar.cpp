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
}

const uint16_t* icons[] = {wifi_0_img, wifi_1_img, wifi_2_img, wifi_3_img};

void StatusBarApp::run() {
    while (1) {
        canvas->fillScreen(lilka::colors::Black);
        canvas->setTextColor(lilka::colors::White, lilka::colors::Black);
        canvas->setFont(FONT_9x15);

        drawClock();
        drawRam();
        drawNetwork();
        drawBattery();

        // Draw everything
        queueDraw();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void StatusBarApp::drawClock() {
    ClockService* clockService = ServiceManager::getInstance()->getService<ClockService>("clock");
    struct tm timeinfo = clockService->getTime();
    char strftime_buf[16];
    strftime(strftime_buf, sizeof(strftime_buf), "%H:%M", &timeinfo);
    canvas->setCursor(24, 17);
    canvas->print(strftime_buf);
}

void StatusBarApp::drawRam() {
    uint32_t freeRAM = ESP.getFreeHeap();
    uint32_t freePSRAM = ESP.getFreePsram();
#ifdef KEIRA_RAM_ICON
    int xOffset = canvas->width() - 128;
    uint32_t totalRAM = ESP.getHeapSize();
    uint32_t totalPSRAM = ESP.getPsramSize();

    int16_t padding = 2;
    int16_t barWidth = 24 - padding * 2;
    int16_t barHeight = 10;
    int16_t barWidthRAM = barWidth * (totalRAM - freeRAM) / totalRAM;
    canvas->fillRect(xOffset + padding, padding, barWidthRAM, barHeight / 2, lilka::colors::Red);
    int16_t barWidthPSRAM = barWidth * (totalPSRAM - freePSRAM) / totalPSRAM;
    canvas->fillRect(xOffset + padding, padding + barHeight / 2, barWidthPSRAM, barHeight / 2, lilka::colors::Green);
    canvas->draw16bitRGBBitmapWithTranColor(xOffset, 0, ram_img, lilka::colors::Black, 24, 24);
#else
    char ram_str[15], psram_str[15];
    formatSize(freeRAM, ram_str, sizeof(ram_str));
    formatSize(freePSRAM, psram_str, sizeof(psram_str));

    char ram_buf[32];
    snprintf(ram_buf, sizeof(ram_buf), " %s/%s", ram_str, psram_str);
    canvas->print(ram_buf);
#endif
}

void StatusBarApp::drawNetwork() {
    NetworkService* networkService = ServiceManager::getInstance()->getService<NetworkService>("network");

    int xOffset = canvas->width() - 100;
    if (networkService != NULL) {
        if (networkService->getNetworkState() == NETWORK_STATE_DISABLED) {
            canvas->draw16bitRGBBitmapWithTranColor(xOffset, 0, wifi_disabled_img, 0, 24, 24);
        } else if (networkService->getNetworkState() == NETWORK_STATE_OFFLINE) {
            canvas->draw16bitRGBBitmapWithTranColor(xOffset, 0, wifi_offline_img, 0, 24, 24);
        } else if (networkService->getNetworkState() == NETWORK_STATE_CONNECTING) {
            canvas->draw16bitRGBBitmapWithTranColor(xOffset, 0, wifi_connecting_img, 0, 24, 24);
        } else if (networkService->getNetworkState() == NETWORK_STATE_ONLINE) {
            canvas->draw16bitRGBBitmapWithTranColor(xOffset, 0, icons[networkService->getSignalStrength()], 0, 24, 24);
        }
    }
}

void StatusBarApp::drawBattery() {
    int level = lilka::battery.readLevel();
    int xOffset = canvas->width() - 72;
    if (level == -1) {
        canvas->draw16bitRGBBitmapWithTranColor(xOffset, 0, battery_absent_img, lilka::colors::Fuchsia, 16, 24);
    } else {
        int16_t x1 = 4, y1 = 6;
        int16_t width = 8, fullHeight = 14;
        int filledHeight = fullHeight * level / 100;
        if (filledHeight < 1) filledHeight = 1;
        int emptyHeight = fullHeight - filledHeight;
        int16_t color = level > 50 ? lilka::colors::Green : (level > 20 ? lilka::colors::Yellow : lilka::colors::Red);
        canvas->draw16bitRGBBitmapWithTranColor(
            xOffset, 0, level > 10 ? battery_img : battery_danger_img, lilka::colors::Fuchsia, 16, 24
        );
        canvas->fillRect(xOffset + x1, y1 + emptyHeight, width, filledHeight, color);
        xOffset += 4 + 16;
        canvas->setCursor(xOffset, 17);
        canvas->print(String(level) + "%");
    }
}

void StatusBarApp::formatSize(uint32_t bytes, char* buf, size_t len) {
    if (bytes >= 1048576) snprintf(buf, len, "%.1fM", bytes / 1048576.0f);
    else if (bytes >= 1024) snprintf(buf, len, "%uk", bytes / 1024);
    else snprintf(buf, len, "%uB", bytes);
}