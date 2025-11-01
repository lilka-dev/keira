#pragma once
#include <lilka.h>
#define TXT_MAX_BLOCK_SIZE 512
// To be moved in lilka sdk

//
#define TXT_VIEWER_DEBUG

#ifdef TXT_VIEWER_DEBUG
#    define TXT_DBG if (1)
#else
#    define TXT_DBG if (0)
#endif
// TODO: Move to keira/macro sdk?
#define TENTER            TXT_DBG lilka::serial.log("==> %s:%d %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);
#define TEND              TXT_DBG lilka::serial.log("<== %s:%d %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);

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
    ~TxtView();

private:
    // Positioning and file read
    void tBlockRefresh(); // read text block, prepare noffs and doffs
    void nOffsRefresh();
    void dOffsRefresh();

    // Scrolling
    void scrollUp();
    void scrollDown();
    void scrollPageUp();
    void scrollPageDown();

    // Input
    void updateKeys();

    // Display options
    void setColor(uint16_t color);
    void setBgColor(uint16_t bgColor);
    void setFont(const uint8_t* font);

    // Canvas setup
    void setCanvasOptions(Arduino_GFX* canvas);

    // File stuff
    FILE* fp = NULL;
    long tOffset = 0;

    // Text data
    char tBlock[TXT_MAX_BLOCK_SIZE];
    size_t tLen = 0; // text block length
    bool tBlockRefreshRequired = true; // inital value

    std::vector<char*> noffs; // offsets to actual lines[separated by \n]
    std::vector<char*> doffs; // offsets to displayed lines

    // UI options
    bool done = false; // TODO: move to ABSTRACT WIDGET
    uint16_t color = lilka::colors::White;
    uint16_t bgColor = lilka::colors::Black;
    const uint8_t* font = FONT_9x15;

    // we need somehow calculate how much lines/characters fit display
    // so we store last canvas pointer to be acessible in dOffsRefresh
    Arduino_GFX* lastCanvas = NULL;
};
