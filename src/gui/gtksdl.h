//  Copyright (C) 2007, Ole Laursen
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef __GTK_SDL_H__
#define __GTK_SDL_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkwidget.h>

#include <SDL.h>

G_BEGIN_DECLS

#define GTK_SDL_TYPE		(gtk_sdl_get_type ())
#define GTK_SDL(obj)		(GTK_CHECK_CAST ((obj), GTK_SDL_TYPE, GtkSDL))
#define GTK_SDL_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_SDL, GtkSDLClass))
#define GTK_IS_SDL(obj)		(GTK_CHECK_TYPE ((obj), GTK_SDL_TYPE))
#define GTK_IS_SDL_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GTK_SDL_TYPE))


typedef struct _GtkSDL		GtkSDL;
typedef struct _GtkSDLClass	GtkSDLClass;

struct _GtkSDL
{
  GtkWidget widget;

  SDL_Surface *surface;
  gint bpp;
  Uint32 flags;
};

struct _GtkSDLClass
{
  GtkWidgetClass parent_class;

  void (* surface_attached) (GtkSDL *sdl);
};


GtkWidget *gtk_sdl_new (gint width, gint height, gint bpp, Uint32 flags);
GType gtk_sdl_get_type (void);
//void gtk_sdl_size (GtkSDL *sdl, gint width, gint height);

G_END_DECLS


#endif /* __GTK_SDL_H__ */
