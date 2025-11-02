#include "txtview.h"
#include "keira/keira_macro.h"
#include <errno.h>
#ifdef TXT_VIEWER_DEBUG
void checkOffs(const std::vector<char*>& noffs, const std::vector<char*>& doffs) {
    bool ok = true;

    // Monotonic increase
    for (size_t i = 1; i < noffs.size(); i++)
        if (noffs[i] <= noffs[i - 1]) {
            printf("Noffs not monotonic: %p after %p\n", noffs[i], noffs[i - 1]);
            ok = false;
        }

    for (size_t i = 1; i < doffs.size(); i++)
        if (doffs[i] <= doffs[i - 1]) {
            printf("Doffs not monotonic: %p after %p\n", doffs[i], doffs[i - 1]);
            ok = false;
        }

    // Every noff must exist exactly in doffs
    for (auto n : noffs) {
        bool found = false;
        for (auto d : doffs)
            if (d == n) { found = true; break; }
        if (!found) {
            printf("Missing doff for noff %p\n", n);
            ok = false;
        }
    }

    // Count rule
    if (doffs.size() <= noffs.size()) {
        printf("Count mismatch: doffs=%zu <= noffs=%zu\n",
               doffs.size(), noffs.size());
        ok = false;
    }

    if (ok)
        printf("Offsets OK\n");
}

#endif

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
    while ((*(unsigned char*)p & 0xC0) == 0x80)
        --p; // skip continuation bytes
    return p;
}
// get length of a first Unicode character
static inline size_t ulen(char* cstr) {
    auto nextchar = uforward(cstr);
    return nextchar - cstr;
}

