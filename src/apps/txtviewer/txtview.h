#pragma once
#include <lilka.h>
#define TXT_MAX_BLOCK_SIZE 2048
// To be moved in lilka sdk

// 


class TxtView{
public:
    TxtView();
    bool isFinished();
    void setTextFile(const String &fPath);    
    void update();
    void draw(Arduino_GFX* canvas);
    ~TxtView();
private:
    void tBlockRefresh();   // read text block, prepare noffs and doffs
    void nOffsRefresh();
    void dOffsRefresh();

    void scrollUp();
    void scrollDown();
    void scrollPageUp();
    void scrollPageDown();
    
    void updateKeys();

    FILE *fp = NULL;
    long tOffset = 0;
    char tBlock[TXT_MAX_BLOCK_SIZE];
    bool tBlockRefreshRequired = true; // inital value
    std::vector <char *> noffs;  // offsets to actual lines
    std::vector <char *> doffs;  // offsets to displayed lines

    bool done = false;          // TODO: move to ABSTRACT WIDGET
};
