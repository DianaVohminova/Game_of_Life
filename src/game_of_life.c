#include <ncurses.h>
#include <stdio.h>

#define WIDTH 80
#define HEIGHT 25
#define MIN_DELAY 0
#define MAX_DELAY 100
#define SPEED_STEP 20

int GetCell(const int field[HEIGHT][WIDTH], int row, int col);
int CountNeighbors(const int field[HEIGHT][WIDTH], int row, int col);
void UpdateGeneration(const int current[HEIGHT][WIDTH], int next[HEIGHT][WIDTH]);

void ClearField(int field[HEIGHT][WIDTH]);
int IsValidLine(const char *line);
void SetFieldRow(int field[HEIGHT][WIDTH], const char *line, int row);
int ReadFieldRow(int field[HEIGHT][WIDTH], int row);
int ReadInitialState(int field[HEIGHT][WIDTH]);

void InitUI(void);
void DrawField(const int field[HEIGHT][WIDTH], int *delay);
int ProcessInput(int *delay);
void WaitDelay(int delay);
void CloseUI(void);
void CopyField(const int source[HEIGHT][WIDTH], int destination[HEIGHT][WIDTH]);

int main(void) {
    int current[HEIGHT][WIDTH];
    int delay = 40;
    int result = 0;

    if (!ReadInitialState(current)) {
        result = 1;
    }

    if (result == 0) {
        if (freopen("/dev/tty", "r", stdin) == NULL) {
            result = 1;
        }
    }

    if (result == 0) {
        InitUI();

        int next[HEIGHT][WIDTH];
        int running = 1;
        while (running) {
            DrawField(current, &delay);

            running = ProcessInput(&delay);

            if (running) {
                UpdateGeneration(current, next);
                CopyField(next, current);
                WaitDelay(delay);
            }
        }

        CloseUI();
    }

    return result;
}
void CopyField(const int source[HEIGHT][WIDTH], int destination[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            destination[row][col] = source[row][col];
        }
    }
}

int GetCell(const int field[HEIGHT][WIDTH], int row, int col) {
    int wrapped_row = (row + HEIGHT) % HEIGHT;
    int wrapped_col = (col + WIDTH) % WIDTH;
    int result = field[wrapped_row][wrapped_col];

    return result;
}

int CountNeighbors(const int field[HEIGHT][WIDTH], int row, int col) {
    int count = 0;

    for (int row_offset = -1; row_offset <= 1; row_offset++) {
        for (int col_offset = -1; col_offset <= 1; col_offset++) {
            if (row_offset != 0 || col_offset != 0) {
                count += GetCell(field, row + row_offset, col + col_offset);
            }
        }
    }

    return count;
}

void UpdateGeneration(const int current[HEIGHT][WIDTH], int next[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            int neighbors = CountNeighbors(current, row, col);
            int alive = 0;

            if (current[row][col]) {
                if (neighbors == 2 || neighbors == 3) {
                    alive = 1;
                }
            } else if (neighbors == 3) {
                alive = 1;
            }

            next[row][col] = alive;
        }
    }
}

void ClearField(int field[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            field[row][col] = 0;
        }
    }
}

int IsValidLine(const char *line) {
    int col = 0;
    int valid = 1;

    while (line[col] != '\0' && line[col] != '\n') {
        if (line[col] != '.' && line[col] != '*') {
            valid = 0;
        }

        col++;
    }

    if (col != WIDTH) {
        valid = 0;
    }

    return valid;
}

void SetFieldRow(int field[HEIGHT][WIDTH], const char *line, int row) {
    for (int col = 0; col < WIDTH; col++) {
        field[row][col] = line[col] == '*';
    }
}

int ReadFieldRow(int field[HEIGHT][WIDTH], int row) {
    char line[WIDTH + 2];
    int valid = 0;

    if (fgets(line, sizeof(line), stdin) != NULL) {
        if (IsValidLine(line)) {
            SetFieldRow(field, line, row);
            valid = 1;
        }
    }

    return valid;
}

int ReadInitialState(int field[HEIGHT][WIDTH]) {
    int row = 0;
    int valid = 1;

    ClearField(field);

    while (row < HEIGHT && valid) {
        valid = ReadFieldRow(field, row);

        if (valid) {
            row++;
        }
    }

    return valid && row == HEIGHT;
}

void InitUI(void) {
    initscr();
    noecho();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);
}

void DrawField(const int field[HEIGHT][WIDTH], int *delay) {
    erase();

    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            char symbol = field[row][col] ? '*' : ' ';

            mvaddch(row, col, symbol);
        }
    }

    mvprintw(HEIGHT, 0, "A: faster | Z: slower | SPACE: exit | Delay: %d ms", *delay);

    refresh();
}

void IncreaseSpeed(int *delay) {
    if (*delay - SPEED_STEP >= MIN_DELAY) {
        *delay -= SPEED_STEP;
    }
}

void DecreaseSpeed(int *delay) {
    if (*delay + SPEED_STEP <= MAX_DELAY) {
        *delay += SPEED_STEP;
    }
}

int ProcessInput(int *delay) {
    int ch = getch();
    int running = 1;

    if (ch == ' ') {
        running = 0;
    } else if (ch == 'a' || ch == 'A') {
        IncreaseSpeed(delay);
    } else if (ch == 'z' || ch == 'Z') {
        DecreaseSpeed(delay);
    }

    return running;
}

void WaitDelay(int delay) { napms(delay); }

void CloseUI(void) { endwin(); }
