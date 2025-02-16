#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define TILE_SIZE 75
#define BOARD_SIZE 8
#define FPS 60
#define NUM_PIECES 10

typedef struct {
    int x, y;
} Position;

typedef struct {
    int speed, attack, defense;
} Stats;

typedef struct {
    Position pos;
    Stats stats; 
    int type; // 0: None, 1: King, 2: Queen, etc.
    int player; // 1 or 2
    SDL_Texture* texture;
} Piece;

// Timeline queue for turn order
Piece *turnOrder[NUM_PIECES];
int turnIndex = 0;

void sortTurnOrder(Piece *pieces, int num_pieces) {
    for (int i = 0; i < num_pieces; i++) {
        turnOrder[i] = &pieces[i];
    }
    for (int i = 0; i < num_pieces - 1; i++) {
        for (int j = i + 1; j < num_pieces; j++) {
            if (turnOrder[i]->stats.speed < turnOrder[j]->stats.speed) {
                Piece *temp = turnOrder[i];
                turnOrder[i] = turnOrder[j];
                turnOrder[j] = temp;
            }
        }
    }
}

Piece *getNextTurn() {
    Piece *nextPiece = turnOrder[turnIndex];
    turnIndex = (turnIndex + 1) % NUM_PIECES;
    return nextPiece;
}

bool isValidMove(Piece *piece, int newX, int newY) {
    switch (piece->type) {
        case 1: // King: One tile in any direction
            return abs(newX - piece->pos.x) <= 1 && abs(newY - piece->pos.y) <= 1;
        case 2: // Queen: Any straight line
            return (newX == piece->pos.x || newY == piece->pos.y || abs(newX - piece->pos.x) == abs(newY - piece->pos.y));
        case 3: // Bishop: Diagonal movement
            return abs(newX - piece->pos.x) == abs(newY - piece->pos.y);
        case 4: // Rook: Vertical or horizontal movement
            return (newX == piece->pos.x || newY == piece->pos.y);
        case 5: // Knight: L-shaped movement
            return (abs(newX - piece->pos.x) == 2 && abs(newY - piece->pos.y) == 1) || (abs(newX - piece->pos.x) == 1 && abs(newY - piece->pos.y) == 2);
        default:
            return false;
    }
}

void movePiece(Piece *piece, int newX, int newY) {
    if (isValidMove(piece, newX, newY)) {
        piece->pos.x = newX;
        piece->pos.y = newY;
        printf("Piece moved to (%d, %d)\n", newX, newY);
    } else {
        printf("Invalid move!\n");
    }
}

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* textures[NUM_PIECES];
Piece board[BOARD_SIZE][BOARD_SIZE];
Piece *selectedPiece = NULL;

bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    window = SDL_CreateWindow("Tactics Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

SDL_Texture* loadTexture(const char* path) {
    SDL_Texture* newTexture = NULL;
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
    } else {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == NULL) {
            printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

void drawBoard() {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            SDL_Rect tile = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            if ((row + col) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            }
            SDL_RenderFillRect(renderer, &tile);
        }
    }
}

void renderPieces(Piece *pieces, int num_pieces) {
    for (int i = 0; i < num_pieces; i++) {
        SDL_Rect rect = {pieces[i].pos.x * TILE_SIZE + 20, pieces[i].pos.y * TILE_SIZE + 20, 40, 40};
        SDL_RenderCopy(renderer, pieces[i].texture, NULL, &rect);
    }
}

void handleMouseClick(int x, int y, Piece *pieces) {
    int boardX = x / TILE_SIZE;
    int boardY = y / TILE_SIZE;
    
    for (int i = 0; i < NUM_PIECES; i++) {
        if (pieces[i].pos.x == boardX && pieces[i].pos.y == boardY) {
            selectedPiece = &pieces[i];
            printf("Piece selected at (%d, %d)\n", boardX, boardY);
            return;
        }
    }
    if (selectedPiece) {
        movePiece(selectedPiece, boardX, boardY);
        selectedPiece = NULL;
    }
}

void gameLoop(Piece *pieces) {
    bool running = true;
    SDL_Event e;
    Uint32 frameStart;
    int frameTime;
    const int frameDelay = 1000 / FPS;
    
    sortTurnOrder(pieces, NUM_PIECES);

    while (running) {
        frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
            if (e.type == SDL_MOUSEBUTTONDOWN) handleMouseClick(e.button.x, e.button.y, pieces);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        drawBoard();
        renderPieces(pieces, NUM_PIECES);
        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < frameDelay) SDL_Delay(frameDelay - frameTime);
    }
}

void closeSDL() {
    for (int i = 0; i < NUM_PIECES; i++) {
        if (textures[i] != NULL) {
            SDL_DestroyTexture(textures[i]);
            textures[i] = NULL;
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    if (!initSDL()) return 1;
    
    // Load textures
    textures[0] = loadTexture("black-king.png");
    textures[1] = loadTexture("black-queen.png");
    textures[2] = loadTexture("black-bishop.png");
    textures[3] = loadTexture("black-rook.png");
    textures[4] = loadTexture("black-knight.png");
    textures[5] = loadTexture("white-king.png");
    textures[6] = loadTexture("white-queen.png");
    textures[7] = loadTexture("white-bishop.png");
    textures[8] = loadTexture("white-rook.png");
    textures[9] = loadTexture("white-knight.png");

    Piece pieces[NUM_PIECES] = {
        {{0, 0}, {5, 10, 5}, 1, 1, textures[0]},
        {{1, 0}, {3, 8, 6}, 2, 1, textures[1]},
        {{2, 0}, {4, 7, 7}, 3, 1, textures[2]},
        {{3, 0}, {6, 9, 4}, 4, 1, textures[3]},
        {{4, 0}, {3, 8, 6}, 5, 1, textures[4]},
        {{0, 7}, {4, 7, 7}, 1, 2, textures[5]},
        {{1, 7}, {5, 10, 5}, 2, 2, textures[6]},
        {{2, 7}, {3, 8, 6}, 3, 2, textures[7]},
        {{3, 7}, {4, 7, 7}, 4, 2, textures[8]},
        {{4, 7}, {6, 9, 4}, 5, 2, textures[9]}
    };

    gameLoop(pieces);
    closeSDL();
    return 0;
}