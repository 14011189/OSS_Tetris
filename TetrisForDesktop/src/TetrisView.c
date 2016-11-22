#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include "TetrisView.h"
#include "TetrisManager.h"
#include "Constant.h"
#include "Util.h"
#include "Menu.h"

#pragma comment(lib, "winmm.lib")//소리 및 음악 재생을 위해서 winmm.lib라이브러리를 로드

#define TETRIS_BACKGROUND_MUSIC_FILE_NAME "..\\res\\tetris_background_music.wav"//테트리스 배경 음악 파일명을 상수화(상대 경로 포함)

#define PROCESS_REACHED_CASE_COUNT 2 
#define ADD_ID_POSITION_X_TO_PRINT 25  //RNAK에 추가할 때, rank ID적는 창의 x좌표
#define ADD_ID_POSITION_Y_TO_PRINT 10 //RNAK에 추가할 때, rank ID적는 창의 y좌표
#define SETTING_POSITION_X_TO_PRINT 30  //셋팅에 들어갔을 때 뜨는 창의 x좌표
#define SETTING_POSITION_Y_TO_PRINT 10  //셋팅에 들어갔을 때 뜨는 창의 y좌표

enum MenuStartPosition{
	MAIN_MENU_X = 18, MAIN_MENU_Y = 7, //메인 메뉴 시작 위치 MAIN_MENU_X를 감소시키면 왼쪽으로, MAIN_MENU_Y 감소시키면 위로
	PAUSE_MENU_X = 5, PAUSE_MENU_Y = 12, //게임 실행 중 ESC키를 눌렀을 경우 나타나는 창 
	END_MENU_X = 14, END_MENU_Y = 8 //게임이 END했을 떄 뜨는 창 위치
};

// 게임 시작에 필요한 작업
void TetrisView_StartGame(TetrisView* tetrisView){
	PlaySound(TEXT(TETRIS_BACKGROUND_MUSIC_FILE_NAME), NULL, SND_ASYNC | SND_LOOP);//파일명(리소스명)과 각종 플래그들을 점겨줌
	 //SND_FILENAME:파일명, SND_ASYNC:비기동적으로 재생,SMD_LOOP:무한 재생	-> |(OR)로 연결해서 매개 변수로 전달				
	if (!(tetrisView->level >= MIN_SPEED_LEVEL && tetrisView->level <= MAX_SPEED_LEVEL))//level이 최소 스피드 레벨보다 크지 않거나, 최대 스피드 레벨보다 작지 않으면
	{
		tetrisView->level = MIN_SPEED_LEVEL; //level이 임의로 최소 레벨부터 시작
	}
	TetrisManager_Init(&tetrisView->tetrisManager, tetrisView->level); //변수 초기화
	system("cls");//실행화면 지우기
	FontUtil_ChangeFontColor(DEFAULT_FONT_COLOR); //색 변경
	TetrisManager_PrintBoard(&tetrisView->tetrisManager);//테트리스가 움직일 수 있는 창
	TetrisManager_PrintDetailInfomation(&tetrisView->tetrisManager);//있으면 시간,next1,next2 화면에 표시
}

//입력된 키를 통해 게임 진행
//process type(사용자가 어떤 키를 사용하였는지), 방향등을 전달 -> 실제 테트리스 게임을 진행
void TetrisView_ProcessGame(TetrisView* tetrisView, int processType, int direction){
	//it is used to move left or right at bottom in case of space which you want to move is available

	static int processReachedCaseCount = 0;
	if (processType == DIRECTION){//방향키를 누른 경우
		TetrisManager_ProcessDirection(&tetrisView->tetrisManager, direction);//블럭 방향을 업데이트 -> 블럭을 회전
	}
	else if (processType == DIRECT_DOWN){//스페이스 바를 누른 경우
		TetrisManager_ProcessDirectDown(&tetrisView->tetrisManager);//현재 블럭이 내려갈 수 있을때 까지 내리고
	}
	else if (processType == AUTO){//자동으로 내려오는 경우
		TetrisManager_ProcessAuto(&tetrisView->tetrisManager);//한 칸 밑으로 블럭을 내린 뒤 화면을 갱신
	}
	if (TetrisManager_IsReachedToBottom(&tetrisView->tetrisManager, MOVING_BLOCK)){//현재 블럭이 바닥에 닿았는지 체크
		if (processType == DIRECT_DOWN){
			processReachedCaseCount = 0;
			if (TetrisManager_ProcessReachedCase(&tetrisView->tetrisManager) == END){//현재 블럭이 바닥에 닿은 경우 처리
				TetrisView_EndGame(tetrisView);                                      //반환값을 확인하여 END인 경우, endgame을 호출 -> 게임 종료
				return;
			}
		}
		else{//!processType=DIRCT_DOWN
			//if you are going to move the block which has bottom wall or bottom fixed block, permit the block to move the direction
			if (processReachedCaseCount == PROCESS_REACHED_CASE_COUNT){
				if (TetrisManager_ProcessReachedCase(&tetrisView->tetrisManager) == END){
					TetrisView_EndGame(tetrisView);
					return;
				}
				processReachedCaseCount = 0;
			}
			else{
				processReachedCaseCount++;
			}
		}
	}
	TetrisManager_ProcessDeletingLines(&tetrisView->tetrisManager); // 굳어진 블럭이 한줄을 이뤘을 경우를 처리
}

