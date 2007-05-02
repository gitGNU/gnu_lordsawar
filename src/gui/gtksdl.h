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
