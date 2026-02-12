#include "txtview.h"
#include "keira/keira_macro.h"
#include <errno.h>
// ===== MEMORY
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

size_t shiftMemLeft(char* block, const size_t shiftAmount, const size_t contentLength, const size_t blockSize) {
    if (!block || blockSize == 0 || contentLength == 0) {
        return 0;
    }

    // Cap toShift to contentLength
    size_t toShift = (shiftAmount > contentLength) ? contentLength : shiftAmount;

    // Calculate new content length after shift
    size_t newContentLength = contentLength - toShift;

    // Move memory left (memmove handles overlapping regions)
    memmove(block, block + toShift, newContentLength);

    // Zero out the freed space at the end
    if (newContentLength < blockSize) {
        memset(block + newContentLength, 0, blockSize - newContentLength);
    }

    return toShift;
}

// Shift memory to the right (for scrolling up - adds space at start)
// Returns the actual number of bytes shifted
size_t shiftMemRight(char* block, const size_t shiftAmount, const size_t contentLength, const size_t blockSize) {
    if (!block || blockSize == 0) {
        return 0;
    }

    // Cap toShift to available space
    size_t availableSpace = blockSize - contentLength;
    size_t toShift = (shiftAmount > availableSpace) ? availableSpace : shiftAmount;

    if (toShift == 0) {
        return 0;
    }

    // Move memory right (memmove handles overlapping regions)
    memmove(block + toShift, block, contentLength);

    // Zero out the space at the beginning
    memset(block, 0, toShift);

    return toShift;
}

// ===== END MEMORY

// ===== CANVAS
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
    // TODO: add caching to this one, store values in noffs_cache, implement in a form of macro,
    // stick back to original implementation in case nothing found in cache
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

// ===== END CANVAS

// ===== TXT VIEW
TxtView::TxtView() {
    TXT_DBG LEP;
}

// TODO: PSRAM VFS for other cases?
void TxtView::setTextFile(const String& fPath) {
    TXT_DBG LEP;
    // TODO: reset all used vars, caches, offsets here
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
    vTaskDelay(pdMS_TO_TICKS(10));
}

void TxtView::updateButtons() {
    // TXT_DBG LEP;
    auto state = lilka::controller.getState();

    /* 
    TODO: to be restored after large single line blocks optimizations
    BY LINE/BY PAGE SCROLL 
    if (state.up.justPressed) scrollUp();
    else if (state.down.justPressed) scrollDown();
    else if (state.left.justPressed) scrollPageUp();
    else if (state.right.justPressed) scrollPageDown();
    */

    // BY Page only scroll
    if (state.up.justPressed) scrollPageUp();
    else if (state.down.justPressed) scrollPageDown();
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
    // TODO: fix first few frames problem, displays this stuff when nothing loaded yet
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
    if (availableHeight % (lineHeight + spacing) >= lineHeight) {
        maxLines++;
    }

    size_t countDisplayedLines = 0;
    size_t countDisplayedBytes = 0;

    for (size_t i = 0; i < dptrs.size(); i++) {
        char* lineStart = dptrs[i];
        char* lineEnd = (i + 1 == dptrs.size()) ? tBlock + tLen : dptrs[i + 1]; // last line check

        char backup = *lineEnd;
        *lineEnd = '\0';

        canvas->setCursor(TXT_MARGIN_LEFT, y);
        // TODO: add ANSI colors support here, would be fun
        canvas->print(lineStart);
        // TXT_DBG lilka::serial.log("drawing %s, Length %d, ULength %d", lineStart, strlen(lineStart), lilka::sutils.ulength(lineStart));
        *lineEnd = backup;

        y += lineHeight + spacing;
        countDisplayedLines++;
        countDisplayedBytes += (lineEnd - lineStart);
        if (y > canvas->height() - TXT_MARGIN_BOTTOM) break;
    }
    lastDisplayedLines = countDisplayedLines;
    lastDisplayedBytes = countDisplayedBytes;

    // sleep
}