//게임 멈췄을 경우
void TetrisView_PauseGame(TetrisView* tetrisView){
	TetrisManager_PauseTotalTime(&tetrisView->tetrisManager); 
	PlaySound(NULL, 0, 0);//노래 종료
	FontUtil_ChangeFontColor(LIGHT_YELLOW);//ESC눌렀을 경우 뜨는 창 폰트 색 노란생으로
	TetrisView_ProcessPauseMenu(tetrisView);//pauseMenu 보여줌
	FontUtil_ChangeFontColor(DEFAULT_FONT_COLOR); 
	switch (tetrisView->pauseMenu){//pauseMenu인경우
	case RESUME_PAUSE_MENU: //Resume을 선택 했을 경우
		TetrisManager_StartTotalTime(&tetrisView->tetrisManager);
		PlaySound(TEXT(TETRIS_BACKGROUND_MUSIC_FILE_NAME), NULL, SND_ASYNC | SND_LOOP);//노래 시작
		break;
	case MAIN_MENU_PAUSE_MENU://Main menu를 선택 했을 경우
		TetrisManager_StopTotalTime(&tetrisView->tetrisManager);
		break;
	}
}

void TetrisView_MissionGame(TetrisView* tetrisView){

}
//게임 종료
void TetrisView_EndGame(TetrisView* tetrisView){//endGame의 정의
	TetrisManager_PauseTotalTime(&tetrisView->tetrisManager); //게임 진행했을 때 카운트하던 전체 시간 멈춤
	PlaySound(NULL, 0, 0);//현재 재생되고있는 소리를 멈춤
	TetrisView_ProcessEndMenu(tetrisView);
}

//rank 보여주기
void TetrisView_ShowRanking(TetrisView* tetrisView){

	RankingManager_Create(&tetrisView->rankingManager);
	RankingManager_Load(&tetrisView->rankingManager);
	RankingManager_Print(&tetrisView->rankingManager);
	RankingManager_Destroy(&tetrisView->rankingManager);
}

//rank에 추가
void TetrisView_AddRanking(TetrisView* tetrisView){

	Ranking ranking;

	int x = ADD_ID_POSITION_X_TO_PRINT;
	int y = ADD_ID_POSITION_Y_TO_PRINT;
	int i;
	char id[ID_SIZE + 1] = { '\0', };
	system("cls");//화면에 있던 그림 전부 지움

	//ranking 창,화면 
	CursorUtil_GotoXY(x, y++);
	printf("┏━━━━━━━━━━━━━┓");
	CursorUtil_GotoXY(x, y++);
	printf("┃Input ID (%d chars limit!)┃", ID_SIZE);
	CursorUtil_GotoXY(x, y++);
	printf("┃:                         ┃");
	CursorUtil_GotoXY(x, y++);
	printf("┗━━━━━━━━━━━━━┛");

	x += 4;
	y -= 2;
	CursorUtil_GotoXY(x, y++);//ID입력하는 커서 위치

	CursorUtil_Show();//커서 보임
	fgets(id, ID_SIZE + 1, stdin);//커서 크기 
	CursorUtil_Hide();
	for (i = ID_SIZE; i >= 0; i--){
		if (id[i] == '\n'){
			id[i] = '\0';
			break;
		}
	}
	if (i == -1){
		while (getchar() != '\n');
	}
	strcpy(ranking.id, id);//strcpy(복사받을 변수, 복사할 변수)
	ranking.score = tetrisView->tetrisManager.score;
	ranking.level = tetrisView->tetrisManager.speedLevel;
	ranking.deletedLineCount = tetrisView->tetrisManager.deletedLineCount;
	ranking.totalTime = tetrisView->tetrisManager.totalTime;
	ranking.timestamp = time(NULL);
	RankingManager_Create(&tetrisView->rankingManager);
	RankingManager_Load(&tetrisView->rankingManager);
	RankingManager_Add(&tetrisView->rankingManager, ranking);
	RankingManager_Save(&tetrisView->rankingManager);
	RankingManager_Destroy(&tetrisView->rankingManager);
}

