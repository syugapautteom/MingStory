// 컴파일러 제공 외부 헤더파일
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

// 최대 한도량
#define MAX_NAME 50         // 최대 이름 글자 수 50바이트
#define MAX_SKILLS 50       // 최대 스킬 레벨 50레벨
#define MAX_LEVEL 200       // 캐릭터 최대 레벨 200레벨

// 캐릭터 최대 획득 경험치량 2147483647 = 200레벨
#define MAX_EXPERIENCE 2147483647

// 레벨 당 주어지는 스킬, 스탯 포인트 수
#define STAT_POINTS_PER_LEVEL 15
#define SKILL_POINTS_PER_LEVEL 5

// 경험치 관련 테이블 (최대 200레벨, 200+1)
int expTable[201];

// 직업 관련 구조체
typedef enum {
    JOB_NONE,               // 초보자 직업군
    JOB_WARRIOR,            // 전사 직업군
    JOB_MAGICIAN,           // 마법사 직업군
    JOB_THIEF,              // 도적 직업군
    JOB_ARCHER              // 궁수 직업군
} JobType;

// 스킬 특성 구조체
typedef enum {
    SKILL_ATTACK,           // 공격 스킬
    SKILL_DEFENSE,          // 방어 스킬
    SKILL_PASSIVE           // 패시브 스킬
} SkillType;

// 스킬 총합 구조체
typedef struct {
    char name[50];          // 스킬 이름
    SkillType type;         // 스킬 특성 (공격, 방어, 패시브)
    int tier;               // 전직 스킬 1~4차
    int power;              // 능력치(공격력) 수치
    char description[100];  // 스킬 설명
    int level;              // 스킬 레벨
} Skill;

// Player 정보 처리용 구조체
typedef struct {
    char name[MAX_NAME];        // 플레이어 이름
    int level;                  // 플레이어 레벨
    int exp;                    // 플레이어 경험치량
    int hp;                     // 플레이어 현재 체력량
    int mp;                     // 플레이어 현재 마나량
    int str;                    // 플레이어 힘(STR) 스탯
    int dex;                    // 플레이어 재능(DEX) 스탯
    int luk;                    // 플레이어 운(LUX) 스탯
    int intl;                   // 플레이어 민첩(INT) 스택
    JobType job;                // 플레이어 직업
    int jobTier;                // 플레이어 직업 티어 (초보자 포함 1~4차)
    int statPoints;             // 사용 가능한 스탯 포인트 수
    int skillPoints;            // 사용 가능한 스킬 포인트 수
    Skill skills[MAX_SKILLS];
    int skillCount;
    int maxHp;                  // 플레이어 최대 체력량
    int maxMp;                  // 플레이어 최대 마나량
} Player;

// 초보자 스킬
Skill beginnerSkills[] = {
    // 스킬 이름       스킬 특성    전직, 공격력       스킬 설명          스킬 레벨(필수 아님)
    {"달팽이 세마리", SKILL_ATTACK, 0, 10, "적을 향해 달팽이 껍질을 던진다."}
};

// 전사 스킬
Skill warriorSkills[] = {
    {"파워 스트라이크", SKILL_ATTACK, 1, 150, "거친 힘으로 한번 공격"},
    {"브랜디쉬", SKILL_ATTACK, 1, 200, "두번 공격하는 파워 스트라이크"},
    {"아이언 윌", SKILL_DEFENSE, 1, 5, "방어력 +5 (1턴)"},
    {"체력 회복", SKILL_DEFENSE, 1, 20, "HP +20"},
    {"체력 증가", SKILL_PASSIVE, 1, 20, "Max HP +20"},
    {"분노", SKILL_PASSIVE, 1, 2, "공격력 +2"},

    {"더블 슬래시", SKILL_ATTACK, 2, 170, "두배 더 강력한 파워로 2~4번 공격"},
    {"그라운드 슬램", SKILL_ATTACK, 2, 120, "주변의 모든 적 공격"},
    {"분노의 외침", SKILL_DEFENSE, 2, 150, "2턴 동안 공격력 증가"},
    {"전장의 회복", SKILL_DEFENSE, 2, 30, "HP +30"},
    {"강인함", SKILL_PASSIVE, 2, 30, "HP +30"},
    {"맹공격", SKILL_PASSIVE, 2, 10, "기본공격 데미지 10%% 증가"},

    {"블러디 블레이드", SKILL_ATTACK, 3, 300, "HP 흡수형 공격 (공격력 180)"},
    {"소닉 크래시", SKILL_ATTACK, 3, 70, "3회 연타, 70 데미지"},
    {"절대 방어", SKILL_DEFENSE, 3, 50, "2턴간 받는 데미지 50%% 감소"},
    {"분신", SKILL_DEFENSE, 3, 0, "1턴간 회피율 증가"},
    {"타고난 근성", SKILL_PASSIVE, 3, 0, "전투 후 HP 자동 회복"},
    {"전장의 전사", SKILL_PASSIVE, 3, 10, "데미지 10%% 증가"},

    {"콤보 어썰트", SKILL_ATTACK, 4, 400, "2.5배 공격 + 치명타 확률 증가"},
    {"히어로 블래스트", SKILL_ATTACK, 4, 180, "전체 적 강타"},
    {"히어로의 외침", SKILL_DEFENSE, 4, 0, "공/방 모두 증가 (2턴)"},
    {"불굴의 투지", SKILL_DEFENSE, 4, 0, "HP 1 남기고 생존 (1회)"},
    {"궁극의 체력", SKILL_PASSIVE, 4, 100, "HP +100"},
    {"무쌍", SKILL_PASSIVE, 4, 15, "크리티컬 확률 증가"}
};

