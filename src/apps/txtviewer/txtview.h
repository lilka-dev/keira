#pragma once
#include "abstractwidget.h"

#define TXT_MAX_BLOCK_SIZE 1024
#define TXT_BUFFER_SIZE    64
// To be moved in lilka sdk

//

#ifdef TXT_VIEWER_DEBUG
#    define TXT_DBG if (1)
#else
#    define TXT_DBG if (0)
#endif

#define TXT_MARGIN_LEFT   28 // as well as right :D
#define TXT_MARGIN_BOTTOM 25 // margin from the bottom, actually maybe name it toolbar

// tBlockAddrToOffset
#define TADDR2OFF(X)       X - tBlock + ftell(fp)

typedef enum {TXT_FORWARD, TXT_BACKWARD} TxtScrollDirection;

// To be moved in K_S_STRINGS, but cause expected movage to sdk, 've no idea
#define TXT_S_EMPTY       "== EMPTY =="
#define TXT_S_ENC_UNKNOWN "== UNKNOWN ENCODING =="

typedef enum { TXT_UNKNOWN, TXT_EMPTY, TXT_UTF8, TXT_BIN, TXT_LEGACY } FileEncoding;
typedef void (*PTXTViewCallback)(void*);
class TxtView : public AbstractWidget {
public:
    TxtView();
    void setTextFile(const String& fPath);
    void update() override;
    void draw(Arduino_GFX* canvas) override;

    // Display options
    void setColor(uint16_t color);
    void setBackgroundColor(uint16_t bgColor);
    void setFont(const uint8_t* font);
    void setSpacing(uint16_t spacing); // distance between lines in pixels
    void setTextSize(uint8_t textSize); // actually is scale, but in GFX is named this way

    // Misc options
    void jumpToOffset(long offset); // jump to nearest offset
    long getFileSize();
    long getOffset();
    // Callback setup. To be triggered on any activation button
    void setCallback(PTXTViewCallback clbk, void* clbkData);

    ~TxtView();

private:
    // Positioning and file read
    void tBlockRefresh(); // read text block, prepare noffs and doffs
    void nPtrRefresh(long maxoffset = -1);
    void dPtrRefresh(long maxoffset = -1);

    // Scrolling
    void scrollUp(size_t linesToScroll = 1);
    void scrollDown();
    void scrollPageUp();
    void scrollPageDown();

    // Input
    void updateButtons();
    // Canvas setup
    void setCanvasOptions(Arduino_GFX* canvas);

    // File stuff
    FILE* fp = NULL;
    // long tOffset = 0; unused
    long fSize = 0;

    // Text data
    char tBlock[TXT_MAX_BLOCK_SIZE] = {};
    char tBuffer[TXT_BUFFER_SIZE] = {}; // to be used by flineback()
    size_t tLen = 0; // text block length
    bool tBlockRefreshRequired = true; // inital value
    size_t lastDisplayedLines = 0;
    size_t maxLines = 0;
    std::vector<char*> nptrs; // ptrs to \n separated lines withing tBlock
    std::vector<char*> dptrs; // ptrs to displayed lines withing tBlock
    FileEncoding encoding = TXT_UNKNOWN;

    // UI options
    uint16_t color = lilka::colors::White;
    uint16_t bgColor = lilka::colors::Black;
    const uint8_t* font = FONT_8x13;
    uint16_t spacing = 1;
    uint8_t textSize = 1;

    // we need somehow calculate how much lines/characters fit display
    // so we store last canvas pointer to be acessible in dOffsRefresh
    Arduino_GFX* lastCanvas = NULL;

    // Callback
    PTXTViewCallback clbk = nullptr;
    void* clbkData = nullptr;
};
