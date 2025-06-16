#include "SnakeGame.h"
#include "Constants.h"
#include "Point.h"
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;

// 생성자: 초기 설정 및 ncurses 초기화
SnakeGame::SnakeGame() : growthItems(0), poisonItems(0), stage(1), gameState(RUNNING), gateUses(0) { 
    srand(time(0));  // 난수 초기화
    startTime = time(NULL);

    initscr();       // ncurses 화면 초기화
    cbreak();
    noecho();
    curs_set(0);     // 커서 숨김
    timeout(TICK_RATE);  // getch() 대기 시간 설정
    keypad(stdscr, TRUE); // 방향키 입력 가능 설정

    // 윈도우 생성: 게임창, 점수판, 미션판
    gameWin = newwin(FIELD_HEIGHT, FIELD_WIDTH, 0, 0);
    scoreWin = newwin(8, 25, 0, FIELD_WIDTH + 1);
    missionWin = newwin(8, 25, 9, FIELD_WIDTH + 1);

    // 색상 초기화 (성장 아이템 녹색, 독 아이템 빨간색)
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);

    init();  // 게임 초기화 함수 호출
}

// 벽에서 게이트 위치 2개 무작위 선택
void SnakeGame::placeGate() {
    vector<Point> candidates;
    // 면역 벽(immuneWalls) 제외한 벽 좌표 중 후보 선정
    for (auto& w : walls) {
        bool immune = false;
        for (auto& iw : immuneWalls) {
            if (w.x == iw.x && w.y == iw.y) {
                immune = true;
                break;
            }
        }
        if (!immune) candidates.push_back(w);
    }

    gates.clear();
    if (candidates.size() < 2) return;  // 후보가 2개 미만이면 게이트 생성 불가

    // 후보 중 서로 다른 두 위치 랜덤 선택
    int idx1 = rand() % candidates.size();
    int idx2;
    do {
        idx2 = rand() % candidates.size();
    } while (idx2 == idx1);

    gates.push_back(candidates[idx1]);
    gates.push_back(candidates[idx2]);
}

// 특정 좌표가 게이트 위치인지 검사
bool SnakeGame::isGate(Point p) {
    for (auto& g : gates) {
        if (g.x == p.x && g.y == p.y) return true;
    }
    return false;
}

// 입구 게이트 위치 기준 출구 게이트 위치 반환
Point SnakeGame::getGateExit(Point enterGate) {
    if (gates.size() < 2) return enterGate;
    if (enterGate.x == gates[0].x && enterGate.y == gates[0].y)
        return gates[1];
    else
        return gates[0];
}

// 방향의 반대 방향 반환
Direction SnakeGame::getOppositeDirection(Direction d) {
    switch (d) {
        case UP: return DOWN;
        case DOWN: return UP;
        case LEFT: return RIGHT;
        case RIGHT: return LEFT;
    }
    return UP; // 기본값(절대 오지 않음)
}

// 점수판 화면 그리기
void SnakeGame::drawScoreBoard() {
    wclear(scoreWin);
    box(scoreWin, 0, 0);
    mvwprintw(scoreWin, 0, 2, " Score Board ");

    mvwprintw(scoreWin, 1, 2, "B: %d / %d", (int)snake.size(), 100);  // 뱀 길이 / 최대 길이 100(임의)
    mvwprintw(scoreWin, 2, 2, "+: %d", growthItems);    // 성장 아이템 횟수
    mvwprintw(scoreWin, 3, 2, "-: %d", poisonItems);    // 독 아이템 횟수
    mvwprintw(scoreWin, 4, 2, "G: %d", gateUses);       // 게이트 사용 횟수

    // 경과 시간 출력
    time_t now = time(NULL);
    int elapsed = static_cast<int>(now - startTime);
    mvwprintw(scoreWin, 5, 2, "Time: %d sec", elapsed);

    wrefresh(scoreWin);
}

// 미션판 화면 그리기
void SnakeGame::drawMissionBoard() {
    wclear(missionWin);
    box(missionWin, 0, 0);
    mvwprintw(missionWin, 0, 2, " Mission ");

    // 미션 목표값 (임의 지정)
    int goalB = 10;
    int goalPlus = 5;
    int goalMinus = 2;
    int goalG = 3;

    // 현재 달성 여부 계산
    bool achievedB = (snake.size() >= goalB);
    bool achievedPlus = (growthItems >= goalPlus);
    bool achievedMinus = (poisonItems >= goalMinus);
    bool achievedG = (gateUses >= goalG);

    // 미션 목표 및 달성 여부 출력
    mvwprintw(missionWin, 1, 2, "B: %d (%s)", goalB, achievedB ? "True" : "False");
    mvwprintw(missionWin, 2, 2, "+: %d (%s)", goalPlus, achievedPlus ? "True" : "False");
    mvwprintw(missionWin, 3, 2, "-: %d (%s)", goalMinus, achievedMinus ? "True" : "False");
    mvwprintw(missionWin, 4, 2, "G: %d (%s)", goalG, achievedG ? "True" : "False");

    wrefresh(missionWin);
}

