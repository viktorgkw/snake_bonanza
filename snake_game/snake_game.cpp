#include "raylib.h"
#include <deque>
#include <string>
#include <iostream>

const int screenWidth = 800;
const int screenHeight = 600;
const int cellSize = 30;

struct SnakeSegment {
	int x, y;
};

static bool IsOnSnake(const std::deque<SnakeSegment>& snake, int x, int y) {
	for (const auto& s : snake) {
		if (s.x == x && s.y == y) return true;
	}
	return false;
}

static Vector2 GetRandomFoodPosition(const std::deque<SnakeSegment>& snake) {
	Vector2 pos;

	int gridWidth = screenWidth / cellSize;
	int gridHeight = screenHeight / cellSize;

	int topOffset = 2;

	do {
		pos = {
			(float)GetRandomValue(0, gridWidth - 1),
			(float)GetRandomValue(topOffset, gridHeight - 1)
		};
	} while (IsOnSnake(snake, (int)pos.x, (int)pos.y));

	return pos;
}

static Color GetRandomColor() {
	Color randomColor = {
		(unsigned char)GetRandomValue(80, 255),
		(unsigned char)GetRandomValue(80, 255),
		(unsigned char)GetRandomValue(80, 255),
		255
	};

	return randomColor;
}

int main() {
	InitWindow(screenWidth, screenHeight, "Snake (raylib)");
	SetTargetFPS(60);

	std::deque<SnakeSegment> snake = { {10, 10} };

	Vector2 direction = { 1, 0 };
	Color backgroundColor = GetRandomColor();

	Vector2 food = GetRandomFoodPosition(snake);
	Color foodColor = GetRandomColor();

	bool showBonanza = false;
	bool bonanzaUsed = false;
	float bonanzaTimer = 0.0f;

	bool gameOver = false;
	int points{ 0 };

	float moveTimer = 0;
	float moveDelay = 0.15f;

	while (!WindowShouldClose()) {
		float delta = GetFrameTime();
		moveTimer += delta;

		Vector2 nextDirection = direction;

		// INPUT
		if (IsKeyPressed(KEY_UP) && direction.y != 1) nextDirection = { 0, -1 };
		if (IsKeyPressed(KEY_DOWN) && direction.y != -1) nextDirection = { 0, 1 };
		if (IsKeyPressed(KEY_LEFT) && direction.x != 1) nextDirection = { -1, 0 };
		if (IsKeyPressed(KEY_RIGHT) && direction.x != -1) nextDirection = { 1, 0 };

		direction = nextDirection;

		// BONANZA
		if (points > 0 && points % 5 == 0 && !showBonanza && !bonanzaUsed)
		{
			bonanzaUsed = true;
			showBonanza = true;
			bonanzaTimer = 3.0f;
			moveDelay = 0.05f;
		}

		if (showBonanza)
		{
			bonanzaTimer -= GetFrameTime();

			if (bonanzaTimer <= 0.0f)
			{
				showBonanza = false;
				moveDelay = 0.06f;
			}
		}

		// RESTART
		if (gameOver && IsKeyPressed(KEY_R)) {
			gameOver = false;
			snake = { {10, 10} };
			direction = { 1, 0 };
			moveDelay = 0.15f;
			points = 0;

			showBonanza = false;
			bonanzaUsed = false;

			food = GetRandomFoodPosition(snake);
			foodColor = GetRandomColor();
			backgroundColor = GetRandomColor();
		}

		// MOVE ONLY WHEN TIMER TRIGGERS
		if (!gameOver && moveTimer >= moveDelay) {
			moveTimer = 0;

			SnakeSegment newHead = {
				snake.front().x + (int)direction.x,
				snake.front().y + (int)direction.y,
			};

			int gridWidth = screenWidth / cellSize;
			int gridHeight = screenHeight / cellSize;

			// Wall collision
			if (showBonanza) {
				// If we go off-screen during Bonanza, wrap around
				if (newHead.x < 0) newHead.x = gridWidth - 1;
				else if (newHead.x >= gridWidth) newHead.x = 0;

				if (newHead.y < 0) newHead.y = gridHeight - 1;
				else if (newHead.y >= gridHeight) newHead.y = 0;
			}
			else {
				// Normal Wall collision (Die if outside bounds)
				if (newHead.x < 0 || newHead.y < 0 ||
					newHead.x >= gridWidth ||
					newHead.y >= gridHeight) {
					gameOver = true;
				}
			}

			// Self collision
			for (size_t i = 0; i < snake.size(); i++) {
				if (snake[i].x == newHead.x && snake[i].y == newHead.y) {
					gameOver = true;
					showBonanza = false;
				}
			}

			snake.push_front(newHead);

			bool ateFood = (newHead.x == (int)food.x && newHead.y == (int)food.y);
			if (ateFood) {
				food = GetRandomFoodPosition(snake);
				points++;

				bonanzaUsed = false;

				if (moveDelay > 0.06f) {
					moveDelay -= 0.02f;
				}

				foodColor = GetRandomColor();
				backgroundColor = GetRandomColor();
			}
			else {
				snake.pop_back();
			}
		}

		// DRAW
		BeginDrawing();
		ClearBackground(backgroundColor);

		// Grid
		for (int x = 0; x < screenWidth; x += cellSize)
			DrawLine(x, 0, x, screenHeight, Fade(BLACK, 0.1f));

		for (int y = 0; y < screenHeight; y += cellSize)
			DrawLine(0, y, screenWidth, y, Fade(BLACK, 0.1f));

		// Snake
		for (auto& seg : snake) {
			if (showBonanza)
			{
				Color rndColor = GetRandomColor();
				DrawRectangle(seg.x * cellSize, seg.y * cellSize, cellSize, cellSize, rndColor);
			}
			else {
				DrawRectangle(seg.x * cellSize, seg.y * cellSize, cellSize, cellSize, BLACK);
			}
		}

		// Points
		int textWidth = MeasureText(TextFormat("POINTS: %i", points), 30);
		DrawText(TextFormat("POINTS: %i", points), screenWidth - textWidth - 10, 10, 30, WHITE);

		// Food
		DrawRectangle(food.x * cellSize, food.y * cellSize, cellSize, cellSize, foodColor);

		// BONANZA
		if (showBonanza) {
			DrawText("BONANZA!", screenWidth / 2 - 120, screenHeight / 2, 40, GOLD);
		}

		if (gameOver) {
			DrawText("GAME OVER", screenWidth / 2 - 100, screenHeight / 2, 30, WHITE);
			DrawText("Press R to restart", screenWidth / 2 - 130, screenHeight / 2 + 40, 20, GRAY);
		}

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
