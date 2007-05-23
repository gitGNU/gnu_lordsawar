#include <stdlib.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "gtksdl.h"

static void gtk_sdl_class_init		(GtkSDLClass	*klass);
static void gtk_sdl_init		(GtkSDL		*sdl);
static void gtk_sdl_destroy		(GtkObject      *object);
static void gtk_sdl_realize		(GtkWidget	*widget);
static void gtk_sdl_size_allocate	(GtkWidget	*widget,
					 GtkAllocation	*allocation);
static void gtk_sdl_attach_surface	(GtkSDL *sdl);
static gint gtk_sdl_expose (GtkWidget *widget, GdkEventExpose *event);


enum {
  GTK_SDL_SURFACE_ATTACHED,
  LAST_SIGNAL
};

static guint gtk_sdl_signals[LAST_SIGNAL] = { 0 };

static GtkWidgetClass *parent_class = NULL;

#define DEBUG_OUT if
//#define DEBUG_OUT puts

GType gtk_sdl_get_type (void)
{
  static GType sdl_type = 0;

  if (!sdl_type) {
    static const GTypeInfo sdl_info =
      {
	sizeof (GtkSDLClass),
	NULL, /* base_init */
	NULL, /* base_finalize */
	(GClassInitFunc) gtk_sdl_class_init,
	NULL, /* class_finalize */
	NULL, /* class_data */
	sizeof (GtkSDL),
	0,    /* n_preallocs */			
	(GInstanceInitFunc) gtk_sdl_init,
      };
		
    sdl_type = g_type_register_static (GTK_TYPE_WIDGET,
				       "GtkSDL",
				       &sdl_info,
				       0);
  }

  return sdl_type;
}

static void gtk_sdl_class_init (GtkSDLClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  DEBUG_OUT ("before class init");

  widget_class = GTK_WIDGET_CLASS (class);
  object_class = GTK_OBJECT_CLASS (class);
  
  parent_class = gtk_type_class (gtk_widget_get_type ());
  
  object_class->destroy = gtk_sdl_destroy;
  
  widget_class->realize = gtk_sdl_realize;
  widget_class->size_allocate = gtk_sdl_size_allocate;
  widget_class->expose_event = gtk_sdl_expose;

  gtk_sdl_signals[GTK_SDL_SURFACE_ATTACHED] =
    g_signal_new ("surface-attached",
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (GtkSDLClass, surface_attached),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  DEBUG_OUT ("after class init");
}

static void gtk_sdl_init (GtkSDL *sdl)
{
  DEBUG_OUT ("before sdl init");

  gtk_widget_set_app_paintable(GTK_WIDGET(sdl), TRUE);

  DEBUG_OUT ("after sdl init");
}

GtkWidget *gtk_sdl_new (gint width, gint height, gint bpp, Uint32 flags)
{
  GtkSDL *sdl;

  DEBUG_OUT ("before sdl new");

  sdl = g_object_new (GTK_SDL_TYPE, NULL);

  sdl->bpp = bpp;
  sdl->flags = flags;

  gtk_widget_set_size_request (GTK_WIDGET(sdl), width, height);
  
  DEBUG_OUT ("after sdl new");

  return GTK_WIDGET (sdl);
}

static void gtk_sdl_destroy (GtkObject *object)
{
  GtkSDL *sdl;
  
  sdl = GTK_SDL(object);

  SDL_QuitSubSystem (SDL_INIT_VIDEO);
  
  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void gtk_sdl_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SDL (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED (widget)) {
    gdk_window_move_resize (widget->window,
			    allocation->x, allocation->y,
			    allocation->width, allocation->height);

    gtk_sdl_attach_surface (GTK_SDL(widget));
  }
}

static void gtk_sdl_realize (GtkWidget *widget)
{
  GtkSDL *sdl;
  GdkWindowAttr attributes;
  gint attributes_mask;

  DEBUG_OUT ("before sdl realize");

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_SDL (widget));

  sdl = GTK_SDL (widget);
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				   &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, sdl);

  /* set background */
  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);

  gtk_widget_set_double_buffered(widget, FALSE);

  DEBUG_OUT ("after sdl realize");
}

static void gtk_sdl_attach_surface (GtkSDL *sdl)
{
#define SDL_WINDOWHACK_BUFFER_SIZE 64
  
  gchar SDL_windowhack[SDL_WINDOWHACK_BUFFER_SIZE];
  GtkWidget *widget;

  DEBUG_OUT ("before sdl surface attach");

  SDL_QuitSubSystem (SDL_INIT_VIDEO);
  
  widget = GTK_WIDGET(sdl);

  snprintf (SDL_windowhack, SDL_WINDOWHACK_BUFFER_SIZE, "%ld",
	    GDK_WINDOW_XWINDOW (GTK_WIDGET (sdl)->window));
  DEBUG_OUT (SDL_windowhack);
  setenv ("SDL_WINDOWID", SDL_windowhack, 1);

  if (SDL_InitSubSystem (SDL_INIT_VIDEO) < 0) {
    fprintf (stderr, "unable to init SDL: %s", SDL_GetError() );
    return;
  }

  /* FIXME: what's this code for? */
#if 0
  if (sdl->flags &= (SDL_OPENGLBLIT | SDL_DOUBLEBUF)) {
    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
  }
#endif

  sdl->surface = SDL_SetVideoMode (widget->allocation.width,
				   widget->allocation.height,
				   sdl->bpp, sdl->flags);
  if (!sdl->surface) {
    fprintf (stderr, "Unable to set the video mode: %s", SDL_GetError() );
    return;
  }

  g_signal_emit (G_OBJECT (sdl), 
		 gtk_sdl_signals[GTK_SDL_SURFACE_ATTACHED], 0);  
  DEBUG_OUT ("after sdl surface attach");
}

typedef struct 
{
    SDL_Surface *surface;
    int x, y, w, h;
} ExposeData;

static gboolean sdl_update_rect (ExposeData *data)
{
  SDL_UpdateRect (data->surface, data->x, data->y, data->w, data->h);
#if 0
  fprintf (stderr, "expose %d, %d  %dx%d\n", data->x, data->y,
	   data->w, data->h);
#endif
  free (data);
  return FALSE;
}

static gint gtk_sdl_expose (GtkWidget *widget, GdkEventExpose *event)
{
  SDL_Surface *surface;
  ExposeData *expose_data;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_SDL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

#if 0
  if (event->count > 0)
    return FALSE;
#endif
  
  surface = SDL_GetVideoSurface ();

  if (!surface)
      gtk_sdl_attach_surface(GTK_SDL(widget));

  if (!surface)  
    return FALSE;

  /* put the actual exposure at the back of the event queue to fix an annoying
     bug */
  expose_data = malloc (sizeof(ExposeData));
  expose_data->surface = surface;
  expose_data->x = event->area.x;
  expose_data->y = event->area.y;
  expose_data->w = event->area.width;
  expose_data->h = event->area.height;
  
  g_idle_add ((GSourceFunc) sdl_update_rect, expose_data);

#if 0
  SDL_UpdateRect (surface,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  //SDL_SaveBMP(surface, "tmp.bmp");
  
  fprintf(stderr, "expose %d, %d  %dx%d\n", event->area.x, event->area.y,
	  event->area.width, event->area.height);
#endif
  
  return FALSE;
}
