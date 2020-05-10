#include "menu.h"

#include <stdbool.h>

#include <SDL/SDL_keysym.h>

#include "input.h"
#include "ghost.h"
#include "renderer.h"

//time till ghost-rows start appearing
#define GHOST_START 1000

//time between each ghost-row appearance
#define GHOST_BETWEEN 2000

static void draw_vanity_screen(MenuSystem *menuSystem);
static void draw_remote_choice_screen(MenuSystem *menuSystem);
static void draw_remote_server_screen(MenuSystem *menuSystem);
static void draw_remote_client_connect_screen(MenuSystem *menuSystem);
static void draw_info_screen(MenuSystem *menuSystem);

static void draw_ghost_line(GhostDisplayRow *row, int y, unsigned int dt);
static void draw_player_info(void);
static void draw_mode_choice(MenuSystem *menuSystem);

static GhostDisplayRow enemyRows[4] = {
	{Blinky, "-SHADOW", "\"BLINKY\"", RedText},
	{Pinky, "-SPEEDY", "\"PINKY\"", PinkText},
	{Inky, "-BASHFUL", "\"INKY\"", CyanText},
	{Clyde, "-POKEY", "\"CLYDE\"", OrangeText}};

void menu_init(MenuSystem *menuSystem)
{
	//set to be in solo play
	menuSystem->mode = SoloState;
	menuSystem->role = Nothing;
	menuSystem->action = Nothing;
	menuSystem->ticksSinceModeChange = SDL_GetTicks();
	menuSystem->severIP = (char *)malloc(sizeof(char) * 50);

	menuSystem->explainPage = 0; //2020 ADD
	menuSystem->gameMode = coorperate;
	for (int i = 0; i < 20; i++)
		menuSystem->severIP[i] = NULL;
}

void menu_tick(MenuSystem *menuSystem)
{
	bool startNew = key_held(SDLK_KP_ENTER) || key_held(SDLK_RETURN);

	if (startNew)
	{
		printf(" action : %d\n", menuSystem->action);
		if (menuSystem->mode == RemoteState)
			menuSystem->action = ReadyConnect;
		else if (menuSystem->mode == ExplainState)
			menuSystem->action = ExplainGame; //2020 ADD
		else
			menuSystem->action = GoToGame;

		handle_keyup(SDLK_KP_ENTER);
		handle_keyup(SDLK_RETURN);
	}
}

void remote_tick(MenuSystem *menuSystem, Socket_value *socket_info, int *state)
{
	bool startNew = key_held(SDLK_KP_ENTER) || key_held(SDLK_RETURN);

	if (startNew)
	{
		if (menuSystem->action == ReadyConnect)
		{
			if (menuSystem->role == Server)
			{
				// menuSystem->action = ServerWait;

				// init_server(socket_info);
				// printf("comeon");
				*state = MakeGameRoom;
				menuSystem->action = MakeGame;
			}
			else if (menuSystem->role == Client)
			{
				menuSystem->action = ConnectClient;
			}
		}
		else if (menuSystem->action == ConnectClient)
		{
			// client socket 초기화
			// client가 server와 연결시도

			if (connect_client(socket_info, menuSystem->severIP) == -1)
				for (int i = 0; i < 20; i++)
					menuSystem->severIP[i] = NULL;
			else
				menuSystem->action = GoToGame;
		}

		handle_keyup(SDLK_KP_ENTER);
		handle_keyup(SDLK_RETURN);
	}
}

void menu_render(MenuSystem *menuSystem)
{
	if (num_credits() == 0)
		draw_vanity_screen(menuSystem);
	else
		draw_info_screen(menuSystem);
}

void remote_render(MenuSystem *menuSystem)
{
	if (menuSystem->action == ServerWait)
		draw_remote_server_screen(menuSystem);
	else if (menuSystem->action == ConnectClient)
		draw_remote_client_connect_screen(menuSystem);
	else if (menuSystem->action == ReadyConnect)
		draw_remote_choice_screen(menuSystem);
}

static void draw_vanity_screen(MenuSystem *menuSystem)
{
	unsigned int dt = SDL_GetTicks() - menuSystem->ticksSinceModeChange;

	draw_player_info();
	draw_vanity_charnickname();
	draw_mode_choice(menuSystem);

	for (int i = 0; i < 4; i++)
	{
		unsigned int current = GHOST_START + i * GHOST_BETWEEN;
		if (dt < current)
			break;

		GhostDisplayRow r = enemyRows[i];
		draw_ghost_line(&r, 14 + 3 * i, dt - current);
	}

	if (dt > 9500)
		draw_vanity_pellet_info(false);
	if (dt > 10500)
		draw_vanity_corporate_info();
	if (dt > 11500)
		draw_vanity_animation(dt - 11500);
}

static void draw_remote_choice_screen(MenuSystem *menuSystem)
{
	draw_player_info();

	if (menuSystem->role == None)
		draw_common_indicator(Server, 4, 12);
	else
		draw_common_indicator(menuSystem->role, 4, 12);
	draw_vanity_text("MAKE GAME", 7, 17);
	draw_vanity_text("JOIN GAME", 7, 19);
}

static void draw_remote_server_screen(MenuSystem *menuSystem)
{
	//unsigned int dt = SDL_GetTicks() - menuSystem->ticksSinceModeChange;

	draw_player_info();

	draw_vanity_text("WAIT TO CONNECT...", 6, 17);

	draw_vanity_text("@ PRESS ESC TO EXIT", 6, 25);
	//if (dt%1900 > 400) draw_vanity_text(".", 18, 17);
	//if (dt%1900 > 900) draw_vanity_text(".", 19, 17);
	//if (dt%1900 > 1400) draw_vanity_text(".", 20, 17);
}