// 게임 화면 전체 그리기
void SnakeGame::draw() {
    wclear(gameWin);
    box(gameWin, 0, 0);

    // snake 그리기
    for (Point p : snake) {
        mvwprintw(gameWin, p.y, p.x, "O");
    }
    
    // wall 그리기
    for (Point p : walls) {
        mvwprintw(gameWin, p.y, p.x, "#");
    }
    
    // immune wall 그리기
    for (Point p : immuneWalls) {
        mvwprintw(gameWin, p.y, p.x, "@");
    }

    // gate 그리기
    wattron(gameWin, COLOR_PAIR(2));
    for (auto& g : gates) {
        mvwprintw(gameWin, g.y, g.x, "G");
    }
    wattroff(gameWin, COLOR_PAIR(2));

    // Items 그리기
    wattron(gameWin, COLOR_PAIR(1)); // [추가] Growth item 색상 적용
    mvwprintw(gameWin, growthItem.y, growthItem.x, "+");
    wattroff(gameWin, COLOR_PAIR(1));

    wattron(gameWin, COLOR_PAIR(2)); // [추가] Poison item 색상 적용
    mvwprintw(gameWin, poisonItem.y, poisonItem.x, "-");
    wattroff(gameWin, COLOR_PAIR(2));

    wrefresh(gameWin);

    // 점수판, 미션판 그리기
    drawScoreBoard();
    drawMissionBoard();

}

// 충돌 체크 (벽 또는 자기 몸통, 필드 밖 등)
bool SnakeGame::isCollision(Point p) {
    // 필드 영역 밖 체크
    if (p.x <= 0 || p.x >= FIELD_WIDTH - 1 || p.y <= 0 || p.y >= FIELD_HEIGHT - 1) {
        return true;
    }
    // 뱀 몸통 충돌 체크
    if (isSnake(p)) return true;
    
    // 벽 충돌 체크
    for (Point w : walls) {
        if (w.x == p.x && w.y == p.y) return true;
    }
    return false;
}

// 뱀 몸통 좌표 체크
bool SnakeGame::isSnake(Point p) {
    for (Point s : snake) {
        if (s.x == p.x && s.y == p.y) return true;
    }
    return false;
}

// 미션 달성 여부 판단 (모든 조건 달성 시 true)
bool SnakeGame::missionAchieved() {
    return (snake.size() >= 10 &&
            growthItems >= 5 &&
            poisonItems >= 2 &&
            gateUses >= 3);
}

// 아이템 위치 랜덤 생성 (빈 공간에 생성)
void SnakeGame::generateItem() {
    do{
        growthItem = Point(rand() % FIELD_INNER_WIDTH + 1, rand() % FIELD_INNER_HEIGHT + 1);
        poisonItem = Point(rand() % FIELD_INNER_WIDTH + 1, rand() % FIELD_INNER_HEIGHT + 1);
        
    }while(isSnake(growthItem) || isSnake(poisonItem) || isCollision(growthItem) || isCollision(poisonItem) || (poisonItem.x == growthItem.x && poisonItem.y == growthItem.y));

    itemSpawnTime = time(NULL);
}