// 마법사 스킬
Skill magicianSkills[] = {
    {"매직 클로", SKILL_ATTACK, 1, 130, "단일 공격, 데미지 130"},
    {"썬더볼트", SKILL_ATTACK, 1, 100, "전체 적 마법 공격"},
    {"매직 쉴드", SKILL_DEFENSE, 1, 50, "받는 데미지 50%% 감소 (1턴)"},
    {"마나 회복", SKILL_DEFENSE, 1, 15, "MP +15"},
    {"정신 집중", SKILL_PASSIVE, 1, 20, "MP 최대치 +20"},
    {"마력 증가", SKILL_PASSIVE, 1, 2, "INT +2"},

    {"파이어 애로우", SKILL_ATTACK, 2, 180, "불 속성 강타"},
    {"아이스 스톰", SKILL_ATTACK, 2, 120, "빙결 확률 있는 전체 공격"},
    {"에너지 실드", SKILL_DEFENSE, 2, 60, "받는 데미지 60%% 감소"},
    {"힐링", SKILL_DEFENSE, 2, 40, "HP +40"},
    {"지혜의 오라", SKILL_PASSIVE, 2, 30, "INT +3"},
    {"마나 맥스", SKILL_PASSIVE, 2, 30, "MP +30"},

    {"홀리 크래시", SKILL_ATTACK, 3, 220, "강력한 단일 성속성 마법"},
    {"체인 라이트닝", SKILL_ATTACK, 3, 160, "연쇄 번개 공격"},
    {"매지컬 프로텍션", SKILL_DEFENSE, 3, 0, "마법 데미지 무효화 (1회)"},
    {"마력 방패", SKILL_DEFENSE, 3, 80, "받는 데미지 80%% 감소"},
    {"성스러운 지혜", SKILL_PASSIVE, 3, 40, "INT +4"},
    {"마법 증폭", SKILL_PASSIVE, 3, 20, "마법 데미지 20%% 증가"},

    {"메테오", SKILL_ATTACK, 4, 450, "모든 적에게 큰 피해"},
    {"제네시스", SKILL_ATTACK, 4, 420, "성속성 폭발 공격"},
    {"신의 가호", SKILL_DEFENSE, 4, 0, "모든 데미지 무효화 (1턴)"},
    {"마력 전이", SKILL_DEFENSE, 4, 100, "MP +100, HP +50"},
    {"궁극의 마력", SKILL_PASSIVE, 4, 100, "INT +10"},
    {"무한 마법", SKILL_PASSIVE, 4, 50, "스킬 MP 소비 50%% 감소"}
};

// 도적 스킬
Skill thiefSkills[] = {
    {"더블 스탭", SKILL_ATTACK, 1, 75, "공격력 75로 2회 공격"},
    {"어쌔신 블레이드", SKILL_ATTACK, 1, 160, "치명타 확률 증가"},
    {"연막탄", SKILL_DEFENSE, 1, 0, "회피율 증가 (1턴)"},
    {"속도 향상", SKILL_DEFENSE, 1, 0, "선공 확정 (1턴)"},
    {"민첩성 향상", SKILL_PASSIVE, 1, 2, "DEX +2"},
    {"은신", SKILL_PASSIVE, 1, 0, "전투 시작시 첫 데미지 무효"},

    {"쉐도우 클로", SKILL_ATTACK, 2, 180, "어둠 속성 단일 강타"},
    {"트리플 히트", SKILL_ATTACK, 2, 65, "3회 타격"},
    {"스모크 봄", SKILL_DEFENSE, 2, 0, "회피율 증가 (2턴)"},
    {"독 회피", SKILL_DEFENSE, 2, 0, "상태이상 무효화 (1회)"},
    {"고양된 감각", SKILL_PASSIVE, 2, 3, "LUK +3"},
    {"암살자 본능", SKILL_PASSIVE, 2, 15, "크리티컬 데미지 증가"},

    {"쉐도우 대거", SKILL_ATTACK, 3, 210, "은신 상태 시 추가 데미지"},
    {"섀도우 댄스", SKILL_ATTACK, 3, 100, "랜덤 연속 공격"},
    {"연막 강화", SKILL_DEFENSE, 3, 0, "회피 + 방어 동시 증가"},
    {"블링크", SKILL_DEFENSE, 3, 0, "턴 중간 회피 가능"},
    {"기민함", SKILL_PASSIVE, 3, 3, "DEX/LUK +3"},
    {"완전 은신", SKILL_PASSIVE, 3, 0, "일정 확률로 적이 공격 못함"},

    {"섀도우 킬", SKILL_ATTACK, 4, 400, "치명타 확정 + 회피 가능"},
    {"나이프 스톰", SKILL_ATTACK, 4, 300, "범위 공격, 출혈 효과"},
    {"연막 폭풍", SKILL_DEFENSE, 4, 0, "회피율 + 회복 (2턴)"},
    {"맹독 강화", SKILL_DEFENSE, 4, 0, "지속 데미지 면역"},
    {"그림자 군단", SKILL_PASSIVE, 4, 0, "치명타 확률 + 회피 증가"},
    {"암살 전용 체질", SKILL_PASSIVE, 4, 100, "HP +100, DEX +5"}
};

// 궁수 스킬
Skill archerSkills[] = {
    {"스트레이프", SKILL_ATTACK, 1, 160, "데미지 160 두 발 사격"},
    {"애로우 레인", SKILL_ATTACK, 1, 70, "3연속 공격"},
    {"포커스", SKILL_DEFENSE, 1, 0, "명중률 증가 (1턴)"},
    {"체력 집중", SKILL_DEFENSE, 1, 10, "HP 회복 + 회피율 증가"},
    {"스나이핑 자세", SKILL_PASSIVE, 1, 10, "크리티컬 데미지 증가"},
    {"집중력 향상", SKILL_PASSIVE, 1, 5, "스킬 데미지 5%% 증가"},

    {"멀티 샷", SKILL_ATTACK, 2, 90, "최대 6마리 적에게 화살 연사"},
    {"더블 애로우", SKILL_ATTACK, 2, 180, "2연속 강타"},
    {"숲의 가호", SKILL_DEFENSE, 2, 30, "HP +30, 방어력 증가"},
    {"하늘의 눈", SKILL_DEFENSE, 2, 0, "회피 무시 (1턴)"},
    {"매의 시야", SKILL_PASSIVE, 2, 2, "DEX +2"},
    {"빠른 손놀림", SKILL_PASSIVE, 2, 15, "공격속도 증가"},

    {"피어싱 샷", SKILL_ATTACK, 3, 210, "방어 무시 단일 관통 공격"},
    {"화살 폭풍", SKILL_ATTACK, 3, 110, "전체 공격, 출혈 확률"},
    {"민첩 향상", SKILL_DEFENSE, 3, 0, "회피율 + 공격속도 증가"},
    {"사냥의 기도", SKILL_DEFENSE, 3, 0, "명중률 + HP 회복 (1턴)"},
    {"명사수", SKILL_PASSIVE, 3, 10, "명중률 + 치명타 확률 증가"},
    {"야생의 감각", SKILL_PASSIVE, 3, 0, "모든 능력 소폭 증가"},

    {"궁극의 샷", SKILL_ATTACK, 4, 400, "단일 강타, 치명타 확정"},
    {"에로우 커튼", SKILL_ATTACK, 4, 280, "지속 공격 효과"},
    {"정밀 사격", SKILL_DEFENSE, 4, 0, "다음 공격 크리티컬 확정"},
    {"하이퍼 포커스", SKILL_DEFENSE, 4, 0, "명중률 + 회피율 상승 (2턴)"},
    {"궁극의 집중", SKILL_PASSIVE, 4, 15, "DEX +5, 크리티컬 대미지 증가"},
    {"신궁의 혼", SKILL_PASSIVE, 4, 0, "궁수 전용 모든 능력치 +2"}
};

