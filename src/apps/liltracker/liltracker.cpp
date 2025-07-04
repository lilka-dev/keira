#include <lilka.h>
#include "keira/keira.h"
#include "liltracker.h"
#include "note.h"
#include "i2s_sink.h"
#include "wav_sink.h"
#include "utils/defer.h"

#include "icons/liltracker_icons.h"

#define LILTRACKER_DIR "/liltracker"

// Layout:
// - Title
// - Controls (3 columns, 2 rows)
// - Score header (3 columns)
// - Score (3 columns, rest of the screen)

// Overall layout
constexpr int32_t ITEM_HEIGHT = 15;

// Title
constexpr int32_t TITLE_HEIGHT = 20;

// Controls
constexpr int32_t CONTROL_TOP = TITLE_HEIGHT;
constexpr int32_t CONTROL_ROWS = 3;
constexpr int32_t CONTROL_COLS = 3;
constexpr int32_t CONTROL_PADDING_LEFT = 32;
const int32_t CONTROL_WIDTH = (lilka::display.width() - CONTROL_PADDING_LEFT) / CONTROL_COLS;

// Score header
constexpr int32_t SCORE_HEADER_TOP = CONTROL_TOP + ITEM_HEIGHT * CONTROL_ROWS;

// Score
constexpr int32_t SCORE_TOP = SCORE_HEADER_TOP + ITEM_HEIGHT;
const int32_t SCORE_HEIGHT = lilka::display.height() - SCORE_TOP;
constexpr int32_t SCORE_ITEM_HEIGHT = ITEM_HEIGHT;
const uint8_t* FONT = FONT_8x13;
const uint8_t* SCORE_FONT = FONT_8x13_MONO;
constexpr int32_t SCORE_COUNTER_WIDTH = 32;
const int32_t SCORE_EVENT_WIDTH = (lilka::display.width() - SCORE_COUNTER_WIDTH) / CHANNEL_COUNT;
const int32_t SCORE_ROW_COUNT = SCORE_HEIGHT / ITEM_HEIGHT;
const int32_t SCORE_MIDDLE_ROW_INDEX = SCORE_ROW_COUNT / 2;

const uint8_t* FONT_ICONS = liltracker_icons;

typedef enum : uint8_t {
    BLOCK_CONTROLS,
    BLOCK_EVENT_SCORE,
    BLOCK_COUNT,
} active_block_t;

typedef enum : uint8_t {
    SEGMENT_NOTE,
    SEGMENT_WAVEFORM,
    SEGMENT_VOLUME,
    SEGMENT_EFFECT,
    SEGMENT_EFFECT_PARAM,
    SEGMENT_COUNT,
} active_segment_t;

typedef enum : uint8_t {
    CONTROL_ROW_0_BUTTONS,
    CONTROL_ROW_1_SETTINGS,
    CONTROL_ROW_2_SETTINGS,
    CONTROL_ROW_3_PATTERNS,
    CONTROL_ROW_COUNT,
} control_row_t;

typedef enum : uint8_t {
    VISUALIZER_MODE_PER_CHANNEL,
    VISUALIZER_MODE_MIXED,
    VISUALIZER_MODE_COUNT,
} visualizer_mode_t;

LilTrackerApp::LilTrackerApp() :
    App("LilTracker", 0, 0, lilka::display.width(), lilka::display.height()), sequencer(NULL) {
    this->setFlags(APP_FLAG_FULLSCREEN);
    this->setCore(1);
    this->setStackSize(16384);
    this->initialPath = "";
}

LilTrackerApp::LilTrackerApp(String path) : LilTrackerApp() {
    this->initialPath = lilka::fileutils.getLocalPathInfo(path).path;
}

xSemaphoreHandle xMutex;

