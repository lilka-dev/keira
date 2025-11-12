#include "txtview.h"
#include "keira/keira_macro.h"
#include <errno.h>

static inline FileEncoding detectEncodingByFileBlock(const char* block, size_t len) {
    if (len == 0) return TXT_EMPTY;

    const unsigned char* ptr = reinterpret_cast<const unsigned char*>(block);
    const unsigned char* end = ptr + len;
    bool hasHighBit = false;
    bool isValidUtf8 = true;
    bool hasNonWhitespace = false;

    while (ptr < end) {
        unsigned char c = *ptr;

        // Check for null bytes (binary indicator)
        if (c == 0) return TXT_BIN;

        // Track non-whitespace content
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            hasNonWhitespace = true;
        }

        // Check for other common binary indicators
        if (c < 0x20 && c != '\t' && c != '\n' && c != '\r') {
            return TXT_BIN;
        }

        // ASCII range
        if (c < 0x80) {
            ptr++;
            continue;
        }

        hasHighBit = true;

        // UTF-8 validation
        int extraBytes = 0;
        if ((c & 0xE0) == 0xC0) extraBytes = 1; // 110xxxxx
        else if ((c & 0xF0) == 0xE0) extraBytes = 2; // 1110xxxx
        else if ((c & 0xF8) == 0xF0) extraBytes = 3; // 11110xxx
        else {
            isValidUtf8 = false;
            break;
        }

        // Check if we have enough bytes left for the sequence
        if (ptr + extraBytes >= end) {
            // Incomplete sequence at end - not an error, just stop checking
            break;
        }

        // Check continuation bytes
        for (int i = 0; i < extraBytes; i++) {
            ptr++;
            if ((*ptr & 0xC0) != 0x80) {
                isValidUtf8 = false;
                break;
            }
        }

        if (!isValidUtf8) break;
        ptr++;
    }

    // If only whitespace found, treat as empty
    if (!hasNonWhitespace) return TXT_EMPTY;

    if (!hasHighBit) return TXT_UTF8; // Pure ASCII is valid UTF-8
    return isValidUtf8 ? TXT_UTF8 : TXT_LEGACY;
}

// move to next Unicode character
static inline char* uforward(char* cstr) {
    // TXT_DBG LEP;
    if (!cstr || !*cstr) return cstr;
    cstr++;
    while ((*cstr & 0xC0) == 0x80)
        cstr++; // skip continuation bytes
    return cstr;
}

// move to beginning of previous Unicode character
static inline char* ubackward(char* cstr) {
    // TXT_DBG LEP;
    if (!cstr) return cstr;
    char* p = cstr - 1;
    while ((*(reinterpret_cast<unsigned char*>(p)) & 0xC0) == 0x80)
        --p; // skip continuation bytes
    return p;
}
// get length of a first Unicode character
static inline size_t ulen(char* cstr) {
    const char* nextchar = uforward(cstr);
    return nextchar - cstr;
}

// Get length in Unicode characters
size_t ulength(char* from, const char* to = 0) {
    if (!from) return 0;

    size_t len = 0;
    char* ptr = from;

    if (!to) {
        while (*ptr) {
            ptr = uforward(ptr);
            len++;
        }
    } else {
        if (to - from <= 0) return 0;
        while (ptr < to && *ptr) {
            char* next = uforward(ptr);
            if (next > to) break; // don't go past 'to'
            ptr = next;
            len++;
        }
    }

    return len;
}

// expects bounds and cursor already set
static inline uint16_t getStringWidth(const char* pLine, Arduino_GFX* canvas) {
    if (!pLine || !canvas) return 0;

    int16_t x = canvas->getCursorX();
    int16_t y = canvas->getCursorY();
    int16_t bx, by;
    uint16_t w, h;

    canvas->getTextBounds(pLine, x, y, &bx, &by, &w, &h);
    return w;
}

// expects bounds and cursor already set
bool isLineWithinCanvas(const char* pLine, Arduino_GFX* canvas) {
    return getStringWidth(pLine, canvas) < canvas->width() - TXT_MARGIN_LEFT * 2;
}

