#define TRACE_LENGTH 30

void lastPoint(int* x,int* y,int direction,int power)
{
	int i,j;
	Sint32 dx = spMul(spCos(direction),power);
	Sint32 dy = spMul(spSin(direction),power);
	(*x) += (10+weapon_radius[0])*spCos(direction);
	(*y) += (10+weapon_radius[0])*spSin(direction);
	for (j = 1; j < TRACE_LENGTH; j++)
		for (i = 0; i < TRACE_STEP; i++)
		{
			Sint32 d = gravitation_x((*x) >> SP_ACCURACY,(*y) >> SP_ACCURACY);
			if (d >= 0)
				d >>= PHYSIC_IMPACT;
			else
				d = -(-d >> PHYSIC_IMPACT);
			dx -= d;
			d = gravitation_y((*x) >> SP_ACCURACY,(*y) >> SP_ACCURACY);
			if (d >= 0)
				d >>= PHYSIC_IMPACT;
			else
				d = -(-d >> PHYSIC_IMPACT);
			dy -= d;
			if (circle_is_empty((*x)+dx,(*y)+dy,weapon_radius[0],NULL,1) &&
			    (*x) >= 0 && (*y) >= 0 &&
			    spFixedToInt((*x)) < LEVEL_WIDTH && spFixedToInt((*y)) < LEVEL_HEIGHT)
			{
				(*x) += dx;
				(*y) += dy;
			}
			else
				return;
		}
}

void deleteTrace(pBulletTrace bt)
{
	while (bt)
	{
		pBulletTrace next = bt->next;
		free(bt);
		bt = next;
	}
}

void addToTrace(pBulletTrace* firstTrace,Sint32 x,Sint32 y,pBullet bullet)
{
	pBulletTrace bt = (pBulletTrace)malloc(sizeof(tBulletTrace));
	bt->x = x;
	bt->y = y;
	bt->bullet = bullet;
	bt->next = *firstTrace;
	*firstTrace = bt;
}

pBulletTrace* registerTrace(pPlayer player)
{
	if (player->trace[player->tracePos])
	{
		if (player->trace[player->tracePos]->bullet)
			player->trace[player->tracePos]->bullet->trace = NULL;
		deleteTrace(player->trace[player->tracePos]);
		player->trace[player->tracePos] = NULL;
	}
	pBulletTrace* r = &(player->trace[player->tracePos]);
	player->tracePos = (player->tracePos+1)%TRACE_COUNT;
	return r;
}

void drawTrace(pPlayer player)
{
	int R = spGetRFromColor(get_border_color());
	int G = spGetGFromColor(get_border_color());
	int B = spGetBFromColor(get_border_color());
	int i;
	for (i = 0; i < TRACE_COUNT; i++)
		if (player->trace[i])
		{
			int factor = (2*TRACE_COUNT - player->tracePos + i) % TRACE_COUNT + 1;
			Uint16 color = spGetRGB(R * factor/TRACE_COUNT, G * factor/TRACE_COUNT, B*factor/TRACE_COUNT);
			pBulletTrace trace = player->trace[i];
			Sint32 ox = spMul(trace->x-posX,zoom);
			Sint32 oy = spMul(trace->y-posY,zoom);
			Sint32 x0 = screen->w/2+(spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY);
			Sint32 y0 = screen->h/2+(spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY);
			trace = trace->next;
			while (trace)
			{
				ox = spMul(trace->x-posX,zoom);
				oy = spMul(trace->y-posY,zoom);
				Sint32 x1 = screen->w/2+(spMul(ox,spCos(rotation))-spMul(oy,spSin(rotation)) >> SP_ACCURACY);
				Sint32 y1 = screen->h/2+(spMul(ox,spSin(rotation))+spMul(oy,spCos(rotation)) >> SP_ACCURACY);
				if (abs(x0-x1)+abs(y0-y1) < (LEVEL_WIDTH + LEVEL_WIDTH) / 4)
					spLine(x0,y0,0,x1,y1,0,color);
				x0 = x1;
				y0 = y1;
				trace = trace->next;
			}
		}
}

void deleteAllTraces(pPlayer player)
{
	int i;
	for (i = 0; i < TRACE_COUNT; i++)
		if (player->trace[i])
			deleteTrace(player->trace[i]);
}