void LilTrackerApp::run() {
    Track track;

    I2SSink i2sSink;
    sequencer.setSink(&i2sSink);

    WAVSink* wavSink = NULL;

    if (initialPath.length()) {
        loadTrack(&track, initialPath);
    }

    int pageIndex = 0;

    int8_t activeBlock = BLOCK_EVENT_SCORE;
    int scoreCursorX = 0;
    int scoreCursorY = 0;

    bool isEditing = false;
    bool isPreviewing = false;

    int controlCursorX = 0;
    int controlCursorY = CONTROL_ROW_0_BUTTONS;

    int visualizerMode = VISUALIZER_MODE_PER_CHANNEL;

    event_t copiedEvent = {NOTE_ZERO, WAVEFORM_CONT, MAX_VOLUME, EVENT_TYPE_CONT, {EFFECT_TYPE_NONE, 0}};

    char str[64];

    // Set initial volume
    sequencer.setMasterVolume(0.25f * lilka::audio.getVolume() / 100);

    while (1) {
        seq_state_t seqState = sequencer.getSeqState();

        if (seqState.playing) {
            scoreCursorY = seqState.eventIndex;
            pageIndex = seqState.pageIndex;
        }

        if (!seqState.playing && wavSink) {
            delete wavSink;
            wavSink = NULL;
            sequencer.setSink(&i2sSink);
        }

        int currentChannel = scoreCursorX / SEGMENT_COUNT;
        int currentSegment = scoreCursorX % SEGMENT_COUNT;

        page_t* page = track.getPage(pageIndex);

        canvas->fillScreen(lilka::colors::Black);
        canvas->setFont(FONT);

        float masterVolume = sequencer.getMasterVolume();

        if (seqState.playing) {
            // Draw visualizer
            float graphScale = 1.0f;
            if (masterVolume > 0.01f) {
                graphScale = 1.0f / masterVolume;
            } else {
                graphScale = 0.0f;
            }
            if (visualizerMode == VISUALIZER_MODE_PER_CHANNEL) {
                // Draw per-channel buffers
                for (int channelIndex = 0; channelIndex < CHANNEL_COUNT; channelIndex++) {
                    int16_t buffer[SYNTH_BUFFER_SIZE];
                    sequencer.readBuffer(buffer, channelIndex);
                    int16_t prevX, prevY;
                    int16_t minX = SCORE_COUNTER_WIDTH + channelIndex * SCORE_EVENT_WIDTH;
                    int16_t width = SCORE_EVENT_WIDTH;

                    for (int i = 0; i < SYNTH_BUFFER_SIZE; i += 4) {
                        int x = minX + i * width / SYNTH_BUFFER_SIZE;
                        int index = i / 2; // Make samples wider for nicer display
                        float amplitude = static_cast<float>(buffer[index]) / 32768.0f * graphScale;
                        int y = SCORE_HEADER_TOP / 2 - static_cast<int>(amplitude * SCORE_HEADER_TOP / 2);
                        if (i > 0) {
                            canvas->drawLine(prevX, prevY, x, y, lilka::colors::White);
                        }
                        prevX = x;
                        prevY = y;
                    }
                }
            } else {
                // Draw mixed buffer
                int16_t buffer[SYNTH_BUFFER_SIZE];
                sequencer.readBuffer(buffer);

                int16_t prevX, prevY;
                for (int i = 0; i < SYNTH_BUFFER_SIZE; i++) {
                    int x = i * lilka::display.width() / SYNTH_BUFFER_SIZE;
                    float amplitude = static_cast<float>(buffer[i]) / 32768.0f * graphScale;
                    int y = SCORE_HEADER_TOP / 2 - static_cast<int>(amplitude * SCORE_HEADER_TOP / 2);
                    if (i > 0) {
                        canvas->drawLine(prevX, prevY, x, y, lilka::colors::White);
                    }
                    prevX = x;
                    prevY = y;
                }
            }
        } else {
            // Draw title
            canvas->setFont(FONT_10x20);
            canvas->setTextColor(lilka::colors::Pink, lilka::colors::Pink);
            sprintf(str, "ЛілТрекер by Андерсон");
            canvas->drawTextAligned(
                str, lilka::display.width() / 2, TITLE_HEIGHT / 2, lilka::ALIGN_CENTER, lilka::ALIGN_CENTER
            );

            canvas->setFont(FONT);

            // Draw buttons (load/save/reset)
            for (int i = 0; i < 3; i++) {
                bool isFocused =
                    activeBlock == BLOCK_CONTROLS && controlCursorX == i && controlCursorY == CONTROL_ROW_0_BUTTONS;
                const char* buttonText;
                if (i == 0) {
                    buttonText = "Відкрити";
                } else if (i == 1) {
                    buttonText = "Зберегти";
                } else {
                    buttonText = "Скинути";
                }
                drawElement(
                    buttonText,
                    CONTROL_PADDING_LEFT + CONTROL_WIDTH * i,
                    CONTROL_TOP + ITEM_HEIGHT / 2,
                    lilka::ALIGN_START,
                    lilka::ALIGN_CENTER,
                    isFocused && isEditing,
                    isFocused,
                    lilka::colors::Uranian_blue
                );
            }

            // Draw current page index, BPM and length
            sprintf(str, "Page: %02X", pageIndex);
            bool isPageSelFocused =
                activeBlock == BLOCK_CONTROLS && controlCursorX == 0 && controlCursorY == CONTROL_ROW_1_SETTINGS;
            drawElement(
                str,
                CONTROL_PADDING_LEFT,
                CONTROL_TOP + ITEM_HEIGHT * 3 / 2,
                lilka::ALIGN_START,
                lilka::ALIGN_CENTER,
                isPageSelFocused && isEditing,
                isPageSelFocused,
                lilka::colors::Uranian_blue
            );

            bool isBPMFocused =
                activeBlock == BLOCK_CONTROLS && controlCursorX == 1 && controlCursorY == CONTROL_ROW_1_SETTINGS;
            sprintf(str, "BPM: %3d", track.getBPM());
            drawElement(
                str,
                CONTROL_PADDING_LEFT + CONTROL_WIDTH,
                CONTROL_TOP + ITEM_HEIGHT * 3 / 2,
                lilka::ALIGN_START,
                lilka::ALIGN_CENTER,
                isBPMFocused && isEditing,
                isBPMFocused,
                lilka::colors::Uranian_blue
            );

            bool isLengthFocused =
                activeBlock == BLOCK_CONTROLS && controlCursorX == 2 && controlCursorY == CONTROL_ROW_1_SETTINGS;
            sprintf(str, "Len: %3d", track.getPageCount());
            drawElement(
                str,
                CONTROL_PADDING_LEFT + CONTROL_WIDTH * 2,
                CONTROL_TOP + ITEM_HEIGHT * 3 / 2,
                lilka::ALIGN_START,
                lilka::ALIGN_CENTER,
                isLengthFocused && isEditing,
                isLengthFocused,
                lilka::colors::Uranian_blue
            );

            // Draw unused section, WAV render & master volume
            bool isUnused0Focused =
                activeBlock == BLOCK_CONTROLS && controlCursorX == 0 && controlCursorY == CONTROL_ROW_2_SETTINGS;
            drawElement(
                "-",
                CONTROL_PADDING_LEFT,
                CONTROL_TOP + ITEM_HEIGHT * 5 / 2,
                lilka::ALIGN_START,
                lilka::ALIGN_CENTER,
                false,
                isUnused0Focused,
                lilka::colors::Uranian_blue
            );
            bool isWAVRenderFocused =
                activeBlock == BLOCK_CONTROLS && controlCursorX == 1 && controlCursorY == CONTROL_ROW_2_SETTINGS;
            drawElement(
                "Запис WAV",
                CONTROL_PADDING_LEFT + CONTROL_WIDTH,
                CONTROL_TOP + ITEM_HEIGHT * 5 / 2,
                lilka::ALIGN_START,
                lilka::ALIGN_CENTER,
                false,
                isWAVRenderFocused,
                lilka::colors::Uranian_blue
            );
            sprintf(str, "Vol: %.0f%%", roundf(masterVolume * 100));
            bool isMasterVolumeFocused =
                activeBlock == BLOCK_CONTROLS && controlCursorX == 2 && controlCursorY == CONTROL_ROW_2_SETTINGS;
            drawElement(
                str,
                CONTROL_PADDING_LEFT + CONTROL_WIDTH * 2,
                CONTROL_TOP + ITEM_HEIGHT * 5 / 2,
                lilka::ALIGN_START,
                lilka::ALIGN_CENTER,
                isMasterVolumeFocused && isEditing,
                isMasterVolumeFocused,
                masterVolume < 0.5 ? lilka::colors::Uranian_blue : lilka::colors::Red
            );
        }

        // Draw channel waveform names
        canvas->setTextBound(0, 0, canvas->width(), canvas->height());
        canvas->setTextColor(lilka::colors::White, lilka::colors::White);
        canvas->drawTextAligned(
            "##", SCORE_COUNTER_WIDTH / 2, SCORE_HEADER_TOP + ITEM_HEIGHT / 2, lilka::ALIGN_CENTER, lilka::ALIGN_CENTER
        );

        for (int channelIndex = 0; channelIndex < CHANNEL_COUNT; channelIndex++) {
            bool isChannelWaveformFocused = activeBlock == BLOCK_CONTROLS && controlCursorY == CONTROL_ROW_3_PATTERNS &&
                                            controlCursorX == channelIndex;
            sprintf(str, "%02X", page->patternIndices[channelIndex]);
            drawElement(
                str,
                SCORE_COUNTER_WIDTH + channelIndex * SCORE_EVENT_WIDTH,
                SCORE_HEADER_TOP + ITEM_HEIGHT / 2,
                lilka::ALIGN_START,
                lilka::ALIGN_CENTER,
                isChannelWaveformFocused && isEditing,
                isChannelWaveformFocused,
                lilka::colors::Uranian_blue
            );
        }

        // Draw score
        canvas->setFont(SCORE_FONT);
        for (int eventIndex = 0; eventIndex < CHANNEL_SIZE; eventIndex++) {
            int scoreRowIndex = eventIndex - scoreCursorY + SCORE_MIDDLE_ROW_INDEX - 1;
            if (scoreRowIndex < 0 || scoreRowIndex >= SCORE_ROW_COUNT) {
                // Not visible
                continue;
            }
            // Top coordinate of the score item on screen
            int16_t y = SCORE_TOP + scoreRowIndex * SCORE_ITEM_HEIGHT;
            // Draw score, with current event in the middle
            if (eventIndex % 4 == 0) {
                canvas->fillRect(0, y, canvas->width(), SCORE_ITEM_HEIGHT, lilka::colors::Delft_blue);
            }
            if (eventIndex == scoreCursorY && activeBlock == BLOCK_EVENT_SCORE) {
                canvas->drawRect(0, y, canvas->width(), SCORE_ITEM_HEIGHT, lilka::colors::Blue);
            }
            canvas->setTextColor(lilka::colors::White, lilka::colors::White);
            sprintf(str, "%2d", eventIndex);
            canvas->drawTextAligned(
                str, SCORE_COUNTER_WIDTH / 2, y + SCORE_ITEM_HEIGHT / 2, lilka::ALIGN_CENTER, lilka::ALIGN_CENTER
            );
            for (int channelIndex = 0; channelIndex < CHANNEL_COUNT; channelIndex++) {
                Pattern* pattern = track.getPattern(page->patternIndices[channelIndex]);
                event_t event = pattern->getChannelEvent(channelIndex, eventIndex);
                int xOffset = SCORE_COUNTER_WIDTH + channelIndex * SCORE_EVENT_WIDTH;
                if (event.type == EVENT_TYPE_CONT) {
                    strcpy(str, "...");
                } else if (event.type == EVENT_TYPE_NORMAL) {
                    strcpy(str, event.note.toStr());
                } else if (event.type == EVENT_TYPE_OFF) {
                    strcpy(str, "OFF");
                } else {
                    strcpy(str, "???");
                }
                bool eventFocused =
                    activeBlock == BLOCK_EVENT_SCORE && channelIndex == currentChannel && scoreCursorY == eventIndex;
                // Note
                xOffset += drawElement(
                    str,
                    xOffset,
                    y + SCORE_ITEM_HEIGHT / 2,
                    lilka::ALIGN_START,
                    lilka::ALIGN_CENTER,
                    eventFocused && isEditing,
                    eventFocused && currentSegment == SEGMENT_NOTE,
                    event.type == EVENT_TYPE_CONT ? lilka::colors::Battleship_gray : lilka::colors::White
                );
                // Waveform
                int8_t fontShiftY = 0;
                if (event.waveform == WAVEFORM_CONT) {
                    sprintf(str, ".");
                } else {
                    sprintf(str, "%d", event.waveform - 1);
                    canvas->setFont(FONT_ICONS);
                    // I have absolutely no idea why does the icon font end up aligned incorrectly, and I'm too lazy to fix it properly.
                    fontShiftY = 3;
                }
                drawElement(
                    str,
                    xOffset,
                    y + SCORE_ITEM_HEIGHT / 2 + fontShiftY,
                    lilka::ALIGN_START,
                    lilka::ALIGN_CENTER,
                    eventFocused && isEditing,
                    eventFocused && currentSegment == SEGMENT_WAVEFORM,
                    event.waveform == WAVEFORM_CONT ? lilka::colors::Battleship_gray : lilka::colors::Cyan
                );
                canvas->setFont(SCORE_FONT);
                xOffset += 9;
                // Volume
                if (event.volume == 0) {
                    sprintf(str, "..");
                } else {
                    sprintf(str, "%02X", event.volume);
                }
                xOffset += drawElement(
                    str,
                    xOffset,
                    y + SCORE_ITEM_HEIGHT / 2,
                    lilka::ALIGN_START,
                    lilka::ALIGN_CENTER,
                    eventFocused && isEditing,
                    eventFocused && currentSegment == SEGMENT_VOLUME,
                    event.volume == 0 ? lilka::colors::Battleship_gray : lilka::colors::Green
                );
                // Effect
                sprintf(str, "%c", effect_signs[event.effect.type]);
                xOffset += drawElement(
                    str,
                    xOffset,
                    y + SCORE_ITEM_HEIGHT / 2,
                    lilka::ALIGN_START,
                    lilka::ALIGN_CENTER,
                    eventFocused && isEditing,
                    eventFocused && currentSegment == SEGMENT_EFFECT,
                    event.effect.type == EFFECT_TYPE_NONE ? lilka::colors::Battleship_gray : lilka::colors::Yellow
                );
                // Effect param
                sprintf(str, "%02X", event.effect.param);
                xOffset += drawElement(
                    str,
                    xOffset,
                    y + SCORE_ITEM_HEIGHT / 2,
                    lilka::ALIGN_START,
                    lilka::ALIGN_CENTER,
                    eventFocused && isEditing,
                    eventFocused && currentSegment == SEGMENT_EFFECT_PARAM,
                    event.effect.type == EFFECT_TYPE_NONE ? lilka::colors::Battleship_gray : lilka::colors::Orange
                );
                xOffset += 4;
                (void)xOffset;
            }
        }
        lilka::State state = lilka::controller.getState();

        if (activeBlock == BLOCK_CONTROLS) {
            if (!seqState.playing) {
                if (isEditing) {
                    if (state.a.justPressed) {
                        // Exit edit mode
                        isEditing = false;
                    }
                    if (state.up.justPressed || state.down.justPressed || state.left.justPressed ||
                        state.right.justPressed) {
                        if (controlCursorY == CONTROL_ROW_0_BUTTONS) {
                            // Unreachable
                        } else if (controlCursorY == CONTROL_ROW_1_SETTINGS) {
                            if (controlCursorX == 0) {
                                // Select page
                                if (state.up.justPressed || state.left.justPressed) {
                                    pageIndex = (pageIndex - 1 + track.getPageCount()) % track.getPageCount();
                                } else if (state.down.justPressed || state.right.justPressed) {
                                    pageIndex = (pageIndex + 1) % track.getPageCount();
                                }
                            } else if (controlCursorX == 1) {
                                // Adjust BPM
                                if (state.up.justPressed) {
                                    track.setBPM(track.getBPM() + 1);
                                } else if (state.down.justPressed) {
                                    track.setBPM(track.getBPM() - 1);
                                } else if (state.left.justPressed) {
                                    track.setBPM(track.getBPM() - 5);
                                } else if (state.right.justPressed) {
                                    track.setBPM(track.getBPM() + 5);
                                }
                            } else if (controlCursorX == 2) {
                                // Adjust length
                                if (state.up.justPressed) {
                                    track.setPageCount(track.getPageCount() + 1);
                                } else if (state.down.justPressed) {
                                    track.setPageCount(track.getPageCount() - 1);
                                    if (pageIndex >= track.getPageCount()) {
                                        // Adjust current page if it's out of bounds
                                        pageIndex = track.getPageCount() - 1;
                                    }
                                }
                            }
                        } else if (controlCursorY == CONTROL_ROW_2_SETTINGS) {
                            if (controlCursorX == 0) {
                                // No-op
                            } else if (controlCursorX == 1) {
                                // Unreachable
                            } else if (controlCursorX == 2) {
                                // Adjust master volume
                                if (state.up.justPressed) {
                                    sequencer.setMasterVolume(sequencer.getMasterVolume() + 0.05);
                                } else if (state.down.justPressed) {
                                    sequencer.setMasterVolume(sequencer.getMasterVolume() - 0.05);
                                }
                            }
                        } else if (controlCursorY == CONTROL_ROW_3_PATTERNS) {
                            // Select waveform for pattern's channel
                            if (state.left.justPressed) {
                                // Previous channel
                                controlCursorX = (controlCursorX - 1 + CHANNEL_COUNT) % CHANNEL_COUNT;
                            } else if (state.right.justPressed) {
                                // Next channel
                                controlCursorX = (controlCursorX + 1) % CHANNEL_COUNT;
                            } else if (state.up.justPressed) {
                                // Previous pattern
                                page->patternIndices[controlCursorX] =
                                    (page->patternIndices[controlCursorX] - 1 + track.getPatternCount()) %
                                    track.getPatternCount();
                            } else if (state.down.justPressed) {
                                // Next pattern (auto-resize, unused patterns will not be saved)
                                page->patternIndices[controlCursorX]++;
                            }
                        }
                    }
                } else {
                    if (state.a.justPressed) {
                        // Enter edit mode

                        if (controlCursorY == CONTROL_ROW_0_BUTTONS) {
                            if (controlCursorX == 0) {
                                // Load
                                String filename = filePicker(".lt", false);
                                if (filename.length()) {
                                    loadTrack(&track, filename);
                                    pageIndex = 0;
                                    scoreCursorX = 0;
                                    scoreCursorY = 0;
                                }
                            } else if (controlCursorX == 1) {
                                // Save
                                String filename = filePicker(".lt", true);
                                if (filename.length()) {
                                    saveTrack(&track, filename);
                                }
                            } else if (controlCursorX == 2) {
                                // Reset
                                if (confirm("Увага", "Очистити всі дані\nкомпозиції?")) {
                                    track.reset();
                                    pageIndex = 0;
                                    scoreCursorX = 0;
                                    scoreCursorY = 0;
                                }
                            }
                        } else if (controlCursorY == CONTROL_ROW_2_SETTINGS && (controlCursorX == 0)) {
                            // No-op
                        } else if (controlCursorY == CONTROL_ROW_2_SETTINGS && (controlCursorX == 1)) {
                            // Save WAV
                            String filename = filePicker(".wav", true);
                            if (filename.length()) {
                                wavSink = new WAVSink(filename);
                                sequencer.setSink(wavSink);
                                sequencer.play(&track, pageIndex, false);
                            }
                        } else {
                            isEditing = true;
                        }
                    }
                    if (state.up.justPressed) {
                        // We add 1 to CONTROL_ROWS to account for the score header row which is not part of the control, but is managed by the same cursor
                        controlCursorY = (controlCursorY - 1 + (CONTROL_ROWS + 1)) % (CONTROL_ROWS + 1);
                    } else if (state.down.justPressed) {
                        controlCursorY = (controlCursorY + 1) % (CONTROL_ROWS + 1);
                    } else if (state.left.justPressed) {
                        // This assumes that control column count is the same as channel count
                        controlCursorX = (controlCursorX - 1 + CHANNEL_COUNT) % CHANNEL_COUNT;
                    } else if (state.right.justPressed) {
                        controlCursorX = (controlCursorX + 1) % CHANNEL_COUNT;
                    }
                    if (state.select.justPressed) {
                        activeBlock = (activeBlock + 1) % BLOCK_COUNT;
                    }
                }
            }
        } else if (activeBlock == BLOCK_EVENT_SCORE) {
            if (isEditing) {
                // Edit mode
                if (state.a.justPressed) {
                    // Exit edit mode
                    sequencer.stop();
                    isPreviewing = false;
                    isEditing = false;
                }
                if (state.up.justPressed || state.down.justPressed || state.left.justPressed ||
                    state.right.justPressed || state.c.justPressed) {
                    // Adjust note
                    Pattern* pattern = track.getPattern(page->patternIndices[currentChannel]);
                    event_t event = pattern->getChannelEvent(currentChannel, scoreCursorY);
                    if (currentSegment == SEGMENT_NOTE) {
                        if (state.c.justPressed) {
                            event.type = static_cast<event_type_t>((event.type + 1) % EVENT_TYPE_COUNT);
                        } else {
                            if (event.type != EVENT_TYPE_NORMAL) {
                                event.type = EVENT_TYPE_NORMAL;
                                // Find and use previous note (if any)
                                bool found = false;
                                for (int i = scoreCursorY - 1; i >= 0; i--) {
                                    event_t prevEvent = pattern->getChannelEvent(currentChannel, i);
                                    if (prevEvent.type == EVENT_TYPE_NORMAL) {
                                        event.note = prevEvent.note;
                                        found = true;
                                        break;
                                    }
                                }
                                if (!found) {
                                    event.note = NOTE_ZERO;
                                }
                            }
                            if (state.up.justPressed) {
                                event.note.add(1);
                            } else if (state.down.justPressed) {
                                event.note.add(-1);
                            }
                            if (state.left.justPressed) {
                                event.note.add(-12);
                            } else if (state.right.justPressed) {
                                event.note.add(12);
                            }
                        }
                    } else if (currentSegment == SEGMENT_WAVEFORM) {
                        // Adjust waveform
                        if (state.up.justPressed) {
                            event.waveform = static_cast<waveform_t>((event.waveform + 1) % WAVEFORM_COUNT);
                        } else if (state.down.justPressed) {
                            event.waveform =
                                static_cast<waveform_t>((event.waveform - 1 + WAVEFORM_COUNT) % WAVEFORM_COUNT);
                        }
                    } else if (currentSegment == SEGMENT_VOLUME) {
                        // Adjust volume
                        if (state.up.justPressed) {
                            event.volume = (event.volume + 1) % (MAX_VOLUME + 1);
                        } else if (state.down.justPressed) {
                            event.volume = (event.volume - 1 + (MAX_VOLUME + 1)) % (MAX_VOLUME + 1);
                        }
                        if (state.left.justPressed) {
                            event.volume = (event.volume - 16 + (MAX_VOLUME + 1)) % (MAX_VOLUME + 1);
                        } else if (state.right.justPressed) {
                            event.volume = (event.volume + 16) % (MAX_VOLUME + 1);
                        }
                    } else if (currentSegment == SEGMENT_EFFECT) {
                        // Adjust effect
                        if (state.up.justPressed) {
                            event.effect.type = static_cast<effect_type_t>((event.effect.type + 1) % EFFECT_TYPE_COUNT);
                        } else if (state.down.justPressed) {
                            event.effect.type = static_cast<effect_type_t>(
                                (event.effect.type - 1 + EFFECT_TYPE_COUNT) % EFFECT_TYPE_COUNT
                            );
                        }
                    } else if (currentSegment == SEGMENT_EFFECT_PARAM) {
                        // Adjust effect param
                        if (state.up.justPressed) {
                            event.effect.param = (event.effect.param + 1) % 256;
                        } else if (state.down.justPressed) {
                            event.effect.param = (event.effect.param - 1 + 256) % 256;
                        }
                        if (state.left.justPressed) {
                            event.effect.param = (event.effect.param - 16 + 256) % 256;
                        } else if (state.right.justPressed) {
                            event.effect.param = (event.effect.param + 16) % 256;
                        }
                    }
                    pattern->setChannelEvent(currentChannel, scoreCursorY, event);
                    if (isPreviewing) {
                        // Update preview
                        sequencer.stop();
                        sequencer.play(&track, pageIndex, currentChannel, scoreCursorY);
                    }
                }
                if (state.b.justPressed) {
                    // Play single event
                    sequencer.stop();
                    sequencer.play(&track, pageIndex, currentChannel, scoreCursorY);
                    isPreviewing = true;
                } else if (state.b.justReleased) {
                    // Stop playing single event
                    sequencer.stop();
                    isPreviewing = false;
                }
            } else {
                // Normal mode
                if (seqState.playing) {
                    // Is playing
                    scoreCursorY = seqState.eventIndex;
                    if (state.start.justPressed || (isPreviewing && state.b.justReleased)) {
                        // Stop playing
                        isPreviewing = false;
                        sequencer.stop();
                    }

                    // Adjust master volume
                    if (state.up.justPressed) {
                        sequencer.setMasterVolume(sequencer.getMasterVolume() + 0.05);
                    } else if (state.down.justPressed) {
                        sequencer.setMasterVolume(sequencer.getMasterVolume() - 0.05);
                    }
                } else {
                    // Not playing
                    if (state.b.justPressed) {
                        // Play all events from this row
                        sequencer.stop();
                        sequencer.play(&track, pageIndex, -1, scoreCursorY);
                        isPreviewing = true;
                    } else if (state.b.justReleased) {
                        // Stop playing all events from this row
                        sequencer.stop();
                        isPreviewing = false;
                    }

                    if (state.c.justPressed) {
                        // Copy event
                        Pattern* pattern = track.getPattern(page->patternIndices[currentChannel]);
                        copiedEvent = pattern->getChannelEvent(currentChannel, scoreCursorY);
                    } else if (state.d.justPressed) {
                        // Paste event
                        Pattern* pattern = track.getPattern(page->patternIndices[currentChannel]);
                        pattern->setChannelEvent(currentChannel, scoreCursorY, copiedEvent);
                    }

                    if (state.a.justPressed) {
                        // Enter edit mode
                        sequencer.stop();
                        isEditing = true;
                        isPreviewing = false;
                    }

                    if (state.up.justPressed) {
                        scoreCursorY = (scoreCursorY - 1 + CHANNEL_SIZE) % CHANNEL_SIZE;
                    } else if (state.down.justPressed) {
                        scoreCursorY = (scoreCursorY + 1) % CHANNEL_SIZE;
                    } else if (state.left.justPressed) {
                        scoreCursorX =
                            (scoreCursorX - 1 + CHANNEL_COUNT * SEGMENT_COUNT) % (CHANNEL_COUNT * SEGMENT_COUNT);
                    } else if (state.right.justPressed) {
                        scoreCursorX = (scoreCursorX + 1) % (CHANNEL_COUNT * SEGMENT_COUNT);
                    }

                    if (state.start.justPressed) {
                        // Start playing
                        sequencer.play(&track, pageIndex, true);
                    }
                    if (state.a.justPressed) {
                        // Enter edit mode
                        isEditing = true;
                    }
                    if (state.select.justPressed && !isPreviewing) {
                        activeBlock = (activeBlock + 1) % BLOCK_COUNT;
                    }
                }
            }
            if (seqState.playing && state.select.justPressed) {
                visualizerMode = (visualizerMode + 1) % VISUALIZER_MODE_COUNT;
            }
        }

        queueDraw();
    }
}