void SnakeGame::initStageWalls(int stage) { // 스테이지별 벽 설정 함수
    walls.clear();
    immuneWalls.clear();
    gates.clear();

    int centerX = FIELD_WIDTH / 2;
    int centerY = FIELD_HEIGHT / 2;

    for (int i = 1; i < FIELD_WIDTH; ++i) {
        walls.push_back(Point(i, 0));
        walls.push_back(Point(i, FIELD_HEIGHT - 1));
    }
    for (int i = 1; i < FIELD_HEIGHT; ++i) {
        walls.push_back(Point(0, i));
        walls.push_back(Point(FIELD_WIDTH - 1, i));
    }

    switch(stage) {
        case 1: // 기존 코드의 맵 (가장자리 벽 + 중앙 십자형)
            for(int i = 1; i <= 5; ++i){
                walls.push_back(Point(centerX, centerY+i));
                walls.push_back(Point(centerX, centerY-i));
            }
            for(int i = 1; i <= 10; ++i){
                walls.push_back(Point(centerX+i, centerY));
                walls.push_back(Point(centerX-i, centerY));
            }

            immuneWalls.push_back(Point(centerX, centerY));
            break;

        case 2: // 사각형 모양 내벽 추가 (중앙 십자 제외)
            // 사각형 내벽 생성 (예: FIELD_INNER 영역 테두리)
            for (int i = 10; i < FIELD_WIDTH - 10; ++i) {
                walls.push_back(Point(i, 5));
                walls.push_back(Point(i, FIELD_HEIGHT - 6));
            }
            for (int i = 5; i < FIELD_HEIGHT - 5; ++i) {
                walls.push_back(Point(10, i));
                walls.push_back(Point(FIELD_WIDTH - 11, i));
            }

            immuneWalls.push_back(Point(10, 5));
            immuneWalls.push_back(Point(FIELD_WIDTH - 11, 5));
            immuneWalls.push_back(Point(10, FIELD_HEIGHT - 6));
            immuneWalls.push_back(Point(FIELD_WIDTH - 11, FIELD_HEIGHT - 6));
            break;

        case 3: // X자 모양 내벽 추가
            for (int i = 1; i < FIELD_WIDTH - 1; ++i) {
                immuneWalls.push_back(Point(i, i * FIELD_HEIGHT / FIELD_WIDTH)); // 대각선 1
                immuneWalls.push_back(Point(i, FIELD_HEIGHT - 1 - (i * FIELD_HEIGHT / FIELD_WIDTH))); // 대각선 2
            }

            break;

        case 4: // 복잡한 맵 (예시로 테두리 + 2개의 작은 사각형)
            // 작은 사각형 1
            for (int i = 15; i < 25; ++i) {
                walls.push_back(Point(i, 6));
                walls.push_back(Point(i, 10));
            }
            for (int i = 6; i <= 10; ++i) {
                walls.push_back(Point(15, i));
                walls.push_back(Point(24, i));
            }
            // 작은 사각형 2
            for (int i = 30; i < 40; ++i) {
                walls.push_back(Point(i, 13));
                walls.push_back(Point(i, 17));
            }
            for (int i = 13; i <= 17; ++i) {
                walls.push_back(Point(30, i));
                walls.push_back(Point(39, i));
            }

            immuneWalls.push_back(Point(15, 6));
            immuneWalls.push_back(Point(24, 6));
            immuneWalls.push_back(Point(15, 10));
            immuneWalls.push_back(Point(24, 10));
            immuneWalls.push_back(Point(30, 13));
            immuneWalls.push_back(Point(39, 13));
            immuneWalls.push_back(Point(30, 17));
            immuneWalls.push_back(Point(39, 17));
            break;
        
        default:
            break;
    }

    immuneWalls.push_back(Point(0, 0));  // 좌측 상단
    immuneWalls.push_back(Point(FIELD_WIDTH - 1, 0));  // 우측 상단
    immuneWalls.push_back(Point(0, FIELD_HEIGHT - 1));  // 좌측 하단
    immuneWalls.push_back(Point(FIELD_WIDTH - 1, FIELD_HEIGHT - 1));  // 우측 하단

    // 게이트는 벽 생성 후에 호출해야 함
    placeGate(); // 벽 생성 후 placeGate() 호출
    
}

// 게임 초기화
void SnakeGame::init() {
    snake.clear();
    direction = UP;
    growthItems = 0;
    poisonItems = 0;
    gateUses = 0;
    gameState = RUNNING;

    lastGateChangeTime = time(NULL);

    // 초기 위치 고정 또는 랜덤 조정 (기존처럼 랜덤 유지)
    int startX = rand() % (FIELD_WIDTH - 4) + 2;
    int startY = rand() % (FIELD_HEIGHT - 4) + 2;

    snake.push_back(Point(startX, startY));
    snake.push_back(Point(startX, startY + 1));
    snake.push_back(Point(startX, startY + 2));
    direction = UP;

    // 스테이지별 벽 초기화 호출
    initStageWalls(stage);

    generateItem();
}