//레벨 정하는 메뉴
void TetrisView_ShowSetting(TetrisView* tetrisView){

	int x = SETTING_POSITION_X_TO_PRINT;
	int y = SETTING_POSITION_Y_TO_PRINT;
	if (!(tetrisView->level >= MIN_SPEED_LEVEL && tetrisView->level <= MAX_SPEED_LEVEL)){//레벨이 최소 속도보다 작거나 최대 속도보다 작으면
		tetrisView->level = MIN_SPEED_LEVEL; //level 
	}
	system("cls");
	CursorUtil_GotoXY(x, y++);
	printf("┏━━━━━━━━━┓");
	CursorUtil_GotoXY(x, y++);
	printf("┃Current Level : %2d┃", tetrisView->level);
	CursorUtil_GotoXY(x, y++);
	printf("┃New Level (%d ~ %d)┃", MIN_SPEED_LEVEL, MAX_SPEED_LEVEL);
	CursorUtil_GotoXY(x, y++);
	printf("┃:                 ┃");
	CursorUtil_GotoXY(x, y++);
	printf("┗━━━━━━━━━┛");
	x += 4;
	y -= 2;
	CursorUtil_GotoXY(x, y++);
	CursorUtil_Show();
	scanf("%d", &tetrisView->level);
	CursorUtil_Hide();
	if (tetrisView->level >= MIN_SPEED_LEVEL && tetrisView->level <= MAX_SPEED_LEVEL){

	}
	else if (tetrisView->level < MIN_SPEED_LEVEL){
		tetrisView->level = MIN_SPEED_LEVEL;
	}
	else if (tetrisView->level > MAX_SPEED_LEVEL){
		tetrisView->level = MAX_SPEED_LEVEL;
	}
	else{
		tetrisView->level = MIN_SPEED_LEVEL;
	}
	while (getchar() != '\n');
}

//메인 메뉴 창
void TetrisView_ProcessMainMenu(TetrisView* tetrisView){
	Menu menu = { 0, };
	char items[MENU_ITEMS_COUNT][MENU_ITEMS_CONTENT_SIZE] = {
		"[1] S T A R T", "[2] R A N K I N G", "[3] S E T T I N G","[4] M I S S I O N" ,"[5] E X I T"
	};
	int menuCount = 5;
	int x = MAIN_MENU_X;
	int y = MAIN_MENU_Y;
	WindowUtil_ChangeWindowSize(WINDOW_LINE_SIZE, WINDOW_COL_SIZE);
	system("cls");
	FontUtil_ChangeFontColor(RED);
	CursorUtil_GotoXY(x, y++);
	printf("■■■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■");
	x += 8;
	y -= 5;
	FontUtil_ChangeFontColor(LIGHT_RED);
	CursorUtil_GotoXY(x, y++);
	printf("■■■");
	CursorUtil_GotoXY(x, y++);
	printf("■");
	CursorUtil_GotoXY(x, y++);
	printf("■■■");
	CursorUtil_GotoXY(x, y++);
	printf("■");
	CursorUtil_GotoXY(x, y++);
	printf("■■■");
	x += 8;
	y -= 5;
	FontUtil_ChangeFontColor(LIGHT_YELLOW);
	CursorUtil_GotoXY(x, y++);
	printf("■■■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■");
	x += 8;
	y -= 5;
	FontUtil_ChangeFontColor(GREEN);
	CursorUtil_GotoXY(x, y++);
	printf("■■");
	CursorUtil_GotoXY(x, y++);
	printf("■ ■");
	CursorUtil_GotoXY(x, y++);
	printf("■■");
	CursorUtil_GotoXY(x, y++);
	printf("■ ■");
	CursorUtil_GotoXY(x, y++);
	printf("■  ■");
	x += 8;
	y -= 5;
	FontUtil_ChangeFontColor(LIGHT_BLUE);
	CursorUtil_GotoXY(x, y++);
	printf("■");
	CursorUtil_GotoXY(x, y++);
	printf("■");
	CursorUtil_GotoXY(x, y++);
	printf("■");
	CursorUtil_GotoXY(x, y++);
	printf("■");
	CursorUtil_GotoXY(x, y++);
	printf("■");
	x += 4;
	y -= 5;
	FontUtil_ChangeFontColor(LIGHT_PURPLE);
	CursorUtil_GotoXY(x, y++);
	printf(" ■■");
	CursorUtil_GotoXY(x, y++);
	printf("■");
	CursorUtil_GotoXY(x, y++);
	printf("■■■");
	CursorUtil_GotoXY(x, y++);
	printf("    ■");
	CursorUtil_GotoXY(x, y++);
	printf(" ■■");
	x -= 22;
	y += 2;
	Menu_Create(&menu, items, menuCount, x, y, DEFAULT_FONT_COLOR); //default된 색으로 메뉴를 만들고
	Menu_Print(&menu);//메뉴 출력
	tetrisView->mainMenu = Menu_ProcessKey(&menu) + 1;
	tetrisView->pauseMenu = 0;
	tetrisView->endMenu = 0;
}