// 플레이어 데이터를 파일에 저장하는 함수
bool SavePlayerData(const Player* player) {
    // Try open for read/write, if not exist, create new
    FILE* file = fopen("player.data", "r+");
    if (!file) file = fopen("player.data", "w+");
    if (!file) return 0;

    // 저장된 파일 불러오기
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buf = malloc(filesize + 4096); // allow extra for growth
    if (!buf) {
        fclose(file);
        return 0;
    }
    size_t readsz = fread(buf, 1, filesize, file);
    buf[readsz] = 0;

    // 파일 내부 데이터 저장 형태
    char newblock[4096];
    int offset = 0;
    offset += snprintf(newblock + offset, sizeof(newblock) - offset,
        "data {\n"
        "  name: %s\n"
        "  level: %d\n"
        "  exp: %d\n"
        "  hp: %d\n"
        "  mp: %d\n"
        "  str: %d\n"
        "  dex: %d\n"
        "  luk: %d\n"
        "  intl: %d\n"
        "  job: %d\n"
        "  jobTier: %d\n"
        "  statPoints: %d\n"
        "  skillPoints: %d\n"
        "  maxHp: %d\n"
        "  maxMp: %d\n",
        player->name, player->level, player->exp, player->hp, player->mp,
        player->str, player->dex, player->luk, player->intl, player->job,
        player->jobTier,
        player->statPoints, player->skillPoints,
        player->maxHp, player->maxMp);
    
    // 플레이어가 현재 보유중인 스킬 저장 (스킬명, 스킬 레벨)
    offset += snprintf(newblock + offset, sizeof(newblock) - offset,
        "  skillCount: %d\n", player->skillCount);
    for (int i = 0; i < player->skillCount; i++) {
        offset += snprintf(newblock + offset, sizeof(newblock) - offset,
            "  skill%d: %s|%d|%d|%d|%s|%d\n", i,
            player->skills[i].name,
            player->skills[i].type,
            player->skills[i].tier,
            player->skills[i].power,
            player->skills[i].description,
            player->skills[i].level);
    }
    offset += snprintf(newblock + offset, sizeof(newblock) - offset, "}\n");

    // 불러온 데이터 매칭 확인
    char* p = buf;
    char* match_start = NULL;
    char* match_end = NULL;
    while ((p = strstr(p, "data {"))) {
        char* block_start = p;
        char* block_end = strstr(block_start, "}\n");
        if (!block_end) break;
        block_end += 2; // after "}\n"
        // Find name field in block
        char* name_line = strstr(block_start, "name:");
        if (name_line && name_line < block_end) {
            char nameval[MAX_NAME+2] = {0};
            const char* ptr = strchr(name_line, ':');
            if (ptr) {
                ptr++;
                while (*ptr == ' ') ptr++;
                sscanf(ptr, "%49[^\n]", nameval);
                if (strcmp(nameval, player->name) == 0) {
                    match_start = block_start;
                    match_end = block_end;
                    break;
                }
            }
        }
        p = block_end;
    }

    // Compose new file content
    FILE* out = file;
    fseek(file, 0, SEEK_SET);
    if (match_start && match_end) {
        // Overwrite matching block
        size_t before = match_start - buf;
        size_t after = readsz - (match_end - buf);
        fwrite(buf, 1, before, out);
        fwrite(newblock, 1, strlen(newblock), out);
        fwrite(match_end, 1, after, out);
        // Truncate file if new content is shorter
        long newlen = before + strlen(newblock) + after;
        fflush(out);
#ifdef _WIN32
        _chsize(_fileno(out), newlen);          // Windows 전용
#else
        ftruncate(fileno(out), newlen);         // Linux, macOS 전용
#endif
    } else {
        // Append new block
        fseek(file, 0, SEEK_END);
        fwrite(newblock, 1, strlen(newblock), out);
    }
    free(buf);
    fclose(file);
    return 1;
}

// 플레이어 데이터를 파일에서 불러오는 함수
bool LoadPlayerData(Player* player, const char* filename, const char* inputName) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "data {", 6) == 0) {
            // Read until name field inside block
            while (fgets(line, sizeof(line), file)) {
                if (strstr(line, "name:")) {
                    char nameInFile[MAX_NAME];
                    char* ptr = strchr(line, ':');
                    if (ptr) {
                        ptr++; // Move past ':'
                        while (*ptr == ' ') ptr++; // Skip spaces
                        sscanf(ptr, "%49[^\n]", nameInFile);
                        if (strcmp(nameInFile, inputName) == 0) {
                            found = 1;
                            strcpy(player->name, nameInFile);
                            // Read block until closing brace
                            while (fgets(line, sizeof(line), file)) {
                                if (strstr(line, "level:")) sscanf(line, " level: %d", &player->level);
                                else if (strstr(line, "exp:")) sscanf(line, " exp: %d", &player->exp);
                                else if (strstr(line, "hp:")) sscanf(line, " hp: %d", &player->hp);
                                else if (strstr(line, "mp:")) sscanf(line, " mp: %d", &player->mp);
                                else if (strstr(line, "str:")) sscanf(line, " str: %d", &player->str);
                                else if (strstr(line, "dex:")) sscanf(line, " dex: %d", &player->dex);
                                else if (strstr(line, "luk:")) sscanf(line, " luk: %d", &player->luk);
                                else if (strstr(line, "intl:")) sscanf(line, " intl: %d", &player->intl);
                                else if (strstr(line, "job:")) sscanf(line, " job: %d", (int*)&player->job);
                                else if (strstr(line, "jobTier:")) sscanf(line, " jobTier: %d", &player->jobTier);
                                else if (strstr(line, "statPoints:")) sscanf(line, " statPoints: %d", &player->statPoints);
                                else if (strstr(line, "skillPoints:")) sscanf(line, " skillPoints: %d", &player->skillPoints);
                                else if (strstr(line, "maxHp:")) sscanf(line, " maxHp: %d", &player->maxHp);
                                else if (strstr(line, "maxMp:")) sscanf(line, " maxMp: %d", &player->maxMp);
                                else if (strstr(line, "skillCount:")) sscanf(line, " skillCount: %d", &player->skillCount);
                                else if (strstr(line, "skill")) {
                                    Skill s;
                                    int slot;
                                    char name[50] = {0}, desc[100] = {0};
                                    int type = 0, tier = 0, power = 0, level = 0;
                                    // Parse slot number
                                    // The line format: " skill%d: ..."
                                    // We'll use strtok for parsing as requested
                                    char* token = strtok(line, ":");
                                    token = strtok(NULL, ":"); // Skip to actual data
                                    if (token) {
                                        // First, parse slot from the original line (before strtok)
                                        sscanf(line, " skill%d", &slot);
                                        token = strtok(token, "|");
                                        if (token) strncpy(name, token, sizeof(name));

                                        token = strtok(NULL, "|");
                                        if (token) type = atoi(token);

                                        token = strtok(NULL, "|");
                                        if (token) tier = atoi(token);

                                        token = strtok(NULL, "|");
                                        if (token) power = atoi(token);

                                        token = strtok(NULL, "|");
                                        if (token) strncpy(desc, token, sizeof(desc));

                                        token = strtok(NULL, "|");
                                        if (token) level = atoi(token);

                                        strncpy(s.name, name, sizeof(s.name)); s.name[sizeof(s.name)-1] = '\0';
                                        s.type = type;
                                        s.tier = tier;
                                        s.power = power;
                                        strncpy(s.description, desc, sizeof(s.description)); s.description[sizeof(s.description)-1] = '\0';
                                        s.level = level;

                                        if (slot >= 0 && slot < MAX_SKILLS) {
                                            player->skills[slot] = s;
                                        }
                                    }
                                }
                                else if (strstr(line, "}")) break;
                            }
                            break;
                        }
                    }
                }
                if (strstr(line, "}")) break;
            }
            if (found) break;
        }
    }

    fclose(file);
    return found;
}

