#pragma once
#include <lilka.h>
#define TXT_MAX_BLOCK_SIZE 512
#define TXT_BUFFER_SIZE    64
// To be moved in lilka sdk

//

#ifdef TXT_VIEWER_DEBUG
#    define TXT_DBG if (1)
#else
#    define TXT_DBG if (0)
#endif

#define TXT_MARGIN_LEFT   38
#define STATUS_BAR_HEIGHT 30

#define OFF2ROFF(X)       X - tBlock + ftell(fp)

class TxtView {
public:
    TxtView();
    bool isFinished();
    void setTextFile(const String& fPath);
    void update();
    void draw(Arduino_GFX* canvas);

    // Display options
    void setColor(uint16_t color);
    void setBgColor(uint16_t bgColor);
    void setFont(const uint8_t* font);
    void setSpacing(uint16_t spacing); // distance between lines in pixels
    void setTextSize(uint8_t textSize); // actually is scale, but in GFX is named this way
    ~TxtView();

private:
    // Positioning and file read
    void tBlockRefresh(); // read text block, prepare noffs and doffs
    void nOffsRefresh(long maxoffset = -1);
    void dOffsRefresh(long maxoffset = -1);

    // Scrolling
    void scrollUp();
    void scrollDown();
    void scrollPageUp();
    void scrollPageDown();

    // Input
    void updateKeys();
    // Canvas setup
    void setCanvasOptions(Arduino_GFX* canvas);

    // File stuff
    FILE* fp = NULL;
    long tOffset = 0;

    // Text data
    char tBlock[TXT_MAX_BLOCK_SIZE];
    char tBuffer[TXT_BUFFER_SIZE]; // to be used by flineback()
    size_t tLen = 0; // text block length
    bool tBlockRefreshRequired = true; // inital value
    size_t lastDisplayedLines = 0;  
    std::vector<char*> noffs; // offsets to actual lines[separated by \n]
    std::vector<char*> doffs; // offsets to displayed lines

    // UI options
    bool done = false; // TODO: move to ABSTRACT WIDGET
    uint16_t color = lilka::colors::White;
    uint16_t bgColor = lilka::colors::Black;
    const uint8_t* font = FONT_8x13;
    uint16_t spacing = 1;
    uint8_t textSize = 1;

    // we need somehow calculate how much lines/characters fit display
    // so we store last canvas pointer to be acessible in dOffsRefresh
    Arduino_GFX* lastCanvas = NULL;
};
