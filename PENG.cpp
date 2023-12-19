#include <SDL.h>
#include <chrono>
#undef main // Dette fikser unresolved external symbol main. Vettafaen koffor.
#include <stdio.h>
#include <SDL_ttf.h>
#include <string>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int BALL_WIDTH = 15;
const int BALL_HEIGHT = 15;

const int PADDLE_WIDTH = 15;
const int PADDLE_HEIGHT = 75;

const float PADDLE_SPEED = 1000.0f;
const float BALL_SPEED = 1000.0f;

Uint8 COLOR1[4] = {0xFF, 0xA5, 0x0, 0xFF};
Uint8 COLOR2[4] = {0x0, 0x0, 0x0, 0xFF };

enum Buttons
{
	PaddleOneUp = 0,
	PaddleOneDown,
	PaddleTwoUp,
	PaddleTwoDown,
};

enum class CollisionType
{
	None,
	Top,
	Middle,
	Bottom,
	Left,
	Right
};

struct Contact
{
	CollisionType type;
	float penetration;
};

class Vec2
{
public:
	Vec2()
		: x(0.0f), y(0.0f)
	{}

	Vec2(float x, float y)
		: x(x), y(y)
	{}

	Vec2 operator+(Vec2 const& rhs)
	{
		return Vec2(x + rhs.x, y + rhs.y);
	}

	Vec2& operator+=(Vec2 const& rhs)
	{
		x += rhs.x;
		y += rhs.y;

		return *this;
	}

	Vec2 operator*(float rhs)
	{
		return Vec2(x * rhs, y * rhs);
	}

	float x, y;
};

class Ball
{
public:
	Ball(Vec2 position, Vec2 velocity)
		: position(position), velocity(velocity)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = BALL_WIDTH;
		rect.h = BALL_HEIGHT;
	}

	void Update(float dt)
	{
		position += velocity * dt;
	}

	void Draw(SDL_Renderer* renderer)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);

		SDL_RenderFillRect(renderer, &rect);
	}

	void CollidewithPaddle(Contact const& contact)
	{
		position.x += contact.penetration;
		velocity.x = -velocity.x;

		if (contact.type == CollisionType::Top)
		{
			velocity.y = -0.75f * BALL_SPEED;
		}
		else if (contact.type == CollisionType::Bottom)
		{
			velocity.y = 0.75f * BALL_SPEED;
		}
	}

	void CollideWithWall(Contact const& contact)
	{
		if ((contact.type == CollisionType::Top) || (contact.type == CollisionType::Bottom))
		{
			position.y += contact.penetration;
			velocity.y = -velocity.y;
		}
		else if (contact.type == CollisionType::Left)
		{
			position.x = WINDOW_WIDTH / 2.0f;
			position.y = WINDOW_HEIGHT / 2.0f;
			velocity.x = BALL_SPEED;
			velocity.y = 0.75f * BALL_SPEED;
		}
		else if (contact.type == CollisionType::Right)
		{
			position.x = WINDOW_WIDTH / 2.0f;
			position.y = WINDOW_HEIGHT / 2.0f;
			velocity.x = -BALL_SPEED;
			velocity.y = 0.75f * BALL_SPEED;
		}
	}


	Vec2 position;
	Vec2 velocity;
	SDL_Rect rect{};
};

class Paddle
{
public:
	Paddle(Vec2 position, Vec2 velocity)
		: position(position), velocity(velocity)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = PADDLE_WIDTH;
		rect.h = PADDLE_HEIGHT;
	}
	void Update(float dt)
	{
		position += velocity * dt; // Fart * deltatime gir enheter pr sekund i stedet for per frame

		if (position.y < 0)
		{
			position.y = 0;
		}
		else if (position.y > WINDOW_HEIGHT - PADDLE_HEIGHT)
		{
			position.y = WINDOW_HEIGHT - PADDLE_HEIGHT;
		} // Begrenser racketene til topp og bunn
	}
	void Draw(SDL_Renderer* renderer)
	{
		rect.y = static_cast<int>(position.y);

		SDL_RenderFillRect(renderer, &rect);
	}

	Vec2 position;
	Vec2 velocity;
	SDL_Rect rect{};
};

class PlayerScore
{
public:
	PlayerScore(Vec2 position, SDL_Renderer* renderer, TTF_Font* font)
		: renderer(renderer), font(font)
	{
		surface = TTF_RenderText_Solid(font, "0", { COLOR1[0], COLOR1[1], COLOR1[2], COLOR1[3]});
		texture = SDL_CreateTextureFromSurface(renderer, surface);

		int width, height;
		SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = width;
		rect.h = height;
	}

	~PlayerScore()
	{
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);
	}

	void SetScore(int score)
	{
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);

		surface = TTF_RenderText_Solid(font, std::to_string(score).c_str(), { COLOR1[0], COLOR1[1], COLOR1[2], COLOR1[3]});
		texture = SDL_CreateTextureFromSurface(renderer, surface);

		int width, height;
		SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
		rect.w = width;
		rect.h = height;
	}
	void Draw()
	{
		SDL_RenderCopy(renderer, texture, nullptr, &rect);
	}

	SDL_Renderer* renderer;
	TTF_Font* font;
	SDL_Surface* surface{};
	SDL_Texture* texture{};
	SDL_Rect rect{};
};

