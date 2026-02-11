//
// MadPlayer – програвач аудіо-файлів на базі бібліотеки ESP8266Audio для Lilka зі звуковим модулем PCM5102.
// Підтримувані формати: MOD, WAV, MP3, AAC, FLAC.
// Автори: Олексій "Alder" Деркач (https://github.com/alder) та Андрій "and3rson" Дунай (https://github.com/and3rson)
//

#include <AudioFileSourceSD.h>

#include "madplayer.h"

// Визначає тип аудіо за розширенням файлу. Повертає nullptr, якщо формат невідомий.
static const char* detectAudioType(const String& path) {
    String lower = path;
    lower.toLowerCase();
    if (lower.endsWith(".mod")) return "mod";
    if (lower.endsWith(".wav")) return "wav";
    if (lower.endsWith(".mp3")) return "mp3";
    if (lower.endsWith(".aac")) return "aac";
    if (lower.endsWith(".flac")) return "flac";
    return nullptr;
}

MadPlayerApp::MadPlayerApp(String path) : App("MadPlayer", 0, 0, lilka::display.width(), lilka::display.height()) {
    setFlags(AppFlags::APP_FLAG_FULLSCREEN);
    setCore(1);
    fileName = lilka::fileutils.getLocalPathInfo(path).path;
}

void MadPlayerApp::run() {
    // Detect audio type by extension
    const char* audioType = detectAudioType(fileName);
    if (!audioType) {
        alert("Помилка", "Непідтримуваний формат аудіо");
        return;
    }

    // Read file into memory (scoped so SD file is closed immediately, freeing VFS buffer)
    uint8_t* fileData;
    size_t fileSize;
    {
        AudioFileSourceSD fileSource(fileName.c_str());
        fileSize = fileSource.getSize();
        fileData = new uint8_t[fileSize];
        fileSource.read(fileData, fileSize);
    }

    // Create Sound (takes ownership of fileData)
    sound = new lilka::Sound(fileData, fileSize, audioType);

    // Create I2S output and analyzer (owned by this app)
    i2sOutput = new AudioOutputI2S();
    i2sOutput->SetPinout(LILKA_I2S_BCLK, LILKA_I2S_LRCK, LILKA_I2S_DOUT);
    analyzer = new lilka::AudioOutputAnalyzer(i2sOutput);

    // Start playback via AudioPlayer
    lilka::AudioPlayer* player = lilka::AudioPlayer::getInstance();
    player->play(sound, analyzer);

    while (1) {
        mainWindow();

        lilka::State state = lilka::controller.getState();
        if (state.a.justPressed) {
            if (player->isPaused()) {
                player->resume();
            } else {
                player->pause();
            }
        }
        if (state.b.justPressed) {
            player->stop();
            break;
        }
        if (state.up.justPressed) {
            player->setGain(player->getGain() + 0.25f);
        }
        if (state.down.justPressed) {
            player->setGain(player->getGain() - 0.25f);
        }
    }

    // Cleanup
    delete analyzer;
    analyzer = nullptr;
    delete i2sOutput;
    i2sOutput = nullptr;
    delete sound;
    sound = nullptr;
}

void MadPlayerApp::mainWindow() {
    canvas->fillScreen(lilka::colors::Black);

    lilka::AudioPlayer* player = lilka::AudioPlayer::getInstance();
    bool shouldDrawAnalyzer = !player->isPaused() && !player->isFinished();
    if (shouldDrawAnalyzer) {
        int16_t analyzerBuffer[ANALYZER_BUFFER_SIZE];
        analyzer->readBuffer(analyzerBuffer);
        int16_t head = analyzer->getBufferHead();
        float gain = player->getGain();

        int16_t prevX, prevY;
        int16_t width = canvas->width();
        int16_t height = canvas->height();

        constexpr int16_t HUE_SPEED_DIV = 4;
        constexpr int16_t HUE_SCALE = 4;
        int16_t yCenter = height * 5 / 7;

        int64_t time = millis();

        for (int i = 0; i < ANALYZER_BUFFER_SIZE; i += 4) {
            int x = i * width / ANALYZER_BUFFER_SIZE;
            int index = (i + head) % ANALYZER_BUFFER_SIZE;
            float amplitude = static_cast<float>(analyzerBuffer[index]) / 32768 * fmaxf(gain, 1.0f);
            int y = yCenter + static_cast<int>(amplitude * height / 2);
            if (i > 0) {
                int16_t hue = (time / HUE_SPEED_DIV + i / HUE_SCALE) % 360;
                canvas->drawLine(prevX, prevY, x, y, lilka::display.color565hsv(hue, 100, 100));
            }
            prevX = x;
            prevY = y;
        }
    }

    float currentGain = player->getGain();
    bool currentFinished = player->isFinished();

    canvas->setFont(FONT_9x15);
    canvas->setTextBound(32, 32, canvas->width() - 64, canvas->height() - 32);
    canvas->setTextColor(lilka::colors::White);
    canvas->setCursor(32, 32 + 15);
    canvas->println("MadPlayer");
    canvas->println("------------------------");
    canvas->println("A - Відтворення / пауза");
    canvas->setFont(FONT_9x15_SYMBOLS);
    canvas->print("↑ / ↓");
    canvas->setFont(FONT_9x15);
    canvas->println(" - Гучність");
    canvas->println("B - Вихід");
    canvas->println("------------------------");
    canvas->println("Гучність: " + String(currentGain));
    if (currentFinished) canvas->println("Трек закінчився");

    lilka::Canvas titleCanvas(canvas->width(), 20);
    titleCanvas.fillScreen(lilka::colors::Black);
    titleCanvas.setFont(FONT_9x15);
    titleCanvas.setTextColor(lilka::display.color565hsv((millis() * 30) % 360, 100, 100));
    titleCanvas.drawTextAligned(
        fileName.c_str(), titleCanvas.width() / 2, titleCanvas.height() / 2, lilka::ALIGN_CENTER, lilka::ALIGN_CENTER
    );
    const uint16_t* titleCanvasFB = titleCanvas.getFramebuffer();
    uint16_t yOffset = canvas->height() - titleCanvas.height() - 8;
    float time = millis() / 1500.0f;
    for (int16_t x = 0; x < titleCanvas.width(); x++) {
        int16_t yShift = sinf(time + x / 25.0f) * 4 + yOffset;
        for (int16_t y = 0; y < titleCanvas.height(); y++) {
            uint16_t color = titleCanvasFB[x + y * titleCanvas.width()];
            if (color) {
                canvas->drawPixel(x, y + yShift, color);
            }
        }
    }
    queueDraw();
}
