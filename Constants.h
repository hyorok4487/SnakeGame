#ifndef CONSTANTS_H
#define CONSTANTS_H

const int FIELD_WIDTH = 50;        // 전체 게임창 가로 크기
const int FIELD_HEIGHT = 21;       // 전체 게임창 세로 크기
const int FIELD_INNER_WIDTH = 48;  // 게임 내부 영역 가로 크기 (벽 제외)
const int FIELD_INNER_HEIGHT = 19; // 게임 내부 영역 세로 크기 (벽 제외)
const int INIT_SNAKE_LENGTH = 3;   // 초기 뱀 길이
const int TICK_RATE = 500;         // 게임 업데이트 주기 (ms 단위)
const int ITEM_DURATION = 5000;    // 아이템 지속 시간 (ms 단위)
const int MAX_STAGE = 4;           // 최대 스테이지 수

#endif