Contact CheckPaddleCollision(Ball const& ball, Paddle const& paddle)
{
	float ballLeft = ball.position.x;
	float ballRight = ball.position.x + BALL_WIDTH;
	float ballTop = ball.position.y;
	float ballBottom = ball.position.y + BALL_HEIGHT;

	float paddleLeft = paddle.position.x;
	float paddleRight = paddle.position.x + PADDLE_WIDTH;
	float paddleTop = paddle.position.y;
	float paddleBottom = paddle.position.y + PADDLE_HEIGHT;

	Contact contact{};

	if (ballLeft >= paddleRight)
	{
		return contact;
	}

	if (ballRight <= paddleLeft)
	{
		return contact;
	}

	if (ballTop >= paddleBottom)
	{
		return contact;
	}

	if (ballBottom <= paddleTop)
	{
		return contact;
	}

	float paddleRangeUpper = paddleBottom - (2.0f * PADDLE_HEIGHT / 3.0f);
	float paddleRangeMiddle = paddleBottom - (PADDLE_HEIGHT / 3.0f);

	if (ball.velocity.x < 0)
	{
		// Left racket
		contact.penetration = paddleRight - ballLeft;
	}
	else if (ball.velocity.x > 0)
	{
		// Right racket
		contact.penetration = paddleLeft - ballRight;
	}
	if ((ballBottom > paddleTop)
		&& (ballBottom < paddleRangeMiddle))
	{
		contact.type = CollisionType::Top;
	}
	else if ((ballBottom > paddleRangeUpper)
		&& (ballBottom < paddleRangeMiddle))
	{
		contact.type = CollisionType::Middle;
	}
	else
	{
		contact.type = CollisionType::Bottom;
	}


	return contact;
}

Contact CheckWallCollision(Ball const& ball)
{
	float ballLeft = ball.position.x;
	float ballRight = ball.position.x + BALL_WIDTH;
	float ballTop = ball.position.y;
	float ballBottom = ball.position.y + BALL_HEIGHT;

	Contact contact{};

	if (ballLeft < 0.0f)
	{
		contact.type = CollisionType::Left;
	}
	else if (ballRight > WINDOW_WIDTH)
	{
		contact.type = CollisionType::Right;
	}
	else if (ballTop < 0.0f)
	{
		contact.type = CollisionType::Top;
		contact.penetration = -ballTop;
	}
	else if (ballBottom > WINDOW_HEIGHT)
	{
		contact.type = CollisionType::Bottom;
		contact.penetration = WINDOW_HEIGHT - ballBottom;
	}

	return contact;
}

