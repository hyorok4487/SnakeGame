#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include <vector>
#include <ctime>
#include <ncurses.h>
#include "Point.h"
#include "Constants.h"

enum Direction { UP, DOWN, LEFT, RIGHT };
enum GameState { RUNNING, GAME_OVER, CONGRATULATION };

class SnakeGame {
public:
    SnakeGame();
    void run();

private:
    time_t startTime;      // 게임 시작 시간

    WINDOW *gameWin;       // 메인 게임 화면
    WINDOW *scoreWin;      // 점수판 창
    WINDOW *missionWin;    // 미션판 창

    std::vector<Point> gates;   // 게이트 위치 2개 저장
    std::vector<Point> snake;   // 뱀 몸통 좌표 저장
    Direction direction;   // 현재 이동 방향
    int growthItems;       // 성장 아이템 먹은 개수
    int poisonItems;       // 독 아이템 먹은 개수
    int gateUses;          // 게이트 사용 횟수
    
    Point growthItem;      // 성장 아이템 위치
    Point poisonItem;      // 독 아이템 위치

    GameState gameState;   // 게임 상태 (RUNNING, CONGRATULATION, GAME_OVER)
    int stage;             // 현재 스테이지 번호 (1 ~ 4)

    time_t itemSpawnTime;       // 마지막 아이템 생성 시간
    time_t lastGateChangeTime;  // 마지막 게이트 위치 변경 시간
    
    std::vector<Point> walls;       // 현재 스테이지 벽 좌표
    std::vector<Point> immuneWalls; // 면역 벽 좌표 (게이트 생성 후보에서 제외)

    void init();                        // 게임 초기화 함수
    void initStageWalls(int stage);    // 스테이지별 벽 초기화 함수
    
    void draw();                       // 전체 화면 그리기
    void drawScoreBoard();             // 점수판 그리기
    void drawMissionBoard();           // 미션판 그리기
    void update();                     // 게임 상태 업데이트
    void generateItem();               // 아이템 위치 재생성
    void removeItemIfExpired();        // 아이템 지속시간 만료 시 제거 후 재생성

    bool isCollision(Point);           // 충돌 체크 (벽, 뱀 몸통, 테두리)
    bool isSnake(Point);               // 뱀 몸통 좌표와 일치 여부 체크
    bool isGate(Point);                // 게이트 좌표와 일치 여부 체크

    void placeGate();                  // 게이트 위치 무작위 설정
    Point getGateExit(Point);          // 입구 게이트 기준 출구 게이트 위치 반환
    Direction getOppositeDirection(Direction);  // 방향 반대값 반환

    void showGameOver();               // 게임오버 메시지 출력
    void showCongratulation();         // 축하 메시지 출력

    bool missionAchieved();            // 미션 달성 여부 판단
};

#endif