// 플레이어의 현재 정보를 출력하는 함수
void DisplayPlayerInfo(const Player* player) {
    printf("=== 캐릭터 정보 ===\n");
    printf("이름: %s\n", player->name);
    printf("레벨: %d\n", player->level);
    printf("경험치: %d\n", player->exp);
    printf("HP: %d / %d, MP: %d / %d\n", player->hp, player->maxHp, player->mp, player->maxMp);
    printf("STR: %d, DEX: %d, LUK: %d, INT: %d\n", player->str, player->dex, player->luk, player->intl);
    printf("남은 스탯포인트: %d, 스킬포인트: %d\n", player->statPoints, player->skillPoints);
    
    // 직업별 티어에 따라 직업명 표시 (티어 기반)
    const char* jobTitle = "초보자";
    int highestTier = player->jobTier;
    switch (player->job) {
        case JOB_WARRIOR:
            if (highestTier == 1) jobTitle = "검사";
            else if (highestTier == 2) jobTitle = "파이터";
            else if (highestTier == 3) jobTitle = "크루세이더";
            else if (highestTier == 4) jobTitle = "히어로";
            else jobTitle = "전사";
            break;
        case JOB_MAGICIAN:
            if (highestTier == 1) jobTitle = "매지션";
            else if (highestTier == 2) jobTitle = "클레릭";
            else if (highestTier == 3) jobTitle = "프리스트";
            else if (highestTier == 4) jobTitle = "비숍";
            else jobTitle = "마법사";
            break;
        case JOB_THIEF:
            if (highestTier == 1) jobTitle = "로그";
            else if (highestTier == 2) jobTitle = "시프";
            else if (highestTier == 3) jobTitle = "시프마스터";
            else if (highestTier == 4) jobTitle = "섀도어";
            else jobTitle = "도적";
            break;
        case JOB_ARCHER:
            if (highestTier == 1) jobTitle = "아처";
            else if (highestTier == 2) jobTitle = "헌터";
            else if (highestTier == 3) jobTitle = "레인저";
            else if (highestTier == 4) jobTitle = "보우마스터";
            else jobTitle = "궁수";
            break;
        default:
            jobTitle = "초보자";
            break;
    }
    printf("직업: %s\n", jobTitle);
    int statAttack = (player->str + player->dex + player->luk + player->intl) / 4;
    int skillAttackBonus = 0;
    for (int i = 0; i < player->skillCount; i++)
    {
        if (player->skills[i].type == SKILL_ATTACK)
        {
            skillAttackBonus += player->skills[i].power * player->skills[i].level;
        }
    }
    int combatPower = statAttack + skillAttackBonus;

    printf("스탯 기반 공격력: %d\n", statAttack);
    printf("전투력: %d\n", combatPower);
    printf("===================\n\n");
}

// 스탯을 직업에 따라 자동으로 분배하는 함수
void AutoDistributeStats(Player* player) {
    if (player->statPoints <= 0) {
        printf("남은 스탯 포인트가 없습니다.\n");
        return;
    }

    int* stats[] = {&player->str, &player->dex, &player->luk, &player->intl};
    int mainStat = 0;
    int subStat = 1;

    switch (player->job) {
        case JOB_WARRIOR: mainStat = 0; subStat = 1; break;     // STR, DEX
        case JOB_MAGICIAN: mainStat = 3; subStat = 2; break;    // INT, LUX
        case JOB_THIEF: mainStat = 1; subStat = 2; break;       // DEX, LUX
        case JOB_ARCHER: mainStat = 1; subStat = 2; break;      // DEX, LUX
        default:
            // 초보자일 경우 4개의 스탯 모두 균등 분배
            for (int i = 0; i < player->statPoints; i++) {
                stats[i % 4][0]++;
            }
            player->statPoints = 0;
            return;
    }

    for (int i = 0; i < player->statPoints; i++) {
        if (i % 5 < 4)
            (*stats[mainStat])++;   // 5point 중 4point = main stats에 투자
        else
            (*stats[subStat])++;    // 5point 중 1point = sub stats에 투자
    }
    player->statPoints = 0;
}

// 플레이어가 직접 스탯을 분배하는 함수
void ManualDistributeStats(Player* player) {
    printf("현재 능력치:\n");
    printf("STR: %d, DEX: %d, LUK: %d, INT: %d\n", player->str, player->dex, player->luk, player->intl);
    while (player->statPoints > 0) {
        printf("남은 스탯포인트: %d\n", player->statPoints);
        printf("1. STR\n2. DEX\n3. LUK\n4. INT\n선택: ");
        int choice;
        scanf("%d", &choice);
        switch (choice) {
            case 1: player->str++; break;
            case 2: player->dex++; break;
            case 3: player->luk++; break;
            case 4: player->intl++; break;
            default: printf("잘못된 입력!\n"); continue;
        }
        player->statPoints--;
    }
}

// 플레이어에게 해당 티어의 직업 스킬을 추가하는 함수
void AddJobSkills(Player* player, Skill* jobSkills, int skillCount, int tier) {
    bool exists;
    for (int i = 0; i < skillCount; i++) {
        if (jobSkills[i].tier == tier) {
            exists = 0;
            for (int j = 0; j < player->skillCount; j++) {
                if (strcmp(player->skills[j].name, jobSkills[i].name) == 0) {
                    exists = 1;
                    break;
                }
            }
            if (!exists && player->skillCount < MAX_SKILLS) {
                player->skills[player->skillCount++] = jobSkills[i];
            }
        }
    }
}

