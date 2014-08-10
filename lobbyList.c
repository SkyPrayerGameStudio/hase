#include "lobbyList.h"
#include "client.h"
#include "level.h"
#include "window.h"

SDL_Surface* ll_surface;
SDL_Surface* ll_level = NULL;
spFontPointer ll_font;
int ll_counter;
pGame ll_game_list;
int ll_game_count;
int ll_scroll;
int ll_selected;
pPlayer ll_player_list = NULL;
spTextBlockPointer ll_block = NULL;
void ( *ll_resize )( Uint16 w, Uint16 h );
char ll_game_name[33] = "New game";
int ll_game_players = 8;
int ll_game_seconds = 30;
int ll_game_hares = 5;

pGame mom_game;

void update_ll_surface()
{
	spSelectRenderTarget(ll_surface);
	spClearTarget(LL_FG);
	int pos = 0;
	spFontDraw( 2                    , pos*ll_font->maxheight-ll_scroll/1024, 0, "Name", ll_font );
	spFontDraw( 2+ 7*ll_surface->w/16, pos*ll_font->maxheight-ll_scroll/1024, 0, "Mom Pl.", ll_font );
	spFontDraw( 2+10*ll_surface->w/16, pos*ll_font->maxheight-ll_scroll/1024, 0, "Max Pl.", ll_font );
	spFontDraw( 2+13*ll_surface->w/16, pos*ll_font->maxheight-ll_scroll/1024, 0, "Status", ll_font );
	pos++;
	spLine(2,1+pos*ll_font->maxheight-ll_scroll/1024, 0, ll_surface->w-2,1+pos*ll_font->maxheight-ll_scroll/1024, 0, 65535);
	pGame game = ll_game_list;
	mom_game = NULL;
	while (game)
	{
		if (pos-1 == ll_selected)
		{
			spRectangle(ll_surface->w/2, 2+(2*pos+1)*ll_font->maxheight/2-ll_scroll/1024,0, ll_surface->w-4,ll_font->maxheight,LL_BG);
			mom_game = game;
		}
		spFontDraw( 2                    , 2+pos*ll_font->maxheight-ll_scroll/1024, 0, game->name, ll_font );
		char buffer[16];
		sprintf(buffer,"%i",game->player_count);
		spFontDraw( 2+ 7*ll_surface->w/16, 2+pos*ll_font->maxheight-ll_scroll/1024, 0, buffer, ll_font );
		sprintf(buffer,"%i",game->max_player);
		spFontDraw( 2+10*ll_surface->w/16, 2+pos*ll_font->maxheight-ll_scroll/1024, 0, buffer, ll_font );
		switch (game->status)
		{
			case  1: sprintf(buffer,"Running"); break;
			case -1: sprintf(buffer,"Done"); break;
			default: sprintf(buffer,"Open");
		}
		spFontDraw( 2+13*ll_surface->w/16, 2+pos*ll_font->maxheight-ll_scroll/1024, 0, buffer, ll_font );
		game = game->next;
		pos++;
	}
	spSelectRenderTarget(spGetWindowSurface());
}

int ll_reload_now = 0;

void ll_draw(void)
{
	update_ll_surface();
	SDL_Surface* screen = spGetWindowSurface();
	spClearTarget(LL_BG);
	char buffer[256];
	spFontDrawMiddle( screen->w/2, 0*ll_font->maxheight, 0, "Hase Lobby", ll_font );
	
	sprintf(buffer,"%i Games on Server:\n",ll_game_count);
	spFontDrawMiddle( screen->w/3+2, 1*ll_font->maxheight, 0, buffer, ll_font );
	spBlitSurface(screen->w/3,screen->h/2+ll_font->maxheight/2,0,ll_surface);
	spFontDrawMiddle(5*screen->w/6+4, 1*ll_font->maxheight, 0, "Preview", ll_font );
	spRectangle(5*screen->w/6, 2*ll_font->maxheight+screen->w/6-4, 0,screen->w/3-6,screen->w/3-6,LL_FG);
	if (ll_level)
		spBlitSurface(5*screen->w/6, 2*ll_font->maxheight+screen->w/6-4, 0,ll_level);
	spFontDrawMiddle(5*screen->w/6+4, 2*ll_font->maxheight+screen->w/3-6, 0, "Players", ll_font );
	int h = screen->h-(screen->w/3+4*ll_font->maxheight-4);
	spRectangle(5*screen->w/6, screen->h-1*ll_font->maxheight-h/2, 0,screen->w/3-6,h,LL_FG);
	spFontDrawTextBlock(middle,4*screen->w/6+6, screen->h-1*ll_font->maxheight-h, 0,
		ll_block,h,0,ll_font);
	if (ll_reload_now)
	{
		spFontDrawMiddle( screen->w/2, screen->h-ll_font->maxheight, 0, "Reloading list...", ll_font );
		ll_reload_now = 2;
	}
	else
	{
		if (mom_game && mom_game->status == -1)
			spFontDraw( 2, screen->h-ll_font->maxheight, 0, "[o]Replay   [3]Create   [R]/[B]Back", ll_font );
		else
			spFontDraw( 2, screen->h-ll_font->maxheight, 0, "[o]Join   [3]Create   [R]/[B]Back", ll_font );
		sprintf(buffer,"Next update: %is",(10000-ll_counter)/1000);
		spFontDrawRight( screen->w-2, screen->h-ll_font->maxheight, 0, buffer, ll_font );
	}
	spFlip();
}