static void draw_remote_client_connect_screen(MenuSystem *menuSystem)
{
	unsigned int dt = SDL_GetTicks() - menuSystem->ticksSinceModeChange;

	draw_player_info();

	draw_vanity_text("WRITE SERVER IP", 6, 15);
	if (dt % 800 > 400)
		draw_vanity_text("- ", 4, 17);
	draw_vanity_text(menuSystem->severIP, 6, 17);
}

static void draw_info_screen(MenuSystem *menuSystem)
{
	draw_player_info();
	draw_mode_choice(menuSystem);

	draw_instrc_info();
	draw_instrc_corporate_info();
}

static void draw_player_info(void)
{
	draw_common_oneup(false, 0);
	draw_common_twoup(false, 0);
	draw_common_highscore(0);

	draw_credits(num_credits());
}

static void draw_mode_choice(MenuSystem *menuSystem)
{
	draw_common_indicator(menuSystem->mode, 6, 1);
	draw_vanity_text("PLAY ONE", 9, 4);
	draw_vanity_text("PLAY TWO", 9, 6);
	draw_vanity_text("PLAY MULTI", 9, 8);
	draw_vanity_text("GAME INFO", 9, 10); //2020 ADD
}

static void draw_ghost_line(GhostDisplayRow *row, int y, unsigned int dt)
{
	bool drawDescription = dt > 1000;
	bool drawName = dt > 1500;
	draw_vanity_ghostline(row, y, drawDescription, drawName);
}

//2020 ADD
void draw_explain_screen(MenuSystem *menuSystem)
{
	if (menuSystem->explainPage == 0)
	{
		draw_vanity_text("GAME ", 7, 3);

		draw_vanity_text("THE PACMAN HAVE TO EAT", 1, 7);
		draw_vanity_text("PELLETS OF COOKIE WITHIN", 1, 9);
		draw_vanity_text("THE NUMBER OF LIVES", 1, 11);
		draw_vanity_text("AVOIDING GHOSTS !!", 2, 15);
		draw_vanity_text("PLAYER 1", 1, 19);
		draw_vanity_text("@ UP @ DOWN @ LEFT @ RIGHT", 1, 21);
		draw_vanity_text("PLAYER 2", 1, 23);
		draw_vanity_text("@ W @ S @ A @ D", 1, 25);
		draw_vanity_text("CONTROLL CAREFULLY!", 1, 27);
		draw_vanity_text("PACMAN KEEPS GOING!", 1, 29);

		draw_vanity_text("@ PRESS NEXT", 6, 33);
	}
	else if (menuSystem->explainPage == 1)
	{
		draw_vanity_text("EXPLAIN TIP", 7, 3);

		draw_vanity_text("BIG PELLET OF COOKIE", 1, 7);
		draw_vanity_text("CAN ALLOW THE PACMAN", 2, 9);
		draw_vanity_text("TO IGNORE THE GHOSTS", 3, 11);
		draw_vanity_text("THE FRIUT HAS SPECIAL SCORE", 1, 15);
		draw_vanity_text("WE PREPARED SOME ITEMS", 1, 17);
		draw_vanity_text("USE WISELY!", 1, 19);

		draw_vanity_text("IN MULTI MODE !", 7, 23);
		draw_vanity_text("YOU MAY COMPETE A PARTNER", 1, 27);

		draw_vanity_text("@ PRESS GOTO MENU", 6, 33);
	}
}

void explain_tick()
{
}

void draw_checkquit_screen(int check)
{
	draw_common_indicator(check, 6, 1);
	draw_vanity_text("QUIT", 9, 4);
	draw_vanity_text("MENU", 9, 6);
	draw_vanity_text("BACK", 9, 8);
}

void check_quit_tick(int *check, int *state, int *beforeState, int *stopFlag, MenuSystem *menuSys)
{
	bool startNew = key_held(SDLK_KP_ENTER) || key_held(SDLK_RETURN);

	if (startNew)
	{
		//Consider Display 0~2
		if (*check == 0) // NONE(0) -> QUIT(3)
		{
			*check = 3;
		}
		if (*check == 1) // GoMenu
		{
			menu_init(menuSys);
			*beforeState = Menu;
			*check = 2;
		}
		if (*check == 2) // BACK
		{
			*state = *beforeState;
			*stopFlag = 0;
			*check = 0;
		}

		handle_keyup(SDLK_KP_ENTER);
		handle_keyup(SDLK_RETURN);
	}
}

void draw_makegame_screen(MenuSystem *menuSystem)
{
	draw_common_indicator(menuSystem->gameMode, 6, 1);
	draw_vanity_text("COOPERATE MODE", 7, 4);
	draw_vanity_text("CHASE MODE", 7, 6);
}

void makegame_tick(MenuSystem *menuSystem, Socket_value *socket_info, int *state)
{
	bool startNew = key_held(SDLK_KP_ENTER) || key_held(SDLK_RETURN);

	if (startNew)
	{
		if (menuSystem->gameMode == coorperate)
		{ // base game
			menuSystem->action = ServerWait;
			*state = Remote;
			init_server(socket_info);
			printf("comeon");
		}
		else
		{ //chase game
			printf("test game menu");
		}

		handle_keyup(SDLK_KP_ENTER);
		handle_keyup(SDLK_RETURN);
	}
}