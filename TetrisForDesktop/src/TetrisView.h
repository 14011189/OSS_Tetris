#ifndef _TETRIS_VIEW_H
#define _TETRIS_VIEW_H

#include "TetrisManager.h"
#include "RankingManager.h"

typedef struct _tetrisView{//TetrisView 구조체의 정의부
	TetrisManager tetrisManager;
	RankingManager rankingManager;
	int level;
	int mainMenu;
	int pauseMenu;
	int missionMenu;
	int endMenu;
}TetrisView;

void TetrisView_StartGame(TetrisView* tetrisView);
void TetrisView_ProcessGame(TetrisView* tetrisView, int processType, int direction);
void TetrisView_PauseGame(TetrisView* tetrisView);
void TetrisView_EndGame(TetrisView* tetrisView);
void TetrisView_ShowRanking(TetrisView* tetrisView);
void TetrisView_AddRanking(TetrisView* tetrisView);
void TetrisView_ShowSetting(TetrisView* tetrisView);
void TetrisView_ProcessMainMenu(TetrisView* tetrisView);
void TetrisView_ProcessPauseMenu(TetrisView* tetrisView);
void TetrisView_ProcessMisionMenu(TetrisView* tetrisView);//Mission을 선택하고 난 다음에 뜨는 화면에 관련된 함수
void TetrisView_ProcessEndMenu(TetrisView* tetrisView);

//make temporarily for calling from main function
DWORD TetrisView_GetDownMilliSecond(TetrisView* tetrisView);
void TetrisView_MakeHold(TetrisView* tetrisView);

#endif
