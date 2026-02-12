/** ColecoVision Emulator     *************************************/
/**                                                              **/
/** Author: Jean-Marc Harvengt                                   **/
/** Project repo: https://github.com/Jean-MarcHarvengt/espMCUME/ **/
/**                                                              **/
/******************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void coc_Init(void);
int coc_Start(char * CartName);
void coc_Step(void);
void coc_Stop(void);

#ifdef __cplusplus
}
#endif