// 뱀 움직임 처리 및 상태 업데이트
void SnakeGame::update() {
    // 일정 시간마다 게이트 위치 변경
    time_t now = time(NULL);
    if (now - lastGateChangeTime >= 5) {
        placeGate();
        lastGateChangeTime = now;
    }

    int ch = getch();
    Direction newDirection = direction; // 새로운 방향을 저장할 변수

    // 사용자 입력에 따른 방향 변경
    switch (ch) {
    case KEY_UP:
        newDirection = UP;
        break;
    case KEY_DOWN:
        newDirection = DOWN;
        break;
    case KEY_LEFT:
        newDirection = LEFT;
        break;
    case KEY_RIGHT:
        newDirection = RIGHT;
        break;
    }

    // 새로운 방향이 현재 방향과 반대되지 않을 때만 적용
    if (newDirection == direction){
        direction = direction;
    }else{
        direction = newDirection;
    }

    // 뱀의 머리 위치
    Point newHead = snake[0];
    switch (direction) {
    case UP:
        newHead.y--;
        break;
    case DOWN:
        newHead.y++;
        break;
    case LEFT:
        newHead.x--;
        break;
    case RIGHT:
        newHead.x++;
        break;
    }

    // 게이트 진입 처리 시작
    if (isGate(newHead)) {
        gateUses++;
        // 출구 게이트 위치 구함
        Point exitGate = getGateExit(newHead);

        // 진입 방향은 현재 방향 반대
        Direction enterDir = getOppositeDirection(direction);

        // 진출 방향 우선순위 (enterDir부터 시계 방향으로)
        Direction dirCandidates[4];
        dirCandidates[0] = enterDir;
        dirCandidates[1] = static_cast<Direction>((enterDir + 1) % 4);
        dirCandidates[2] = static_cast<Direction>((enterDir + 2) % 4);
        dirCandidates[3] = static_cast<Direction>((enterDir + 3) % 4);

        Direction exitDir = dirCandidates[0];
        Point tryPos;

        // 우선순위에 따라 이동 가능한 위치 찾기
        for (int i = 0; i < 4; i++) {
            exitDir = dirCandidates[i];
            tryPos = exitGate;
            switch (exitDir) {
                case UP: tryPos.y -= 1; break;
                case DOWN: tryPos.y += 1; break;
                case LEFT: tryPos.x -= 1; break;
                case RIGHT: tryPos.x += 1; break;
            }
            if (!isCollision(tryPos)) {
                break;
            }
        }

        if (isCollision(tryPos)) {
            gameState = GAME_OVER;
            showGameOver();
            return;
        }

        // 새 머리 위치를 게이트 출구 + 진출 방향 한 칸으로 변경
        newHead = tryPos;
        direction = exitDir;

        // 충돌 여부 확인
        if (isCollision(newHead)) {
            gameState = GAME_OVER;
            showGameOver();
            return;
        }
    }

    // 충돌 여부 확인
    if (isCollision(newHead)) {
        gameState = GAME_OVER;
        showGameOver();
        return;
    }

    // 머리가 성장 아이템 위치에 도달했을 때 처리
    if (newHead.x == growthItem.x && newHead.y == growthItem.y) {
        growthItems++;
        snake.push_back(snake.back());
        generateItem();
    } 
    // 머리가 독 아이템 위치에 도달했을 때 처리
    else if (newHead.x == poisonItem.x && newHead.y == poisonItem.y) {
        poisonItems++;
        if (snake.size() > 3) {
            snake.pop_back();
        } else {
            // 최소 길이 이하로 줄어들면 게임오버
            gameState = GAME_OVER;
            showGameOver();
            return;
        }
        generateItem();
    } 

    // 뱀의 몸통 이동
    for (int i = snake.size() - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = newHead;

    // 아이템 지속 시간이 만료되었는지 확인하여 새로 생성
    removeItemIfExpired();

    // 미션 달성 시 처리
    if (missionAchieved()) {
        if (stage < MAX_STAGE) {
            stage++;
            init();  // 다음 스테이지 초기화
        } else {
            gameState = CONGRATULATION;
        }
    }
}

void SnakeGame::removeItemIfExpired() {
    time_t now = time(NULL);

    if (now - itemSpawnTime >= 5) {  // 5초 지났으면 아이템 위치 재생성
        generateItem();
    }
}

// 게임 종료 메시지 출력
void SnakeGame::showGameOver() {
    wclear(gameWin);
    mvwprintw(gameWin, FIELD_HEIGHT / 2, (FIELD_WIDTH - 10) / 2, "Game Over");
    wrefresh(gameWin);
    napms(2000); // Show "Game Over" for 2 seconds
    endwin();
}

// 축하 메시지 출력
void SnakeGame::showCongratulation() {
    wclear(gameWin);
    mvwprintw(gameWin, FIELD_HEIGHT / 2, (FIELD_WIDTH - 10) / 2, "CONGRATULATION!");
    wrefresh(gameWin);
    napms(3000); // Show "CONGRATULATION!" for 3 seconds
    endwin();
}

// 게임 메인 루프
void SnakeGame::run() {
    while (gameState == RUNNING) {
        draw();
        update();
    }

    if (gameState == GAME_OVER) {
        showGameOver();
    } else if (gameState == CONGRATULATION) {
        showCongratulation();
    }

    delwin(gameWin);
    delwin(scoreWin);
    delwin(missionWin);
    endwin();
}
