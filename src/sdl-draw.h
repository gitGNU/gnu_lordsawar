//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef SDL_DRAW_H
#define SDL_DRAW_H

#include <SDL_video.h>

// SDL drawing helpers, for drawing pixels and lines, all coordinate ranges are
// inclusive in both ends


// from example on libsdl.org
inline void
draw_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

inline void
draw_pixel_clipped(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    if (x < surface->clip_rect.x ||
	x >= surface->clip_rect.x + surface->clip_rect.w)
	return;

    if (y < surface->clip_rect.y ||
	y >= surface->clip_rect.y + surface->clip_rect.h)
	return;

    draw_pixel(surface, x, y, pixel);
}


inline void
draw_hline(SDL_Surface *surface, int x1, int x2, int y, Uint32 color)
{
    for (int x = x1; x <= x2; ++x)
	draw_pixel(surface, x, y, color);
}

inline void
draw_hline_clipped(SDL_Surface *surface, int x1, int x2, int y, Uint32 color)
{
    if (x1 < surface->clip_rect.x)
	x1 = surface->clip_rect.x;
    else if (x1 >= surface->clip_rect.x + surface->clip_rect.w)
	return;

    if (x2 < surface->clip_rect.x)
	return;
    else if (x2 >= surface->clip_rect.x + surface->clip_rect.w)
	x2 = surface->clip_rect.x + surface->clip_rect.w - 1;

    if (y < surface->clip_rect.y ||
	y >= surface->clip_rect.y + surface->clip_rect.h)
	return;

    draw_hline(surface, x1, x2, y, color);
}

inline void
draw_vline(SDL_Surface *surface, int x, int y1, int y2, Uint32 color)
{
    for (int y = y1; y <= y2; ++y)
	draw_pixel(surface, x, y, color);
}

inline void
draw_vline_clipped(SDL_Surface *surface, int x, int y1, int y2, Uint32 color)
{
    if (y1 < surface->clip_rect.y)
	y1 = surface->clip_rect.y;
    else if (y1 >= surface->clip_rect.y + surface->clip_rect.h)
	return;

    if (y2 < surface->clip_rect.y)
	return;
    else if (y2 >= surface->clip_rect.y + surface->clip_rect.h)
	y2 = surface->clip_rect.y + surface->clip_rect.h - 1;

    if (x < surface->clip_rect.x ||
	x >= surface->clip_rect.x + surface->clip_rect.w)
	return;

    draw_vline(surface, x, y1, y2, color);
}

inline void
draw_rect(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 color)
{
    draw_hline(surface, x1, x2, y1, color);
    draw_hline(surface, x1, x2, y2, color);
    draw_vline(surface, x1, y1 + 1, y2 - 1, color);
    draw_vline(surface, x2, y1 + 1, y2 - 1, color);
}

inline void
draw_rect_clipped(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 color)
{
    draw_hline_clipped(surface, x1, x2, y1, color);
    draw_hline_clipped(surface, x1, x2, y2, color);
    draw_vline_clipped(surface, x1, y1 + 1, y2 - 1, color);
    draw_vline_clipped(surface, x2, y1 + 1, y2 - 1, color);
}

inline void
draw_filled_rect(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 color)
{
    SDL_Rect r;
    r.x = x1;
    r.y = y1;
    r.w = x2 - x1;
    r.h = y2 - y1;
    SDL_FillRect(surface, &r, color);
}


#endif