// Get length in Unicode characters
size_t ulength(char* from, char* to = 0) {
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
bool isLineWithinCanvas(char* pLine, Arduino_GFX* canvas) {
    if (!pLine || !canvas) return false;

    int16_t x = canvas->getCursorX();
    int16_t y = canvas->getCursorY();
    int16_t bx, by;
    uint16_t w, h;

    canvas->getTextBounds(pLine, x, y, &bx, &by, &w, &h);

    // return (x + w <= canvas->width()-TXT_MARGIN_LEFT*2 && y + h <= canvas->height());
    // assume wraping disabled
    // TXT_DBG lilka::serial.log("Line = %s\nx =%d, y =%d, bx=%d, by=%d, w=%d, h=%d",pLine, x, y, bx, by, w, h);
    return w < canvas->width() - TXT_MARGIN_LEFT * 2;
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

bool TxtView::isFinished() {
    // TXT_DBG LEP;
    // TODO: move to ABSTRACT WIDGET
    if (done) {
        done = false;
        return true;
    }
    return false;
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
    TXT_DBG lilka::serial.log("Set file to %s", fPath.c_str());
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::update() {
    // TXT_DBG LEP;
    updateKeys();
    // We've to check if we actually can prepare data
    // Cause we've no real idea about canvas dimensions
    // it could be impossible to do
    // We still can read file though, but for simplification we
    // just wait a few cycles
    if (lastCanvas && tBlockRefreshRequired) {
        tBlockRefresh();
        nOffsRefresh();
        dOffsRefresh();
    }
}

void TxtView::updateKeys() {
    // TXT_DBG LEP;
    auto state = lilka::controller.getState();
    if (state.up.justPressed) scrollUp();
    else if (state.down.justPressed) scrollDown();
    else if (state.left.justPressed) scrollPageUp();
    else if (state.right.justPressed) scrollPageDown();
    else if (state.b.justPressed) done = true;
}

void TxtView::draw(Arduino_GFX* canvas) {
    // TXT_DBG LEP;
    setCanvasOptions(canvas);
    lastCanvas = canvas;

    // Doesn't look like we've something to display
    // could be a first run, so we have no data prepared
    if (doffs.empty()) return;

    // display doffs
    int16_t lineH = getLineHeight(canvas);
    int16_t y = lineH;

    for (size_t i = 0; i + 1 < doffs.size(); i++) {
        char* lineStart = doffs[i];
        char* lineEnd = doffs[i + 1];

        char backup = *lineEnd;
        *lineEnd = '\0';

        canvas->setCursor(TXT_MARGIN_LEFT, y);
        canvas->print(lineStart);
        // TXT_DBG lilka::serial.log("drawing %s, Length %d, ULength %d", lineStart, strlen(lineStart), ulength(lineStart));
        *lineEnd = backup;

        y += lineH;
        if (y > canvas->height() - STATUS_BAR_HEIGHT) break;
    }

    // sleep
}

void TxtView::tBlockRefresh() {
    TXT_DBG LEP;
    if (fp == NULL) {
        TXT_DBG lilka::serial.log("File not open. Skiping");
        return; // nothing to refresh. Wait till next update
    }

    // Zero mem
    memset(tBlock, 0, TXT_MAX_BLOCK_SIZE);
    // Read text block
    long curPos = ftell(fp);
    tLen = fread(tBlock, 1, TXT_MAX_BLOCK_SIZE, fp);
    fseek(fp, curPos, SEEK_SET); // save original file offset, could be useful for refresh
    tBlockRefreshRequired = false;

    TXT_DBG lilka::serial.log("Read %d bytes from %ld Position", tLen, curPos);
    TXT_DBG lilka::serial.log("tBlock full content:\n%.*s", tLen, tBlock);
}

void TxtView::nOffsRefresh() {
    TXT_DBG LEP;
    noffs.clear();
    if (!tLen || tBlockRefreshRequired) // Nothing read
        return;
    char* pCurrentChar = tBlock;
    char* pEndBlock = tBlock + tLen;
    noffs.push_back(pCurrentChar);
    // do not check last character here
    for (; pCurrentChar < pEndBlock - 1; pCurrentChar++) {
        if (*pCurrentChar == '\n') {
            noffs.push_back(pCurrentChar + 1);
            TXT_DBG lilka::serial.log("Adding noff at %p\n", noffs.back());
        }
    }
    TXT_DBG lilka::serial.log("Added %d noffs \n", noffs.size());
}
void TxtView::dOffsRefresh() {
    TXT_DBG LEP;

    if (!lastCanvas || tBlockRefreshRequired) {
        TXT_DBG lilka::serial.err("No access to lastCanvas, can't calc doffs");
        tBlockRefreshRequired = true;
        return;
    }

    // readjust canvas options if something changed
    setCanvasOptions(lastCanvas);

    doffs.clear();

    for (auto noff : noffs) {
        char* pLineStart = noff;

        // skip if noff points past the block
        if (pLineStart >= tBlock + tLen) continue;

        char* pLineEnd = uforward(pLineStart); // first letter
        if (pLineEnd > tBlock + tLen) pLineEnd = tBlock + tLen; // clamp

        while (pLineEnd < tBlock + tLen) {
            char backupChar = *pLineEnd;
            *pLineEnd = '\0';

            // reached newline
            if (*(pLineEnd - 1) == '\n') {
                doffs.push_back(pLineStart);
                TXT_DBG lilka::serial.log("Adding doff at %p\n", doffs.back());
                *pLineEnd = backupChar;
                break;
            }

            // check if line fits
            if (!isLineWithinCanvas(pLineStart, lastCanvas)) {
                doffs.push_back(pLineStart);
                // TXT_DBG lilka::serial.log("Adding doff at %p\n", doffs.back());
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
    }
}

void TxtView::scrollUp() {
    TXT_DBG LEP;
    if (!fp || doffs.empty() || !lastCanvas) return;

    long currentFileOffset = ftell(fp);
    long prevNLineOffset =
        flineback(fp, tBuffer, TXT_BUFFER_SIZE);
    // Can't go back
    if (currentFileOffset == 0) return;
    fseek(fp, prevNLineOffset, SEEK_SET);
    // Do refreshes inplace
    tBlockRefresh();
    nOffsRefresh();
    dOffsRefresh();

    // Test offsets
    TXT_DBG checkOffs(noffs, doffs);
    TXT_DBG lilka::serial.log("current offset %ld, prevline %ld", currentFileOffset, prevNLineOffset);
    // now we've to find doff before saved one
    bool found = false;
    char* nextOffset = doffs[0];
    for (auto doff : doffs) {
        if (OFF2ROFF(doff) == currentFileOffset) {
            found = true;
            break;
        }
        nextOffset = doff;
    }
    if (found) fseek(fp, OFF2ROFF(nextOffset), SEEK_SET);
    else {
        TXT_DBG lilka::serial.err("Doff not found, check doffs/noffs consistency");
        TXT_DBG checkOffs(noffs, doffs);
        return; // looks like we already here
    }
    // TODO: lazy reading maybe?
    // should be where need at next refresh
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::scrollDown() {
    TXT_DBG LEP;
    if (!fp) return;

    if (doffs.size() > 1) {
        fseek(fp, OFF2ROFF(doffs[1]), SEEK_SET);
    } else {
        fseek(fp, 0, SEEK_SET); // go begining
    }
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::scrollPageUp() {
    TXT_DBG LEP;
    if (!fp) return;
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::scrollPageDown() {
    TXT_DBG LEP;
    if (!fp) return;
    tBlockRefreshRequired = true; // to be done in update()
}

TxtView::~TxtView() {
    TXT_DBG LEP;
    if (!fp) return;
    // Never forget to close file
    if (fp) fclose(fp);
}

void TxtView::setColor(uint16_t color) {
    this->color = color;
}

void TxtView::setBgColor(uint16_t bgColor) {
    this->bgColor = bgColor;
};

void TxtView::setFont(const uint8_t* font) {
    this->font = font;
}

void TxtView::setCanvasOptions(Arduino_GFX* canvas) {
    canvas->setTextBound(
        TXT_MARGIN_LEFT, 0, canvas->width() - 2 * TXT_MARGIN_LEFT, canvas->height() - STATUS_BAR_HEIGHT
    );
    canvas->setCursor(TXT_MARGIN_LEFT, 0);
    canvas->fillScreen(bgColor);
    canvas->setTextColor(color);
    canvas->setTextWrap(false);
    canvas->setFont(font);
}