// 플레이어에게 스킬포인트를 지급하는 함수
void GainSkillPoint(Player* p) {
    p->skillPoints += SKILL_POINTS_PER_LEVEL;
}

// 플레이어가 보유한 스킬을 강화하는 함수
void UpgradeSkill(Player* p) {
    if (p->skillCount == 0) {
        printf("아직 습득한 스킬이 없습니다!\n");
        return;
    }
    // 업그레이드 전 최소 스탯 레벨 1 이상인지 확인
    for (int i = 0; i < p->skillCount; i++) {
        if (p->skills[i].level <= 0) {
            p->skills[i].level = 1;
        }
    }
    for (int i = 0; i < p->skillCount; i++) {
        // (This for-loop is only for context, the next while-loop is the upgrade logic)
    }
    while (1)
    {
        printf("남은 스킬포인트: %d\n", p->skillPoints);
        for (int i = 0; i < p->skillCount; i++)
        {
            printf("%d. %s (Lv %d): %s\n", i + 1, p->skills[i].name, p->skills[i].level, p->skills[i].description);
        }
        printf("강화할 스킬 번호 선택 (0: 종료): ");
        int choice;
        scanf("%d", &choice);
        if (choice == 0)
            break;
        if (choice < 1 || choice > p->skillCount)
        {
            printf("잘못된 입력입니다.\n");
            continue;
        }
        if (p->skillPoints <= 0)
        {
            printf("스킬포인트가 부족합니다.\n");
            continue;
        }
        p->skills[choice - 1].level++;
        p->skillPoints--;
        printf("%s 스킬이 Lv %d로 강화되었습니다!\n", p->skills[choice - 1].name, p->skills[choice - 1].level);
    }
}

// 전투 중 스킬을 선택하여 사용하는 함수
void UseSkillInCombat(Player* p, int* monsterHp) {
    if (p->skillCount == 0) {
        printf("스킬이 없습니다. 기본 공격으로 진행합니다.\n");
        return;
    }
    printf("=== 사용 가능한 공격 스킬 목록 ===\n");
    int skillOptions[MAX_SKILLS];
    int attackSkillCount = 0;
    for (int i = 0; i < p->skillCount; i++) {
        if (p->skills[i].type == SKILL_ATTACK) {
            skillOptions[attackSkillCount] = i;
            printf("%d. %s (Lv %d) - %s\n", attackSkillCount + 1, p->skills[i].name, p->skills[i].level, p->skills[i].description);
            attackSkillCount++;
        }
    }
    if (attackSkillCount == 0) {
        printf("사용 가능한 공격 스킬이 없습니다. 기본 공격으로 진행합니다.\n");
        int damage = (p->str + p->dex + p->luk + p->intl) / 4;
        *monsterHp -= damage;
        printf("기본 공격! 적에게 %d 데미지를 입혔습니다!\n", damage);
        return;
    }
    // Check if all attack skills have level 0
    int usableAttackSkillCount = 0;
    for (int i = 0; i < attackSkillCount; i++) {
        if (p->skills[skillOptions[i]].level > 0) {
            usableAttackSkillCount++;
        }
    }
    if (usableAttackSkillCount == 0) {
        printf("모든 공격 스킬의 레벨이 0입니다. 기본 공격으로 진행합니다.\n");
        int damage = (p->str + p->dex + p->luk + p->intl) / 4;
        *monsterHp -= damage;
        printf("기본 공격! 적에게 %d 데미지를 입혔습니다!\n", damage);
        return;
    }
    printf("사용할 스킬 번호를 입력하세요: ");
    int skillNum;
    scanf("%d", &skillNum);

    if (skillNum < 1 || skillNum > attackSkillCount) {
        printf("잘못된 입력입니다.\n");
        return;
    }

    Skill* s = &p->skills[skillOptions[skillNum - 1]];

    // 스킬 레벨 확인
    if (s->level <= 0) {
        printf("%s 스킬은 레벨이 0이라 사용할 수 없습니다! 기본 공격으로 전환합니다!\n", s->name);
        int damage = (p->str + p->dex + p->luk + p->intl) / 4;
        *monsterHp -= damage;
        printf("기본 공격! 적에게 %d 데미지를 입혔습니다!\n", damage);
        return;
    }
    int effectivePower = s->power * s->level;
    *monsterHp -= effectivePower;
    printf("%s 스킬을 사용하여 적에게 %d 데미지를 입혔습니다!\n", s->name, effectivePower);
}

