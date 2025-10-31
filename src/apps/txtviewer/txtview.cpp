#include "txtview.h"
#include <errno.h>

// move to next Unicode character
const char *uforward(const char *cstr) {
    if (!cstr || !*cstr) return cstr;
    cstr++;
    while ((*cstr & 0xC0) == 0x80) cstr++; // skip continuation bytes
    return cstr;
}

// move to beginning of previous Unicode character
const char *ubackward(const char *cstr) {
    if (!cstr) return cstr;
    const char *p = cstr - 1;
    while ((*(unsigned char*)p & 0xC0) == 0x80) --p; // skip continuation bytes
    return p;
}

// gets offset to begining of previous line
long flineback(FILE *fp, long maxBack){
    long initalPos = ftell(fp); 
    long curPos = initalPos;
    int newLineCount = 0;
    int c;

    while (curPos > 0){
        fseek(fp, --curPos, SEEK_SET);
        if (initalPos - curPos > maxBack){ // can't go back
            fseek(fp, initalPos, SEEK_SET);        
            return -1;
        }
        c = fgetc(fp);
        if (c == '\n'){
            newLineCount++;
            if (newLineCount == 2){  // first \n to begining of line, previous to begining of previous one
                curPos++;
                break;
            }
        }

    }
    if (newLineCount < 2) curPos = 0;
    fseek(fp, initalPos, SEEK_SET);

    return curPos;
}

TxtView::TxtView() {

}

bool TxtView::isFinished(){
    // TODO: move to ABSTRACT WIDGET
    if (done) {
        done = false;
        return true;
    }
    return false;

}
// TODO: PSRAM VFS for other cases?
void TxtView::setTextFile(const String &fPath){
    if (fp){ // close old file
        fclose(fp);
        fp = NULL;
    }
    fp = fopen(fPath.c_str(), "r");
    if (fp == NULL){
        lilka::serial.err("Tried to open file %s. Errno %d", fPath.c_str(), errno);
    }
    tBlockRefreshRequired = true;   // to be done in update()
}

void TxtView::update(){
    updateKeys();
    if (tBlockRefreshRequired){
        tBlockRefresh();
        nOffsRefresh();
        dOffsRefresh();        
    }
}

void TxtView::updateKeys(){
    auto state = lilka::controller.getState();
    if (state.up.justPressed)
        scrollUp();
    else if (state.down.justPressed)
        scrollDown();
    else if (state.left.justPressed)
        scrollPageUp();
    else if (state.right.justPressed)
        scrollPageDown();
    else if (state.b.justPressed)
        done = true;
}

void TxtView::draw(Arduino_GFX *canvas){
    // display doffs

    // sleep
}

void TxtView::tBlockRefresh(){
    if (fp == NULL)
        return; // nothing to refresh. Wait till next update

    // Read text block
    fread(tBlock, sizeof(tBlock), 1, fp);
    tBlockRefreshRequired = false;
}
void TxtView::nOffsRefresh(){

}
void TxtView::dOffsRefresh(){
    // TODO: invent a good way to store canvas options
}

void TxtView::scrollUp(){
    tBlockRefreshRequired = true;   // to be done in update()
}
void TxtView::scrollDown(){
    tBlockRefreshRequired = true;   // to be done in update()
}
void TxtView::scrollPageUp(){
    tBlockRefreshRequired = true;   // to be done in update()
}
void TxtView::scrollPageDown(){
    tBlockRefreshRequired = true;   // to be done in update()
}
TxtView::~TxtView(){
    // Never forget to close file
    if (fp)
        fclose(fp);
}