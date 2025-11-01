#include "txtview.h"
#include <errno.h>

// move to next Unicode character
static inline char* uforward(char* cstr) {
    // TENTER
    if (!cstr || !*cstr) return cstr;
    cstr++;
    while ((*cstr & 0xC0) == 0x80)
        cstr++; // skip continuation bytes
    return cstr;
}

// move to beginning of previous Unicode character
static inline char* ubackward(char* cstr) {
    // TENTER
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
// Get length in unicode characters
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

// expects bounds and stuff to be set
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
    return w < canvas->width() - TXT_MARGIN_LEFT * 2;
}

// gets offset to begining of previous line
long flineback(FILE* fp, long maxBack) {
    TENTER
    long initalPos = ftell(fp);
    long curPos = initalPos;
    int newLineCount = 0;
    int c;

    while (curPos > 0) {
        fseek(fp, --curPos, SEEK_SET);
        if (initalPos - curPos > maxBack) { // can't go back
            fseek(fp, initalPos, SEEK_SET);
            return -1;
        }
        c = fgetc(fp);
        if (c == '\n') {
            newLineCount++;
            if (newLineCount == 2) { // first \n to begining of line, previous to begining of previous one
                curPos++;
                break;
            }
        }
    }
    if (newLineCount < 2) curPos = 0;
    fseek(fp, initalPos, SEEK_SET);

    return curPos;
}

static inline int16_t getLineHeight(Arduino_GFX* canvas) {
    int16_t x, y;
    uint16_t w, h;
    canvas->getTextBounds("Ay", 0, 0, &x, &y, &w, &h);
    return h;
}

TxtView::TxtView() {
    TENTER
}

bool TxtView::isFinished() {
    // TENTER
    // TODO: move to ABSTRACT WIDGET
    if (done) {
        done = false;
        return true;
    }
    return false;
}
// TODO: PSRAM VFS for other cases?
void TxtView::setTextFile(const String& fPath) {
    TENTER
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
    // TENTER
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
    // TENTER
    auto state = lilka::controller.getState();
    if (state.up.justPressed) scrollUp();
    else if (state.down.justPressed) scrollDown();
    else if (state.left.justPressed) scrollPageUp();
    else if (state.right.justPressed) scrollPageDown();
    else if (state.b.justPressed) done = true;
}

void TxtView::draw(Arduino_GFX* canvas) {
    // TENTER
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
    TENTER
    if (fp == NULL) {
        TXT_DBG lilka::serial.log("File not open. Skiping");
        return; // nothing to refresh. Wait till next update
    }

    // Read text block
    long curPos = ftell(fp);
    tLen = fread(tBlock, 1, TXT_MAX_BLOCK_SIZE, fp);
    fseek(fp, curPos, SEEK_SET); // save original file offset, could be useful for refresh
    tBlockRefreshRequired = false;

    TXT_DBG lilka::serial.log("Read %d bytes from %ld Position", tLen, curPos);
}

void TxtView::nOffsRefresh() {
    TENTER
    noffs.clear();
    if (!tLen) // Nothing read
        return;
    char* pCurrentChar = tBlock;
    char* pEndBlock = tBlock + tLen;
    noffs.push_back(pCurrentChar);
    // do not check last character here
    for (; pCurrentChar < pEndBlock; pCurrentChar++) {
        if (*pCurrentChar == '\n') {
            noffs.push_back(pCurrentChar + 1);
            // TXT_DBG lilka::serial.log("Adding noff at %p\n", noffs.back());
        }
    }
    TXT_DBG lilka::serial.log("Added %d noffs \n", noffs.size());
}

void TxtView::dOffsRefresh() {
    TENTER
    doffs.clear();
    // assume canvas options are already set
    if (!lastCanvas) {
        TXT_DBG lilka::serial.err("No access to lastCanvas, can't calc doffs");
        tBlockRefreshRequired = true; // reread then
        return;
    }
    // iterate by actual lines
    for (auto noff : noffs) {
        // now we've to move from noff till fits screen, or next noff
        char* pLineStart = noff;
        char* pLineEnd = uforward(pLineStart); // first letter
        while (pLineEnd != tBlock + tLen) { // fit bounds
            char backupChar = *pLineEnd;
            // Of course we can use temporary string for that, but why to mess
            // with a memory. Instead we could insert our null terminator where it's
            // needed to send in functions to measure, print what we need to restore it
            // backwards. Moving one byte is much easier than a whole stuff
            *pLineEnd = '\0';

            // reached newline
            if (*(pLineEnd - 1) == '\n') {
                doffs.push_back(pLineStart);
                // TXT_DBG lilka::serial.log("Adding doff at %p\n", doffs.back());
                // Restore old char
                *pLineEnd = backupChar;
                break; // swap to next noff
            }

            // Check if it fits
            if (!isLineWithinCanvas(pLineStart, lastCanvas)) {
                doffs.push_back(pLineStart); // push current, go next
                // Restore old char
                *pLineEnd = backupChar;
                pLineEnd = ubackward(pLineEnd); // go back one character
                // TXT_DBG lilka::serial.log(
                // "Adding doff at %pLine length = %d\n", doffs.back(), ulength(pLineStart, pLineEnd)
                // );
                pLineStart = pLineEnd;
                continue;
            }

            // Restore old char
            *pLineEnd = backupChar;
            // Go to next unicode character
            pLineEnd = uforward(pLineEnd);
        }
    }
}

void TxtView::scrollUp() {
    TENTER
    if (!fp || doffs.empty() || !lastCanvas) return;

    long currentFileOffset = ftell(fp);
    long prevNLineOffset = flineback(fp, TXT_MAX_BLOCK_SIZE);
    // Can't go back
    if (currentFileOffset == 0) return;
    fseek(fp, prevNLineOffset, SEEK_SET);
    // Do refreshes inplace
    tBlockRefresh();
    nOffsRefresh();
    dOffsRefresh();

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
    else return; // looks like we already here
    // TODO: lazy reading maybe?
    // should be where need at next refresh
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::scrollDown() {
    TENTER
    if (!fp) return;

    if (doffs.size() > 1) {
        fseek(fp, OFF2ROFF(doffs[1]), SEEK_SET);
    } else {
        fseek(fp, 0, SEEK_SET); // go begining
    }
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::scrollPageUp() {
    TENTER
    if (!fp) return;
    tBlockRefreshRequired = true; // to be done in update()
}

void TxtView::scrollPageDown() {
    TENTER
    if (!fp) return;
    tBlockRefreshRequired = true; // to be done in update()
}

TxtView::~TxtView() {
    TENTER
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