// 전투를 시뮬레이션하는 메인 전투 함수
void SimulateCombat(Player* player) {
    // 몬스터 Specification 관련 구조체
    typedef struct {
        char name[100];     // 몬스터 이름
        int hp;             // 몬스터 체력량
        int attack;         // 몬스터 공격력(플레이어에게 입히는 데미지량)
        int expReward;      // 몬스터 처치시 경험치 획득량
    } Monster;

    // 몬스터 이름 Byte 수 관련 구조체
    typedef struct {
        char name[100];
        Monster monsters[5];
    } Field;

    Field fields[] = {
        {
            // 저레벨 특화
            "빅토리아 아일랜드",                        // 필드 이름
            {
                {"스포아", 30, 5, 10},               // 몬스터 이름, 몬스터 체력, 몬스터 공격력, 경험치 획득량
                {"주황 버섯", 50, 7, 15},
                {"초록 버섯", 40, 6, 12},
                {"파란 버섯", 45, 8, 18},
                {"머쉬맘(이지 보스)", 100, 15, 35}
            }
        },
        {
            // 저레벨 특화
            "그란디스 행성",
            {
                {"드레이크", 59, 14, 25},
                {"아이스 드레이크", 99, 19, 29},
                {"파이어 드레이크", 124, 37, 40},
                {"저주받은 드레이크", 172, 54, 68},
                {"자이언트 발록(이지 보스)", 219, 80, 95}
            }
        },
        {
            // 중레벨 특화
            "[과거의 문] 추억의 길",
            {
                {"추억의 사제", 250, 70, 80},
                {"추억의 신관", 290, 82, 89},
                {"추억의 수호병", 330, 99, 102},
                {"추억의 수호대장", 390, 105, 107},
                {"자이언트 도도 (이지 보스)", 5000, 120, 210}
            }
        },
        {
            // 중레벨 특화
            "[과거의 문] 후회의 길",
            {
                {"후회의 사제", 350, 110, 110},
                {"후회의 신관", 380, 123, 117},
                {"후회의 수호병", 420, 140, 129},
                {"후회의 수호대장", 450, 155, 134},
                {"자이언트 릴리노흐 (이지 보스)", 10000, 179, 433}
            }
        },
        {
            // 중레벨 특화
            "[과거의 문] 망각의 길",
            {
                {"망각의 사제", 460, 168, 143},
                {"망각의 신관", 487, 175, 159},
                {"망각의 수호병", 497, 181, 170},
                {"망각의 수호대장", 520, 225, 200},
                {"자이언트 라이카 (이지 보스)", 15000, 289, 766}
            }
        },
        {
            // 중~고레벨 특화
            "[미래의 문] 파괴된 빅토리아 아일랜드",
            {
                {"변형된 스포아", 670, 369, 326},
                {"변형된 주황 버섯", 782, 399, 360},
                {"변형된 초록 버섯", 839, 451, 422},
                {"변형된 파란 버섯", 910, 505, 499},
                {"변형된 머쉬맘(이지 보스)", 17500, 599, 1030}
            }
        },
        {
            // 고레벨 특화
            "[미래의 문] 신들의 황혼",
            {
                {"황혼의 사제", 1392, 690, 590},
                {"황혼의 신관", 1911, 888, 914},
                {"황혼의 수호병", 2044, 1011, 999},
                {"황혼의 수호대장", 2095, 1234, 1080},
                {"황혼의 핑크빈(이지 보스)", 20000, 1392, 2030}
            }
        },
        {
            // 고레벨 특화
            "[현재의 문] 아케인리버",
            {
                {"소멸의 루시드", 3348, 1679, 1099},
                {"츄츄의 윌", 3901, 1724, 1135},
                {"얌얌의 진 힐라", 4217, 2000, 1270},
                {"거대 괴수 더스크", 4999, 2077, 1299},
                {"친위대장 듄켈(이지 보스)", 25000, 2222, 2060}
            }
        },
        {
            // 파티, 원정대 등 2~3인 이상 협동컨텐츠
            "[현재의 문] 리멘",
            {
                {"마법사의 알바생", 6073290, 4062, 372},
                {"하얀 마법사의 하수인", 14837294, 7423, 500},
                {"검은 마법사의 하수인", 19284739, 7973, 600},
                {"하얀 마법사 (노말 보스)", 1073741823, 10737, 200000},
                {"검은 마법사 (카오스 보스)", 2147483647, 21474, 400000}
            }
        }
    };
    
/* 사냥터 추가시 바꿔야 할 부분 시작 */
    printf("=== 사냥터 선택 ===\n");
    for (int i = 0; i < 9; i++) {
        printf("%d. %s\n", i + 1, fields[i].name);
    }

    int fieldChoice;
    printf("사냥터를 선택하세요: ");
    scanf("%d", &fieldChoice);
    if (fieldChoice < 1 || fieldChoice > 9) {
        printf("잘못된 선택입니다.\n");
        return;
    }
/* 사냥터 추가시 바꿔야 할 부분 끝 */

    Field selectedField = fields[fieldChoice - 1];

/* 몬스터 추가시 바꾸야 할 부분 시작 */
    printf("=== 몬스터 선택 ===\n");
    for (int i = 0; i < 5; i++) {
        printf("%d. %s (HP: %d, 공격력: %d, EXP: %d)\n",
               i + 1, selectedField.monsters[i].name,
               selectedField.monsters[i].hp,
               selectedField.monsters[i].attack,
               selectedField.monsters[i].expReward);
    }

    int monsterChoice;
    printf("몬스터를 선택하세요: ");
    scanf("%d", &monsterChoice);
    if (monsterChoice < 1 || monsterChoice > 5) {
        printf("잘못된 선택입니다.\n");
        return;
    }
/* 몬스터 추가시 바꿔야 할 부분 끝 */

    Monster m = selectedField.monsters[monsterChoice - 1];
    printf("전투 시작: %s 등장!\n", m.name);

    int monsterHp = m.hp;
    while (player->hp > 0 && monsterHp > 0) {
        printf("\n당신의 HP: %d, %s의 HP: %d\n", player->hp, m.name, monsterHp);
        printf("1. 기본 공격\n2. 스킬 사용\n3. 회복 사용\n선택: ");
        int action;
        scanf("%d", &action);

        int damage = (player->str + player->dex + player->luk + player->intl) / 4;
        
        // 스킬 사용 시 스킬의 공격력 * 스킬의 레벨만큼 몬스터에게 데미지를 입힘
        if (action == 2) {
            UseSkillInCombat(player, &monsterHp);
        }
        // 전투 도중 체력 회복(자신의 최대 체력에서 랜덤으로 100%까지 획득
        else if (action == 3) {
            int hpPercent = rand() % 100 + 1;
            int mpPercent = rand() % 100 + 1;
            int hpHeal = player->maxHp * hpPercent / 100;
            int mpHeal = player->maxMp * mpPercent / 100;
            player->hp += hpHeal;
            if (player->hp > player->maxHp) player->hp = player->maxHp;
            player->mp += mpHeal;
            if (player->mp > player->maxMp) player->mp = player->maxMp;
            printf("체력을 %d%% 회복했습니다! (+%d HP)\n", hpPercent, hpHeal);
            printf("마나를 %d%% 회복했습니다! (+%d MP)\n", mpPercent, mpHeal);
        }
        // 기본 공격(본인의 4가지 스탯에 영향을 받음
        else {
            printf("기본 공격!\n");
            monsterHp -= damage;
            printf("%s에게 %d 데미지를 입혔습니다!\n", m.name, damage);
        }

        // 몬스터의 반격
        if (monsterHp > 0 && action != 3) {
            player->hp -= m.attack;
            printf("%s의 반격! %d 데미지를 입었습니다!\n", m.name, m.attack);
        }
    }
    
    // 플레이어가 사망한 경우 전투 종료
    if (player->hp <= 0) {
        printf("HP가 0이 되어 전투에서 패배하였습니다.\n마을로 돌아갑니다.\n");
        player->hp = 10;
        player->mp = 10;
        return;
    }

    printf("전투 승리! 경험치 %d 획득!\n", m.expReward);
    player->exp += m.expReward;

    int expIntoCurrentLevel = player->exp - expTable[player->level];
    if (player->level >= 200) expIntoCurrentLevel = 0;
    //int requiredExpForNext = (player->level >= 200) ? 1 : expTable[player->level + 1] - expTable[player->level];

    // 레벨업 관련 로직 (최대 200LV), 각 레벨마다 필요 경험치 10씩 증가(10, 20, 30, 40, ... , 80, 90, 100, ...)
    while (player->level < 200 && player->exp >= expTable[player->level + 1]) {
        player->exp = 0; // 레벨업하면 경험치 0으로 초기화
        player->level++;
        player->statPoints += STAT_POINTS_PER_LEVEL;
        player->skillPoints += SKILL_POINTS_PER_LEVEL;
        printf("레벨업! 현재 레벨: %d\n", player->level);
        int hpGain = player->maxHp * 0.1;
        int mpGain = player->maxMp * 0.1;
        player->maxHp += hpGain;
        player->maxMp += mpGain;
        printf("최대 HP/MP 증가! (HP: %d → %d, MP: %d → %d)\n",
               player->maxHp - hpGain, player->maxHp,
               player->maxMp - mpGain, player->maxMp);
        player->hp = player->maxHp;
        player->mp = player->maxMp;
    }

    printf("===========================\n");
}