int ll_wait;
#define MAX_WAIT 300
#define MIN_WAIT 50

int create_game_feedback( pWindowElement elem, int action )
{
	switch (action)
	{
		case WN_ACT_LEFT:
			switch (elem->reference)
			{
				case 1:
					if (ll_game_players > 2)
						ll_game_players--;
					break;
				case 2:
					if (ll_game_seconds > 5)
						ll_game_seconds -= 5;
					break;
				case 3:
					if (ll_game_hares > 1)
						ll_game_hares--;
					break;
			}
			break;
		case WN_ACT_RIGHT:
			switch (elem->reference)
			{
				case 1:
					ll_game_players++;
					break;
				case 2:
					ll_game_seconds += 5;
					break;
				case 3:
					ll_game_hares++;
					break;
			}
			break;
		case WN_ACT_START_POLL:
			spPollKeyboardInput(ll_game_name,32,KEY_POLL_MASK);
			break;
		case WN_ACT_END_POLL:
			spStopKeyboardInput();
			break;
	}
	switch (elem->reference)
	{
		case 0: sprintf(elem->text,"Name: %s",ll_game_name); break;
		case 1: sprintf(elem->text,"Maximum players: %i",ll_game_players); break;
		case 2: sprintf(elem->text,"Seconds per turn: %i",ll_game_seconds); break;
		case 3: sprintf(elem->text,"Hares per player: %i",ll_game_players); break;
	}
	return 0;
}