int LilTrackerApp::drawElement(
    const char* text, int16_t x, int16_t y, lilka::Alignment hAlign, lilka::Alignment vAlign, bool editing,
    bool focused, uint16_t color
) {
    uint16_t textColor;
    uint16_t fillColor;
    uint16_t outlineColor;
    if (focused) {
        if (editing) {
            textColor = lilka::colors::Black;
            fillColor = color;
            outlineColor = 0;
        } else {
            textColor = color;
            fillColor = 0;
            outlineColor = color;
        }
    } else {
        textColor = color;
        fillColor = 0;
        outlineColor = 0;
    }
    canvas->setTextColor(textColor);
    int16_t _x, _y;
    uint16_t _w, _h;
    canvas->getTextBoundsAligned(text, x, y, hAlign, vAlign, &_x, &_y, &_w, &_h);
    if (fillColor) {
        canvas->fillRect(_x, _y, _w, _h, fillColor);
    } else if (outlineColor) {
        canvas->drawRect(_x, _y, _w, _h, outlineColor);
    }
    return canvas->drawTextAligned(text, x, y, hAlign, vAlign);
}

// void LilTrackerApp::startPreview(
//     Track* track, page_t* page, int8_t requestedChannelIndex, uint16_t requestedEventIndex
// ) {
//     for (int channelIndex = 0; channelIndex < CHANNEL_COUNT; channelIndex++) {
//         Pattern* pattern = track->getPattern(page->patternIndices[channelIndex]);
//         event_t event = pattern->getChannelEvent(channelIndex, requestedEventIndex);
//         bool shouldPlayThisChannel = requestedChannelIndex == -1 || requestedChannelIndex == channelIndex;
//         if (shouldPlayThisChannel && event.type == EVENT_TYPE_NORMAL) {
//             // Find waveform for this event by iterating up
//             waveform_t waveform = WAVEFORM_SQUARE; // Default to square if no waveform found
//             for (int i = requestedEventIndex; i >= 0; i--) {
//                 event_t prevEvent = pattern->getChannelEvent(channelIndex, i);
//                 if (prevEvent.waveform != WAVEFORM_CONT) {
//                     waveform = prevEvent.waveform;
//                     break;
//                 }
//             }
//             sequencer.play(
//                 &track,
//                 channelIndex,
//                 waveform,
//                 event.note.toFrequency(),
//                 event.volume > 0 ? ((float)event.volume) / MAX_VOLUME : 1.0,
//                 event.effect
//             );
//         }
//     }
// }