// 전직 조건을 확인하고 직업을 변경하는 함수
void TryJobChange(Player* player) {
    // Determine advancement flags by checking if player has skills of each tier
    bool hasFirstJob = false, hasSecondJob = false, hasThirdJob = false, hasFourthJob = false;
    for (int i = 0; i < player->skillCount; i++) {
        if (player->skills[i].tier == 1) hasFirstJob = true;
        if (player->skills[i].tier == 2) hasSecondJob = true;
        if (player->skills[i].tier == 3) hasThirdJob = true;
        if (player->skills[i].tier == 4) hasFourthJob = true;
    }

    // 1차 전직: 레벨 10 이상, 초보자
    if (player->job == JOB_NONE && player->level >= 10) {
        printf("1차 전직 가능! 직업을 선택하세요:\n");
        printf("1. 전사\n2. 마법사\n3. 도적\n4. 궁수\n선택: ");
        int choice;
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                player->job = JOB_WARRIOR;
                player->jobTier = 1;
                printf("1차 전직 완료: 검사!\n");
                break;
            case 2:
                player->job = JOB_MAGICIAN;
                player->jobTier = 1;
                printf("1차 전직 완료: 매지션!\n");
                break;
            case 3:
                player->job = JOB_THIEF;
                player->jobTier = 1;
                printf("1차 전직 완료: 로그!\n");
                break;
            case 4:
                player->job = JOB_ARCHER;
                player->jobTier = 1;
                printf("1차 전직 완료: 아처!\n");
                break;
            default:
                printf("잘못된 선택입니다.\n");
                return;
        }
        // 1차 스킬 부여
        player->skillCount = 0;
        player->skills[player->skillCount++] = beginnerSkills[0];
        AddJobSkills(player,
            player->job == JOB_WARRIOR ? warriorSkills :
            player->job == JOB_MAGICIAN ? magicianSkills :
            player->job == JOB_THIEF ? thiefSkills :
            archerSkills,
            player->job == JOB_WARRIOR ? sizeof(warriorSkills) / sizeof(Skill) :
            player->job == JOB_MAGICIAN ? sizeof(magicianSkills) / sizeof(Skill) :
            player->job == JOB_THIEF ? sizeof(thiefSkills) / sizeof(Skill) :
            sizeof(archerSkills) / sizeof(Skill),
            1);
        // 안내 메시지
        const char* jobName = player->job == JOB_WARRIOR ? "전사" :
                              player->job == JOB_MAGICIAN ? "마법사" :
                              player->job == JOB_THIEF ? "도적" :
                              player->job == JOB_ARCHER ? "궁수" : "초보자";
        printf("현재 직업: 1차 %s\n", jobName);
        return;
    }
    
    // 2차 전직: 레벨 30 이상, 1차 전직 스킬 있음, 2차 스킬 없음
    else if (player->level >= 30 && player->job != JOB_NONE && hasFirstJob && !hasSecondJob) {
        switch (player->job) {
            case JOB_WARRIOR:
                printf("2차 전직 완료: 파이터!\n");
                AddJobSkills(player, warriorSkills, sizeof(warriorSkills) / sizeof(Skill), 2);
                player->jobTier = 2;
                printf("현재 직업: 2차 전사\n");
                break;
            case JOB_MAGICIAN:
                printf("2차 전직 완료: 클레릭!\n");
                AddJobSkills(player, magicianSkills, sizeof(magicianSkills) / sizeof(Skill), 2);
                player->jobTier = 2;
                printf("현재 직업: 2차 마법사\n");
                break;
            case JOB_THIEF:
                printf("2차 전직 완료: 시프!\n");
                AddJobSkills(player, thiefSkills, sizeof(thiefSkills) / sizeof(Skill), 2);
                player->jobTier = 2;
                printf("현재 직업: 2차 도적\n");
                break;
            case JOB_ARCHER:
                printf("2차 전직 완료: 헌터!\n");
                AddJobSkills(player, archerSkills, sizeof(archerSkills) / sizeof(Skill), 2);
                player->jobTier = 2;
                printf("현재 직업: 2차 궁수\n");
                break;
            default:
                printf("전직 조건을 만족하지 않습니다.\n");
                break;
        }
        return;
    }
    
    // 3차 전직: 레벨 70 이상, 2차 전직 스킬 있음, 3차 스킬 없음
    else if (player->level >= 70 && player->job != JOB_NONE && hasSecondJob && !hasThirdJob) {
        switch (player->job) {
            case JOB_WARRIOR:
                printf("3차 전직 완료: 크루세이더!\n");
                AddJobSkills(player, warriorSkills, sizeof(warriorSkills) / sizeof(Skill), 3);
                player->jobTier = 3;
                printf("현재 직업: 3차 전사\n");
                break;
            case JOB_MAGICIAN:
                printf("3차 전직 완료: 프리스트!\n");
                AddJobSkills(player, magicianSkills, sizeof(magicianSkills) / sizeof(Skill), 3);
                player->jobTier = 3;
                printf("현재 직업: 3차 마법사\n");
                break;
            case JOB_THIEF:
                printf("3차 전직 완료: 시프마스터!\n");
                AddJobSkills(player, thiefSkills, sizeof(thiefSkills) / sizeof(Skill), 3);
                player->jobTier = 3;
                printf("현재 직업: 3차 도적\n");
                break;
            case JOB_ARCHER:
                printf("3차 전직 완료: 레인저!\n");
                AddJobSkills(player, archerSkills, sizeof(archerSkills) / sizeof(Skill), 3);
                player->jobTier = 3;
                printf("현재 직업: 3차 궁수\n");
                break;
            default:
                printf("전직 조건을 만족하지 않습니다.\n");
                break;
        }
        return;
    }
    
    // 4차 전직: 레벨 120 이상, 3차 전직 스킬 있음, 4차 스킬 없음
    else if (player->level >= 120 && player->job != JOB_NONE && hasThirdJob && !hasFourthJob) {
        switch (player->job) {
            case JOB_WARRIOR:
                printf("4차 전직 완료: 히어로!\n");
                AddJobSkills(player, warriorSkills, sizeof(warriorSkills) / sizeof(Skill), 4);
                player->jobTier = 4;
                printf("현재 직업: 4차 전사\n");
                break;
            case JOB_MAGICIAN:
                printf("4차 전직 완료: 비숍!\n");
                AddJobSkills(player, magicianSkills, sizeof(magicianSkills) / sizeof(Skill), 4);
                player->jobTier = 4;
                printf("현재 직업: 4차 마법사\n");
                break;
            case JOB_THIEF:
                printf("4차 전직 완료: 섀도어!\n");
                AddJobSkills(player, thiefSkills, sizeof(thiefSkills) / sizeof(Skill), 4);
                player->jobTier = 4;
                printf("현재 직업: 4차 도적\n");
                break;
            case JOB_ARCHER:
                printf("4차 전직 완료: 보우마스터!\n");
                AddJobSkills(player, archerSkills, sizeof(archerSkills) / sizeof(Skill), 4);
                player->jobTier = 4;
                printf("현재 직업: 4차 궁수\n");
                break;
            default:
                printf("전직 조건을 만족하지 않습니다.\n");
                break;
        }
        return;
    }
    else {
        printf("전직 조건을 만족하지 않습니다.\n");
    }
}