void TetrisView_ProcessPauseMenu(TetrisView* tetrisView){
	Menu menu = { 0, };
	char items[MENU_ITEMS_COUNT][MENU_ITEMS_CONTENT_SIZE] = {
		"[1] R E S U M E", "[2] M A I N M E N U"
	};
	int menuCount = 2;
	int x = PAUSE_MENU_X;
	int y = PAUSE_MENU_Y;
	Menu_Create(&menu, items, menuCount, x, y, LIGHT_YELLOW);
	Menu_Print(&menu);
	tetrisView->pauseMenu = Menu_ProcessKey(&menu) + 1;
	tetrisView->mainMenu = 0;
	tetrisView->endMenu = 0;
	tetrisView->missionMenu = 0;
	if (tetrisView->pauseMenu == RESUME_PAUSE_MENU){
		TetrisManager_PrintBoard(&tetrisView->tetrisManager);
	}
}

void TetrisView_ProcessEndMenu(TetrisView* tetrisView){
	Menu menu = { 0, };
	char items[MENU_ITEMS_COUNT][MENU_ITEMS_CONTENT_SIZE] = {
		"[1] R A N K I N G", "[2] M A I N M E N U", "[3] E X I T"
	};
	int menuCount = 3;
	int x = END_MENU_X;
	int y = END_MENU_Y;
	system("cls");
	FontUtil_ChangeFontColor(RED);
	CursorUtil_GotoXY(x, y++);
	printf("■■■  ■  ■  ■■■          ■■■  ■  ■  ■■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■    ■  ■  ■              ■       ■ ■  ■ ■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■    ■■■  ■■■  ■■■  ■■■  ■■■  ■  ■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■    ■  ■  ■              ■      ■ ■   ■ ■");
	CursorUtil_GotoXY(x, y++);
	printf("  ■    ■  ■  ■■■          ■■■  ■  ■  ■■");
	x += 19;
	y += 2;
	Menu_Create(&menu, items, menuCount, x, y, DEFAULT_FONT_COLOR);
	Menu_Print(&menu);
	tetrisView->endMenu = Menu_ProcessKey(&menu) + 1;
	tetrisView->mainMenu = 0;
	tetrisView->pauseMenu = 0;
}

//미션 탭을 선택했을 때 뜨는 화면
void TetrisView_ProcessMisionMenu(TetrisView* tetrisView)
{
	Menu menu = { 0, };
	char items[MENU_ITEMS_COUNT][MENU_ITEMS_CONTENT_SIZE] = {
		"[1] S T A R T","[2] E X I T"
	};
	int menuCount = 3;
	int x = END_MENU_X - 1;
	int y = END_MENU_Y - 1;
	system("cls");
	FontUtil_ChangeFontColor(LIGHT_YELLOW);
	CursorUtil_GotoXY(x, y++);
	printf(" ■  ■   ■   ■■■   ■■■     ■    ■   ■    ■");
	CursorUtil_GotoXY(x, y++);
	printf("■ ■ ■  ■  ■       ■        ■  ■  ■  ■ ■  ■");
	CursorUtil_GotoXY(x, y++);
	printf("■ ■ ■  ■   ■■■   ■■■  ■    ■ ■  ■ ■  ■");
	CursorUtil_GotoXY(x, y++);
	printf("■    ■  ■        ■       ■  ■  ■  ■  ■  ■ ■");
	CursorUtil_GotoXY(x, y++);
	printf("■    ■  ■   ■■■   ■■■     ■    ■  ■    ■");
	x += 19;
	y += 2;
	Menu_Create(&menu, items, menuCount, x, y, DEFAULT_FONT_COLOR);
	Menu_Print(&menu);
	tetrisView->missionMenu = Menu_ProcessKey(&menu) + 1;
	tetrisView->mainMenu = 0;
	tetrisView->pauseMenu = 0;
	
}

// TetrisManager_GetDownMilliSecond 함수를 호출하여 얻은 반환값을 다시 반환
DWORD TetrisView_GetDownMilliSecond(TetrisView* tetrisView){
	return TetrisManager_GetDownMilliSecond(&tetrisView->tetrisManager);
	//TetrisManager_GetDownMilliSecond: 게임에서 현재 블럭을 얼마만큼 지연 시킨 후 밑으로 이동시켜야하는지를 밀리세컨드로 게산하여 반환
}

void TetrisView_MakeHold(TetrisView* tetrisView){
	TetrisManager_MakeHold(&tetrisView->tetrisManager);
}