int ll_calc(Uint32 steps)
{
	if (spGetInput()->button[MY_BUTTON_SELECT] ||
		spGetInput()->button[MY_BUTTON_START])
		return 1;
	if (spGetInput()->button[MY_PRACTICE_3])
	{
		spGetInput()->button[MY_PRACTICE_3] = 0;
		int res = 1;
		while (res == 1)
		{
			pWindow window = create_window(create_game_feedback,ll_font,"Create game");
			add_window_element(window,1,0);
			add_window_element(window,0,1);
			add_window_element(window,0,2);
			add_window_element(window,0,3);
			res = modal_window(window,ll_resize);
			delete_window(window);
			if (res == 1 && ll_game_name[0] == 0)
				message_box(ll_font,ll_resize,"Please enter a game name");
			else
				break;
		}
		if (res == 1)
		{
			char buffer[512];
			pGame game = create_game(ll_game_name,ll_game_players,ll_game_seconds,create_level_string(buffer,1536,1536,3,3,3),0,ll_game_hares);
			start_lobby_game(ll_font,ll_resize,game);
			delete_game(game);
			ll_counter = 10000;			
		}
		ll_counter = 10000;
	}		
	if (spGetInput()->button[MY_PRACTICE_OK])
	{
		spGetInput()->button[MY_PRACTICE_OK] = 0;
		if (ll_game_count <= 0)
			message_box(ll_font,ll_resize,"No game to join!");
		else
		{
			pGame game = ll_game_list;
			int pos = 0;
			while (game)
			{
				if (pos == ll_selected)
					break;
				game = game->next;
				pos++;
			}
			if (game->status == 1)
				message_box(ll_font,ll_resize,"Game already started!");
			else
			if (game->status == -1) //Replay!
				hase(ll_resize,game,NULL);
			else
			if (game->player_count >= game->max_player)
				message_box(ll_font,ll_resize,"Game full!");
			else
			{
				start_lobby_game(ll_font,ll_resize,game);
				ll_counter = 10000;
			}
		}
	}
	if (ll_reload_now == 2)
	{
		int a = SDL_GetTicks();
		if (ll_reload())
			return 1;
		int b = SDL_GetTicks();
		ll_reload_now = 0;
		ll_counter = a-b;
	}
	int step;
	for (step = 0; step < steps; step++)
	{
		ll_counter++;
		if (ll_wait > 0)
			ll_wait--;
		if (spGetInput()->axis[1] < 0)
		{
			if (ll_wait == -1)
			{
				
				ll_wait = MAX_WAIT;
				ll_selected--;
				if (ll_level)
				{
					spDeleteSurface(ll_level);
					spDeleteTextBlock(ll_block);
					ll_level = NULL;
					ll_block = NULL;
				}
			}
			else
			if (ll_wait == 0)
			{
				ll_wait = MIN_WAIT;
				ll_selected--;
				if (ll_level)
				{
					spDeleteSurface(ll_level);
					spDeleteTextBlock(ll_block);
					ll_level = NULL;
					ll_block = NULL;
				}
			}
		}
		else
		if (spGetInput()->axis[1] > 0)
		{
			if (ll_wait == -1)
			{	
				ll_wait = MAX_WAIT;
				ll_selected++;
				if (ll_level)
				{
					spDeleteSurface(ll_level);
					spDeleteTextBlock(ll_block);
					ll_level = NULL;
					ll_block = NULL;
				}
			}
			else
			if (ll_wait == 0)
			{
				
				ll_wait = MIN_WAIT;
				ll_selected++;
				if (ll_level)
				{
					spDeleteSurface(ll_level);
					spDeleteTextBlock(ll_block);
					ll_level = NULL;
					ll_block = NULL;
				}
			}
		}
		else
			ll_wait = -1;
	}
	if (ll_counter >= 10000)
		ll_reload_now = 1;
	int total_height = (ll_game_count+1)*ll_font->maxheight+2;
	if (ll_game_count > 1)
		ll_scroll = (total_height-ll_surface->h)*1024*ll_selected/(ll_game_count-1);
	if (ll_scroll/1024+ll_surface->h > total_height)
		ll_scroll = spMax(total_height-ll_surface->h,0)*1024;
	if (ll_scroll < 0)
		ll_scroll = 0;
	if (ll_selected < 0)
		ll_selected = 0;
	if (ll_selected >= ll_game_count)
		ll_selected = ll_game_count-1;
	if (ll_wait <0 && ll_selected >= 0 && ll_level == NULL)
	{
		pGame game = ll_game_list;
		int pos = 0;
		while (game)
		{
			if (pos == ll_selected)
				break;
			game = game->next;
			pos++;
		}
		if (game)
		{
			get_game(game,&ll_player_list);
			char temp[4096] = "";
			pPlayer player = ll_player_list;
			while (player)
			{
				add_to_string(temp,player->name);
				player = player->next;
				if (player)
					add_to_string(temp,", ");
			}
			ll_block = spCreateTextBlock(temp,spGetWindowSurface()->w/3-6,ll_font);
			ll_level = create_level(game->level_string,spGetWindowSurface()->w/3-6,spGetWindowSurface()->w/3-6,LL_BG);
			//texturize_level(ll_level,game->level_string);
		}
	}
	return 0;
}

int ll_reload()
{
	if (ll_level)
	{
		spDeleteSurface(ll_level);
		spDeleteTextBlock(ll_block);
		ll_level = NULL;
		ll_block = NULL;
	}
	if (connect_to_server())
	{
		message_box(ll_font,ll_resize,"No Connection to server!");
		return 1;
	}
	if (server_info() > LL_VERSION)
	{
		message_box(ll_font,ll_resize,"Your version is too old for\nonline games. Please update!");
		return 1;
	}
	ll_game_count = get_games(&ll_game_list);
	if (ll_game_count == 0)
		ll_selected = -1;
	return 0;
}


void start_lobby(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ))
{
	ll_selected = 0;
	ll_font = font;
	ll_counter = 10000;//Instead reload
	ll_surface = spCreateSurface(2*spGetWindowSurface()->w/3-4,spGetWindowSurface()->h-3*font->maxheight);
	ll_resize = resize;
	ll_reload_now = 0;
	spLoop(ll_draw,ll_calc,10,resize,NULL);
	spDeleteSurface(ll_surface);
	if (ll_level)
		spDeleteSurface(ll_level);
	if (ll_block)
		spDeleteTextBlock(ll_block);
}