long flineback(FILE* fp, char* buffer, size_t blength) {
    if (!fp || !buffer || blength == 0) return 0;

    long initialPos = ftell(fp);
    if (initialPos <= 0) return 0;

    long curPos = initialPos;
    int newLineCount = 0;

    while (curPos > 0) {
        // Move back by up to blength bytes
        long blockStart = curPos - (long)blength;
        if (blockStart < 0) blockStart = 0;

        size_t toRead = curPos - blockStart;
        fseek(fp, blockStart, SEEK_SET);
        size_t readBytes = fread(buffer, 1, toRead, fp);
        if (readBytes == 0) break;

        // Scan backward through buffer
        for (char* pC = buffer + readBytes - 1; pC >= buffer; pC--) {
            if (*pC == '\n') {
                newLineCount++;
                if (newLineCount == 2) {
                    curPos = blockStart + (pC - buffer) + 1;
                    fseek(fp, initialPos, SEEK_SET);
                    return curPos;
                }
            }
        }

        curPos = blockStart;
    }

    fseek(fp, initialPos, SEEK_SET);
    return 0; // reached file start
}

static inline int16_t getLineHeight(Arduino_GFX* canvas) {
    int16_t x, y;
    uint16_t w, h;
    canvas->getTextBounds("Ay", 0, 0, &x, &y, &w, &h);
    return h;
}

TxtView::TxtView() {
    TXT_DBG LEP;
}