// 경험치 테이블을 초기화하는 함수
void InitializeExpTable(void) {
    expTable[0] = 0;
    for (int i = 1; i <= MAX_LEVEL; i++) {
        expTable[i] = expTable[i - 1] + i * 10;
    }
}

// 밍밍이스토리 스토리모드
void StoryMode(void) {
    const char* story[] = {
        "==== 밍밍이스토리 스토리모드 ====",
        "이곳은 전설의 빅토리아 아일랜드!",
        "귀엽고 개성만점 몬스터들이 한가득!",
        "하지만 평화는 오래가지 않았다...",
        "어느 날! 하늘에서 검은 마법사가 나타나",
        "모든 아일랜드를 상대로 모두에게 도전을 선포했다!",
        "",
        "",
        "지금 이 순간, 용사 당신의 모험을 시작한다!",
        "",
        "아일랜드부터 신들의 황혼, 아케인리버, 그란디스 행성까지...",
        "마법사들의 근거지 모든 리멘의 비밀을 밝혀내라!",
        "최종보스 검은 마법사와의 마지막 대결이 기다린다!",
        "====================================",
        "※ 밍밍이와 함께 하는 스토리, 직접 완성해봐~! ※"
    };
    int n = sizeof(story)/sizeof(story[0]);
    for (int i = 0; i < n; i++) {
        if (i == 8) {
            printf(story[i], "플레이어"); // p->name 변수 넘기면 이름 넣기 가능
        } else {
            printf("%s\n", story[i]);
        }
        fflush(stdout); // 바로바로 출력
        usleep(2500000); // 2.5s(2500000us) 대기
    }
}

// 메인 함수: 게임의 전체 흐름을 관리
int main(void) {
    InitializeExpTable();
    Player p;

    srand((unsigned int)time(NULL));

    // 플레이어 닉네임 설정
    char name[100];
    printf("캐릭터 이름을 입력하세요: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';   // 개행 문자 제거

    // 저장데이터 관련, 있으면 if 실행, 없으면 else 실행
    if (LoadPlayerData(&p, "player.data", name)) {
        printf("저장된 데이터를 불러왔습니다!\n");
    } else {
        strcpy(p.name, name);
        p.level = 1;
        p.exp = 0;
        p.maxHp = 100 + p.level * 25;
        p.maxMp = 100 + p.level * 15;
        p.hp = p.maxHp;
        p.mp = p.maxMp;
        p.str = 4;
        p.dex = 4;
        p.luk = 4;
        p.intl = 4;
        p.job = JOB_NONE;
        p.jobTier = 0;
        p.statPoints = 0;
        p.skillPoints = 0;
        p.skillCount = 0;
        p.skills[p.skillCount++] = beginnerSkills[0];
    }
    // Safeguard: ensure at least the basic skill is present
    if (p.skillCount == 0) {
        p.skills[p.skillCount++] = beginnerSkills[0];
    }
    // Ensure all job skills up to jobTier are granted
    Skill* jobSkills = NULL;
    int jobSkillCount = 0;
    if (p.job == JOB_WARRIOR) { jobSkills = warriorSkills; jobSkillCount = sizeof(warriorSkills)/sizeof(Skill); }
    else if (p.job == JOB_MAGICIAN) { jobSkills = magicianSkills; jobSkillCount = sizeof(magicianSkills)/sizeof(Skill); }
    else if (p.job == JOB_THIEF) { jobSkills = thiefSkills; jobSkillCount = sizeof(thiefSkills)/sizeof(Skill); }
    else if (p.job == JOB_ARCHER) { jobSkills = archerSkills; jobSkillCount = sizeof(archerSkills)/sizeof(Skill); }

    if (jobSkills != NULL) {
        for (int t = 1; t <= p.jobTier; t++) {
            AddJobSkills(&p, jobSkills, jobSkillCount, t);
        }
    }
    
    // 게임 내 메뉴 선택 관련
    int running = 1;
    while (running) {
        printf("\n=== 메뉴 ===\n");
        printf("1. 캐릭터 정보 보기\n");
        printf("2. 전투 시작\n");
        printf("3. 전직 시도\n");
        printf("4. 스탯 분배\n");
        printf("5. 스킬 강화\n");
        printf("6. 저장하고 종료\n");
        printf("7. 스토리 보기\n");
        printf("선택: ");
        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                DisplayPlayerInfo(&p);
                break;
            case 2:
                SimulateCombat(&p);
                break;
            case 3:
                TryJobChange(&p);
                break;
            case 4:
                if (p.statPoints > 0) {
                    printf("스탯 자동배분(A) 또는 수동배분(M)? : ");
                    char mode;
                    scanf(" %c", &mode);
                    if (mode == 'A' || mode == 'a') {
                        AutoDistributeStats(&p);
                    }
                    else if (mode == 'M' || mode == 'm') {
                        ManualDistributeStats(&p);
                    }
                    else {
                        printf("잘못된 입력입니다. 스탯 분배를 건너뜁니다.\n");
                    }
                }
                else {
                    printf("남은 스탯 포인트가 없습니다.\n");
                }
                break;
            case 5:
                UpgradeSkill(&p);
                break;
            case 6:
                if (SavePlayerData(&p)) {
                    printf("저장 완료!\n");
                }
                running = 0;
                break;
            case 7:
                StoryMode();    // 밍밍이 스토리 스토리 감상하기
                break;
            default:
                printf("잘못된 선택입니다.\n");
                break;
        }
    }

    return 0;
}