int main()
{
	// Initialize SDL components
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();

	SDL_Window* window = SDL_CreateWindow("PENG", 40, 40, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	TTF_Font* scoreFont = TTF_OpenFont("square-deal.ttf", 40);

	Ball ball(
		Vec2(WINDOW_WIDTH / 2.0f, (WINDOW_HEIGHT / 2.0f) - (BALL_HEIGHT / 2.0f)),
		Vec2(BALL_SPEED, 0.0f));

	Paddle paddleOne(
		Vec2(50.0f, WINDOW_HEIGHT / 2.0f - (PADDLE_HEIGHT / 2.0f)),
		Vec2(0.0f, 0.0f));

	Paddle paddleTwo(
		Vec2(WINDOW_WIDTH - 50.0f, WINDOW_HEIGHT / 2.0f - (PADDLE_HEIGHT / 2.0f)),
		Vec2(0.0f, 0.0f));


	// Game logic
	{
		PlayerScore playerOneScoreText(Vec2(WINDOW_WIDTH / 4, 20), renderer, scoreFont);
		PlayerScore playerTwoScoreText(Vec2(3 * WINDOW_WIDTH / 4, 20), renderer, scoreFont);

		int playerOneScore = 0;
		int playerTwoScore = 0;

		bool running = true;
		bool buttons[4] = {};
		float dt = 0.0f;

		// Continue looping and processing events until user exits
		while (running)
		{
			auto startTime = std::chrono::high_resolution_clock::now();
			auto stopTime = std::chrono::high_resolution_clock::now();
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
				{
					running = false;
				}
				else if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
						running = false;
					}
					else if (event.key.keysym.sym == SDLK_w)
					{
						buttons[Buttons::PaddleOneUp] = true;
					}
					else if (event.key.keysym.sym == SDLK_s)
					{
						buttons[Buttons::PaddleOneDown] = true;
					}
					else if (event.key.keysym.sym == SDLK_UP)
					{
						buttons[Buttons::PaddleTwoUp] = true;
					}
					else if (event.key.keysym.sym == SDLK_DOWN)
					{
						buttons[Buttons::PaddleTwoDown] = true;
					}
				}
				else if (event.type == SDL_KEYUP)
				{
					if (event.key.keysym.sym == SDLK_w)
					{
						buttons[Buttons::PaddleOneUp] = false;
					}
					else if (event.key.keysym.sym == SDLK_s)
					{
						buttons[Buttons::PaddleOneDown] = false;
					}
					else if (event.key.keysym.sym == SDLK_UP)
					{
						buttons[Buttons::PaddleTwoUp] = false;
					}
					else if (event.key.keysym.sym == SDLK_DOWN)
					{
						buttons[Buttons::PaddleTwoDown] = false;
					}
				}
			}

			if (buttons[Buttons::PaddleOneUp])
			{
				paddleOne.velocity.y = -PADDLE_SPEED;
			}
			else if (buttons[Buttons::PaddleOneDown])
			{
				paddleOne.velocity.y = PADDLE_SPEED;
			}
			else
			{
				paddleOne.velocity.y = 0.0f;
			}

			if (buttons[Buttons::PaddleTwoUp])
			{
				paddleTwo.velocity.y = -PADDLE_SPEED;
			}
			else if (buttons[Buttons::PaddleTwoDown])
			{
				paddleTwo.velocity.y = PADDLE_SPEED;
			}
			else
			{
				paddleTwo.velocity.y = 0.0f;
			}
			paddleOne.Update(dt);
			paddleTwo.Update(dt);

			ball.Update(dt);

			if (Contact contact = CheckPaddleCollision(ball, paddleOne);
				contact.type != CollisionType::None)
			{
				ball.CollidewithPaddle(contact);
			}
			else if (contact = CheckPaddleCollision(ball, paddleTwo);
				contact.type != CollisionType::None)
			{
				ball.CollidewithPaddle(contact);
			}
			else if (contact = CheckWallCollision(ball);
				contact.type != CollisionType::None)
			{
				ball.CollideWithWall(contact);

				if (contact.type == CollisionType::Left)
				{
					++playerTwoScore;
					if ((COLOR1[0] == 0xFF) && (COLOR1[1] == 0xA5) && (COLOR1[2] == 0x0) && (COLOR1[3] == 0xFF))
					{
						COLOR1[0] = 0x0;
						COLOR1[1] = 0x0;
						COLOR1[2] = 0x0;
						COLOR1[3] = 0xFF;

						COLOR2[0] = 0xFF;
						COLOR2[1] = 0xA5;
						COLOR2[2] = 0x0;
						COLOR2[3] = 0xFF;
					}
					else
					{
						COLOR2[0] = 0x0;
						COLOR2[1] = 0x0;
						COLOR2[2] = 0x0;
						COLOR2[3] = 0xFF;

						COLOR1[0] = 0xFF;
						COLOR1[1] = 0xA5;
						COLOR1[2] = 0x0;
						COLOR1[3] = 0xFF;
					}
					playerOneScoreText.SetScore(playerOneScore);
					playerTwoScoreText.SetScore(playerTwoScore);
				}
				else if (contact.type == CollisionType::Right)
				{
					++playerOneScore;
					if ((COLOR1[0] == 0xFF) && (COLOR1[1] == 0xA5) && (COLOR1[2] == 0x0) && (COLOR1[3] == 0xFF))
					{
						COLOR1[0] = 0x0;
						COLOR1[1] = 0x0;
						COLOR1[2] = 0x0;
						COLOR1[3] = 0xFF;

						COLOR2[0] = 0xFF;
						COLOR2[1] = 0xA5;
						COLOR2[2] = 0x0;
						COLOR2[3] = 0xFF;
					}
					else
					{
						COLOR2[0] = 0x0;
						COLOR2[1] = 0x0;
						COLOR2[2] = 0x0;
						COLOR2[3] = 0xFF;

						COLOR1[0] = 0xFF;
						COLOR1[1] = 0xA5;
						COLOR1[2] = 0x0;
						COLOR1[3] = 0xFF;
					}

					playerOneScoreText.SetScore(playerOneScore);
					playerTwoScoreText.SetScore(playerTwoScore);
				}
			}

				// Clears the window to black
				SDL_SetRenderDrawColor(renderer, COLOR2[0], COLOR2[1], COLOR2[2], COLOR2[3]);
				SDL_RenderClear(renderer);
				SDL_SetRenderDrawColor(renderer, COLOR1[0], COLOR1[1], COLOR1[2], COLOR1[3]);

				// Draw the net
				for (int y = 0; y < WINDOW_HEIGHT; ++y)
				{
					if (y % 5 && (y + 1) % 5) // y % 5 returns 0 if y is divisible by 5, if counts that as false, everything else as true
					{
						SDL_RenderDrawPoint(renderer, WINDOW_WIDTH / 2, y);
					}
				}
				dt = std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();


				ball.Draw(renderer);
				paddleOne.Draw(renderer);
				paddleTwo.Draw(renderer);
				playerOneScoreText.Draw();
				playerTwoScoreText.Draw();

				// Presents the backbuffer
				SDL_RenderPresent(renderer);
			}
		}
	

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(scoreFont);
	TTF_Quit();
	SDL_Quit();

	return 0;
}