// TODO: PSRAM VFS for other cases?
void TxtView::setTextFile(const String& fPath) {
    TXT_DBG LEP;
    if (fp) { // close old file
        fclose(fp);
        fp = NULL;
    }
    fp = fopen(fPath.c_str(), "r");
    if (fp == NULL) {
        lilka::serial.err("Tried to open file %s. Errno %d", fPath.c_str(), errno);
    }
    fseek(fp, 0, SEEK_END);
    fSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    setvbuf(fp, NULL, _IOFBF, 0); // disable std buffering for file
    TXT_DBG lilka::serial.log("Set file to %s", fPath.c_str());
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::update() {
    // TXT_DBG LEP;
    updateButtons();
    // We've to check if we actually can prepare data
    // Cause we've no real idea about canvas dimensions
    // it could be impossible to do
    // We still can read file though, but for simplification we
    // just wait a few cycles
    if (lastCanvas && tBlockRefreshRequired) {
        tBlockRefresh();
        nPtrRefresh();
        dPtrRefresh();
    }
}

void TxtView::updateButtons() {
    // TXT_DBG LEP;
    auto state = lilka::controller.getState();
    if (state.up.justPressed) scrollUp();
    else if (state.down.justPressed) scrollDown();
    else if (state.left.justPressed) scrollPageUp();
    else if (state.right.justPressed) scrollPageDown();
    else if (state.b.justPressed) done = true;

    for (lilka::Button activationButton : activationButtons) {
        const lilka::_StateButtons& buttonsArray = *reinterpret_cast<lilka::_StateButtons*>(&state);
        if (buttonsArray[activationButton].justPressed) {
            button = activationButton;
            done = true;
            // Should be made after done flag setup to allow to clear it by isFinished() call
            if (clbk) clbk(clbkData);
        }
    }
    vTaskDelay(LILKA_UI_UPDATE_DELAY_MS / portTICK_PERIOD_MS);
}

void TxtView::draw(Arduino_GFX* canvas) {
    // TXT_DBG LEP;
    lastCanvas = canvas;

    // Doesn't look like we've something to display
    // could be a first run, so we have no data prepared
    if (!fp) return;

    // Setup canvas options
    setCanvasOptions(canvas);

    // Determine line height
    int16_t lineHeight = getLineHeight(canvas);
    int16_t y = lineHeight; // counter

    // TODO: determine good font/size for this
    // Check if encoding supported
    if (encoding == TXT_EMPTY) {
        auto w = getStringWidth(TXT_S_EMPTY, canvas);
        canvas->setCursor((canvas->width() - w) / 2, (canvas->height() / 2) - lineHeight);
        canvas->print(TXT_S_EMPTY);
        return;
    } else if (encoding != TXT_UTF8) {
        auto w = getStringWidth(TXT_S_ENC_UNKNOWN, canvas);
        canvas->setCursor((canvas->width() - w) / 2, (canvas->height() / 2) - lineHeight);
        canvas->print(TXT_S_ENC_UNKNOWN);
        return;
    }

    // display dptrs
    int availableHeight = canvas->height() - TXT_MARGIN_BOTTOM;

    // Calculate the number of lines that can be displayed
    maxLines = availableHeight / (lineHeight + spacing);

    // Adjust if there is leftover space for a partial line
    if (availableHeight % (lineHeight + spacing) > 0) {
        maxLines++;
    }

    size_t countDisplayedLines = 0;
    for (size_t i = 0; i < dptrs.size(); i++) {
        char* lineStart = dptrs[i];
        char* lineEnd = (i + 1 == dptrs.size()) ? tBlock + tLen : dptrs[i + 1]; // last line check

        char backup = *lineEnd;
        *lineEnd = '\0';

        canvas->setCursor(TXT_MARGIN_LEFT, y);
        // TODO: add ANSI colors support here, would be fun
        canvas->print(lineStart);
        // TXT_DBG lilka::serial.log("drawing %s, Length %d, ULength %d", lineStart, strlen(lineStart), ulength(lineStart));
        *lineEnd = backup;

        y += lineHeight + spacing;
        countDisplayedLines++;
        if (y > canvas->height() - TXT_MARGIN_BOTTOM) break;
    }
    lastDisplayedLines = countDisplayedLines;

    // sleep
}

void TxtView::tBlockRefresh() {
    // TXT_DBG LEP;
    if (fp == NULL) {
        TXT_DBG lilka::serial.log("File not open. Skiping");
        return; // nothing to refresh. Wait till next update
    }

    // Zero mem
    memset(tBlock, 0, TXT_MAX_BLOCK_SIZE);
    // This zero memory may be not actually needed, and actually makes it slower
    // but what we've to make better is
    // TODO: shift already read block left/right and read not more than needed amount of bytes
    // Read text block
    long curPos = ftell(fp);
    tLen = fread(tBlock, 1, TXT_MAX_BLOCK_SIZE, fp);
    fseek(fp, curPos, SEEK_SET); // save original file offset, could be useful for refresh
    tBlockRefreshRequired = false;

    // Detect enoding
    if (encoding == TXT_UNKNOWN) {
        encoding = detectEncodingByFileBlock(tBlock, tLen);
    }
    // TXT_DBG lilka::serial.log("Read %d bytes from %ld Position", tLen, curPos);
    // TXT_DBG lilka::serial.log("tBlock full content:\n%.*s", tLen, tBlock);
    // TXT_DBG lilka::serial.log("Encoding %d", encoding);
}

void TxtView::nPtrRefresh(long maxoffset) {
    // TXT_DBG LEP;
    nptrs.clear();
    if (!lastCanvas) {
        TXT_DBG lilka::serial.err("No access to lastCanvas, can't calc dptrs");
        tBlockRefreshRequired = true;
        return;
    }
    if (tBlockRefreshRequired) {
        TXT_DBG lilka::serial.err("Txt block refresh requied, skiping...");
        return;
    }
    char* pCurrentChar = tBlock;
    const char* pEndBlock = tBlock + tLen;
    nptrs.push_back(pCurrentChar);
    // do not check last character here
    for (; pCurrentChar < pEndBlock - 1; pCurrentChar++) {
        if (*pCurrentChar == '\n') {
            if (maxoffset == -1 || maxoffset <= TADDR2OFF(pCurrentChar + 1)) nptrs.push_back(pCurrentChar + 1);
            // TXT_DBG lilka::serial.log("Adding dptr at %p\n", nptrs.back());
        }
    }
    // TXT_DBG lilka::serial.log("Added %d nptrs \n", nptrs.size());
}
void TxtView::dPtrRefresh(long maxoffset) {
    // TXT_DBG LEP;

    if (!lastCanvas) {
        TXT_DBG lilka::serial.err("No access to lastCanvas, can't calc dptrs");
        tBlockRefreshRequired = true;
        return;
    }
    if (tBlockRefreshRequired) {
        TXT_DBG lilka::serial.err("Txt block refresh requied, skiping...");
        return;
    }

    // readjust canvas options if something changed
    setCanvasOptions(lastCanvas);

    dptrs.clear();

    for (auto dptr : nptrs) {
        char* pLineStart = dptr;

        // add a bit of caching here
        // mostly lines would fit + - 1 character, except
        // really funky fonts, so maybe cache a bit and shift
        // or maybe leave it this way to make it universal enough

        // skip if dptr points past the block
        if (pLineStart >= tBlock + tLen) continue;

        char* pLineEnd = uforward(pLineStart); // first letter
        if (pLineEnd > tBlock + tLen) pLineEnd = tBlock + tLen; // clamp

        while (pLineEnd < tBlock + tLen) {
            char backupChar = *pLineEnd;
            *pLineEnd = '\0';

            // reached newline
            if (*(pLineEnd - 1) == '\n') {
                if (maxoffset == -1 || maxoffset <= TADDR2OFF(pLineStart)) dptrs.push_back(pLineStart);
                // TXT_DBG lilka::serial.log("Adding dptr at %p\n", dptrs.back());
                *pLineEnd = backupChar;
                break;
            }

            // check if line fits
            if (!isLineWithinCanvas(pLineStart, lastCanvas)) {
                if (maxoffset == -1 || maxoffset <= TADDR2OFF(pLineStart)) dptrs.push_back(pLineStart);
                // TXT_DBG lilka::serial.log("Adding dptr at %p\n", dptrs.back());
                *pLineEnd = backupChar;

                // move backward safely
                char* prev = ubackward(pLineEnd);
                if (prev == pLineEnd || prev < tBlock) break; // avoid infinite loop
                pLineEnd = prev;

                pLineStart = pLineEnd;
                continue;
            }

            *pLineEnd = backupChar;

            // advance safely
            char* next = uforward(pLineEnd);
            if (next <= pLineEnd || next > tBlock + tLen) break;
            pLineEnd = next;
        }
        // Add the final line
        if (pLineStart < tBlock + tLen) {
            if (dptrs.empty() || dptrs.back() != pLineStart) {
                if (maxoffset == -1 || maxoffset <= TADDR2OFF(pLineStart)) {
                    dptrs.push_back(pLineStart);
                }
            }
        }
        // TXT_DBG {
        //     if (!dptrs.empty()) {
        //         lilka::serial.log("Last dptr: %s", dptrs[dptrs.size() - 1]);
        //     }
        //     lilka::serial.log("pLineStart: %p (%s)", pLineStart, pLineStart);
        //     lilka::serial.log("pLineEnd: %p (%s)", pLineEnd, pLineEnd);
        //     lilka::serial.log("tBlock: %p", tBlock);
        //     lilka::serial.log("tBlock+tLen: %p", tBlock + tLen);
        //     lilka::serial.log("tLen: %zu", tLen);
        // }
        // TXT_DBG for (auto dptr:dptrs){ // AM HERE
        //     lilka::serial.log("dptr addr %x", TADDR2OFF(dptr));
        // }
    }
}

void TxtView::scrollUp(size_t linesToScroll) {
    TXT_DBG LEP;
    if (!fp || !lastCanvas) return;

    long anchorOffset = ftell(fp);
    // Can't go back
    if (anchorOffset == 0) return;

    TxtScrollDirection direction = TXT_BACKWARD;

    size_t skipLeft = linesToScroll;

    long currBlockOffset;

    while (1) {
        // go back a line
        if (direction == TXT_BACKWARD) {
            currBlockOffset = flineback(fp, tBuffer, TXT_BUFFER_SIZE);

        } else // carefully seek for an anchor cause it's lost(extremely long line found)
        {
            if (!dptrs.empty()) currBlockOffset = TADDR2OFF(dptrs[1]);
            else {
                TXT_DBG lilka::serial.err("oh fuck.. dptrs empty");
            }
        }

        // reload block
        fseek(fp, currBlockOffset, SEEK_SET);
        tBlockRefresh();

        // refresh constraints
        nPtrRefresh();
        dPtrRefresh();

        // not actually needed but let's say, why not check
        if (dptrs.empty()) {
            TXT_DBG lilka::serial.err("No dptrs found? Impossible");
            return;
        }

        // seek the anchor
        bool anchorFound = false;
        size_t anchorOffsetIndex = 0;
        // anchor offset index always zero, why?
        for (const auto dptr : dptrs) {
            TXT_DBG lilka::serial.log("dptr addr %x, anchor %x", TADDR2OFF(dptr), anchorOffset);
            if (TADDR2OFF(dptr) >= anchorOffset) {
                anchorFound = true;
                break;
            }
            anchorOffsetIndex++;
        }

        // captain, there's no anchor, we've to do find it or we gonna
        // swim in an ocean for all ethernity
        if (!anchorFound) {
            direction = TXT_FORWARD; // too much backward
            TXT_DBG lilka::serial.log("Anchor not found");
            continue; // skip the fuck
        }

        TXT_DBG lilka::serial.log(
            "Anchor [ index %d, offset %x ] Skip left [ %d ]", anchorOffsetIndex, anchorOffset, skipLeft
        );

        // our ship lays on a piece of land, there's no water around
        if (anchorOffsetIndex == 0) {
            TXT_DBG lilka::serial.log("what");
            break;
        }

        if (skipLeft > anchorOffsetIndex) { // we're close, but not yet
            TXT_DBG lilka::serial.log("wee, go next iteration");
            skipLeft = skipLeft - anchorOffsetIndex;
            anchorOffset = TADDR2OFF(dptrs[0]);
            fseek(fp, anchorOffset, SEEK_SET);
            direction = TXT_BACKWARD;
        } else { // yeh we did it :D
            TXT_DBG lilka::serial.log("all good, we know where to jump");
            anchorOffset = TADDR2OFF(dptrs[anchorOffsetIndex - skipLeft]);
            fseek(fp, anchorOffset, SEEK_SET);
            break;
        }
    }

    tBlockRefreshRequired = true;
}

void TxtView::scrollDown() {
    TXT_DBG LEP;
    // lock scrolling in case we've nothing new to display
    if (!fp || dptrs.empty() || lastDisplayedLines < maxLines) return;

    if (dptrs.size() > 1) {
        fseek(fp, TADDR2OFF(dptrs[1]), SEEK_SET);
    } else {
        fseek(fp, 0, SEEK_SET); // go begining
    }
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::scrollPageUp() {
    TXT_DBG LEP;
    if (!fp || !lastCanvas) return;
    long maxoffset = ftell(fp); // stick to current file position
    if (maxoffset == 0) return;
    TXT_DBG lilka::serial.log("Max lines = %d", maxLines);
    scrollUp(maxLines);
}

void TxtView::scrollPageDown() {
    TXT_DBG LEP;
    if (!fp || dptrs.empty() || lastDisplayedLines < maxLines) return;

    TXT_DBG lilka::serial.log("displayed lines = %d", lastDisplayedLines);
    // TODO: determine end of file reached
    // NOTE: test on empty file
    fseek(fp, TADDR2OFF(dptrs[lastDisplayedLines]), SEEK_SET);
    tBlockRefreshRequired = true; // to be done in update()
}

TxtView::~TxtView() {
    TXT_DBG LEP;
    // Never forget to close file
    if (fp) fclose(fp);
}

void TxtView::setColor(uint16_t color) {
    this->color = color;
}

void TxtView::setBackgroundColor(uint16_t bgColor) {
    this->bgColor = bgColor;
};

void TxtView::setFont(const uint8_t* font) {
    this->font = font;
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::setSpacing(uint16_t spacing) {
    this->spacing = spacing;
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::setTextSize(uint8_t textSize) {
    this->textSize = textSize;
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::jumpToOffset(long offset) {
    // TODO: jumpToOffset() implementation
    // jump in the middle, recalc nearest dptr, jump here, refresh block
}

long TxtView::getFileSize() {
    return fSize;
}

long TxtView::getOffset() {
    return ftell(fp);
}

void TxtView::setCallback(PTXTViewCallback clbk, void* clbkData) {
    this->clbk = clbk;
    this->clbkData = clbkData;
}

void TxtView::setCanvasOptions(Arduino_GFX* canvas) {
    canvas->setTextBound(
        TXT_MARGIN_LEFT, 0, canvas->width() - 2 * TXT_MARGIN_LEFT, canvas->height() - TXT_MARGIN_BOTTOM
    );
    canvas->setCursor(TXT_MARGIN_LEFT, 0);
    canvas->fillScreen(bgColor);
    canvas->setTextSize(textSize);
    canvas->setTextColor(color);
    canvas->setTextWrap(false);
    canvas->setFont(font);
}