void LilTrackerApp::alert(String title, String message) {
    lilka::Alert alertDialog(title, message);
    alertDialog.draw(canvas);
    queueDraw();
    while (!alertDialog.isFinished()) {
        alertDialog.update();
    }
}

bool LilTrackerApp::confirm(String title, String message) {
    lilka::Alert confirmDialog(title, message + "\n\n[START] - Так\n[A] - Ні");
    confirmDialog.addActivationButton(lilka::Button::START);
    confirmDialog.draw(canvas);
    queueDraw();
    while (!confirmDialog.isFinished()) {
        confirmDialog.update();
    }
    return confirmDialog.getButton() == lilka::Button::START;
}

String LilTrackerApp::filePicker(String ext, bool isSave) {
    // isSave determines whether we are writing to file or opening an existing one

    // List files
    if (!SD.exists(LILTRACKER_DIR)) {
        if (!SD.mkdir(LILTRACKER_DIR)) {
            alert(K_S_ERROR, StringFormat(K_S_CANT_CREATE_DIR_FMT, LILTRACKER_DIR));
            return "";
        }
    }
    File root = SD.open("/liltracker");
    if (!root) {
        alert(K_S_ERROR, StringFormat(K_S_CANT_OPEN_DIR_FMT, LILTRACKER_DIR));
        return "";
    }

    int fileCount = lilka::fileutils.getEntryCount(&SD, LILTRACKER_DIR);
    if (fileCount == 0 && !isSave) {
        alert(K_S_ERROR, StringFormat(K_S_DIR_EMPTY_FMT, LILTRACKER_DIR));
        return "";
    }

    lilka::Entry entries[fileCount];
    std::vector<String> filenames;
    lilka::fileutils.listDir(&SD, LILTRACKER_DIR, entries);

    lilka::Menu menu(isSave ? K_S_LILTRACKER_SAVE_TRACK : K_S_LILTRACKER_OPEN_TRACK);
    if (isSave) {
        menu.addItem(K_S_LILTRACKER_CREATE_NEW_TRACK);
    }
    for (int i = 0; i < fileCount; i++) {
        if (entries[i].type == lilka::EntryType::ENT_DIRECTORY) {
            continue;
        }
        if (!entries[i].name.endsWith(ext)) {
            continue;
        }
        menu.addItem(entries[i].name);
        filenames.push_back(entries[i].name);
    }
    menu.addItem(K_S_MENU_BACK);

    while (!menu.isFinished()) {
        menu.update();
        menu.draw(canvas);
        queueDraw();
    }

    int16_t selectedItem = menu.getCursor();
    if (selectedItem == menu.getItemCount() - 1) {
        // Back
        return "";
    }

    if (isSave) {
        if (selectedItem == 0) {
            // Create new file
            lilka::InputDialog dialog(K_S_LILTRACKER_ENTER_FILENAME);
            while (1) {
                dialog.update();
                dialog.draw(canvas);
                queueDraw();
                if (dialog.isFinished()) {
                    String filename = dialog.getValue();
                    filename.trim();
                    if (filename.length() == 0) {
                        alert(K_S_ERROR, K_S_LILTRACKER_FILENAME_CANT_BE_EMPTY);
                    } else {
                        String lowerFilename = filename;
                        lowerFilename.toLowerCase();
                        if (lowerFilename.endsWith(ext)) {
                            filename.remove(filename.length() - 3);
                        }
                        filename += ext;
                        return String(LILTRACKER_DIR) + "/" + filename;
                    }
                }
            }
        } else {
            // Save to existing file
            selectedItem--; // Skip the "Create new" item
            return String(LILTRACKER_DIR) + "/" + filenames[selectedItem];
        }
    } else {
        // Open existing file
        return String(LILTRACKER_DIR) + "/" + filenames[selectedItem];
    }
}