void TxtView::tBlockRefresh() {
    // TXT_DBG LEP;
    if (fp == NULL) {
        TXT_DBG lilka::serial.log("File not open. Skiping");
        return; // nothing to refresh. Wait till next update
    }

    long toReadFrom = ftell(fp);

    // check if we read it already
    bool alreadyRead = tOffset == toReadFrom;
    if (alreadyRead) {
        tBlockRefreshRequired = false;
        return;
    }

    // should be read anyways in this scenario
    bool initalOffsetOrCantBeReadLazy = (tOffset == -1) || (labs(toReadFrom - tOffset) >= TXT_MAX_BLOCK_SIZE / 2);
    if (initalOffsetOrCantBeReadLazy) {
        // Zero mem
        memset(tBlock, 0, TXT_MAX_BLOCK_SIZE);
        tLen = fread(tBlock, 1, TXT_MAX_BLOCK_SIZE, fp);
    } else { // do the lazy read
        long offsetSignedDelta = toReadFrom - tOffset;
        if (offsetSignedDelta > 0) {
            // Moving forward: shift buffer left, refill at end
            size_t shifted = shiftMemLeft(tBlock, offsetSignedDelta, tLen, TXT_MAX_BLOCK_SIZE);

            // After shift, we have (tLen - shifted) bytes of old content
            // We need to read new data starting from the end of remaining content
            size_t remainingInBuffer = tLen - shifted; // This is safe, shifted <= tLen
            size_t spaceAvailable = TXT_MAX_BLOCK_SIZE - remainingInBuffer;

            // File position to read from: we've advanced by 'shifted' bytes
            // Old buffer started at tOffset, now it starts at (tOffset + shifted)
            // We need to read starting from (tOffset + tLen) which = toReadFrom + remainingInBuffer
            long filePos = tOffset + tLen; // or: toReadFrom + remainingInBuffer
            fseek(fp, filePos, SEEK_SET);

            // Read into end of buffer
            size_t actuallyRead = fread(tBlock + remainingInBuffer, 1, spaceAvailable, fp);
            tLen = remainingInBuffer + actuallyRead;

        } else if (offsetSignedDelta < 0) {
            size_t shiftAmount = (size_t)(-offsetSignedDelta);

            // Clamp shift amount first
            size_t actualShift = (shiftAmount > TXT_MAX_BLOCK_SIZE) ? TXT_MAX_BLOCK_SIZE : shiftAmount;

            // Clamp content to fit after shift
            size_t clampedContentLength = (actualShift >= TXT_MAX_BLOCK_SIZE) ? 0 : (TXT_MAX_BLOCK_SIZE - actualShift);

            if (clampedContentLength > tLen) {
                clampedContentLength = tLen;
            }

            size_t shifted = shiftMemRight(tBlock, actualShift, clampedContentLength, TXT_MAX_BLOCK_SIZE);

            fseek(fp, toReadFrom, SEEK_SET);
            size_t actuallyRead = fread(tBlock, 1, shifted, fp);

            tLen = actuallyRead + clampedContentLength;
        }
    }

    fseek(fp, toReadFrom, SEEK_SET); // save original file offset, could be useful for refresh
    tBlockRefreshRequired = false;
    tOffset = toReadFrom; // save current position

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

        char* pLineEnd = lilka::sutils.uforward(pLineStart); // first letter
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
                char* prev = lilka::sutils.ubackward(pLineEnd);
                if (prev == pLineEnd || prev < tBlock) break; // avoid infinite loop
                pLineEnd = prev;

                pLineStart = pLineEnd;
                continue;
            }

            *pLineEnd = backupChar;

            // advance safely
            char* next = lilka::sutils.uforward(pLineEnd);
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
    if (!canScroll(TXT_BACKWARD)) return;

    long anchorOffset = ftell(fp);

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
        for (const auto dptr : dptrs) {
            TXT_DBG lilka::serial.log("dptr off %x, anchor %x", TADDR2OFF(dptr), anchorOffset);
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
    if (!canScroll(TXT_FORWARD)) return;

    if (dptrs.size() > 1) {
        fseek(fp, TADDR2OFF(dptrs[1]), SEEK_SET);
    } else {
        fseek(fp, 0, SEEK_SET); // go begining
    }
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::scrollPageUp() {
    TXT_DBG LEP;
    if (!canScroll(TXT_BACKWARD)) return;
    TXT_DBG lilka::serial.log("Max lines = %d", maxLines);
    scrollUp(maxLines);
}

void TxtView::scrollPageDown() {
    TXT_DBG LEP;
    TXT_DBG lilka::serial.log("displayed lines = %d max lines = %d", lastDisplayedLines, maxLines);

    if (!canScroll(TXT_FORWARD)) return;

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
    return fp ? ftell(fp) : 0;
}

size_t TxtView::getCountDisplayedBytes() {
    return lastDisplayedBytes;
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

bool TxtView::canScroll(TxtScrollDirection TxtScrollDirection) {
    if (TxtScrollDirection == TXT_FORWARD)
        return fp && lastCanvas && (dptrs.size() > lastDisplayedLines) && (lastDisplayedLines == maxLines);
    else return fp && lastCanvas && ftell(fp) != 0;
}
// ===== END TXT VIEW