void LilTrackerApp::loadTrack(Track* track, String filename) {
    File file = SD.open(filename, FILE_READ);
    if (!file) {
        alert(K_S_ERROR, StringFormat(K_S_CANT_OPEN_FILE_FMT, filename.c_str()));
        return;
    }
    Defer closeFile([&file]() { file.close(); });
    uint8_t* buff = new uint8_t[file.size()];
    std::unique_ptr<uint8_t[]> buffPtr(buff);
    file.read(buff, file.size());
    track->readFromBuffer(buff);
}

void LilTrackerApp::saveTrack(Track* track, String filename) {
    if (SD.exists(filename)) {
        // Remove existing file
        if (!SD.remove(filename)) {
            alert(K_S_ERROR, StringFormat(K_S_CANT_REMOVE_FILE_FMT, filename.c_str()));
            return;
        }
    }
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        alert(K_S_ERROR, StringFormat(K_S_CANT_OPEN_FILE_FMT, filename.c_str()));
        return;
    }
    Defer closeFile([&file]() { file.close(); });
    uint8_t* buff = new uint8_t[track->calculateWriteBufferSize()];
    std::unique_ptr<uint8_t[]> buffPtr(buff);
    track->writeToBuffer(buff);
    file.write(buff, track->calculateWriteBufferSize());
}
