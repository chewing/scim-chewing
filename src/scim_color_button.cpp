/* 
 * Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/*
 * The original code is gimpfgbgeditor.c in GIMP-2.3.2.
 * Copyright (C) 2004 Michael Natterer <mitch@gimp.org>
 */

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#ifdef HAVE_GETTEXT
  #include <libintl.h>
  #define _(String) dgettext(GETTEXT_PACKAGE,String)
  #define N_(String) (String)
#else
  #define _(String) (String)
  #define N_(String) (String)
  #define bindtextdomain(Package,Directory)
  #define textdomain(domain)
  #define bind_textdomain_codeset(domain,codeset)
#endif

#include <string.h>

#include "scim_color_button.h"

enum
{
  PROP_0,
  PROP_ACTIVE_COLOR
};

enum
{
  COLOR_CHANGED,
  LAST_SIGNAL
};

typedef enum
{
    INVALID_AREA,
    FOREGROUND_AREA,
    BACKGROUND_AREA,
    SWAP_AREA,
    DEFAULT_AREA
} FgBgTarget;


#define WIDGET_WIDTH 32
#define WIDGET_HEIGHT 32

static void     scim_color_button_class_init (ScimChewingColorButtonClass *klass);
static void     scim_color_button_init       (ScimChewingColorButton      *object);

#if GTK_CHECK_VERSION(3, 0, 0)
static void     scim_color_button_destroy         (GtkWidget      *object);
static gboolean scim_color_button_draw            (GtkWidget      *widget, cairo_t *cr);
#else
static void     scim_color_button_destroy         (GtkObject      *object);
static gboolean scim_color_button_expose          (GtkWidget      *widget,
                                                   GdkEventExpose *eevent);
#endif
static gboolean scim_color_button_button_press    (GtkWidget      *widget,
                                                   GdkEventButton *bevent);
static gboolean scim_color_button_button_release  (GtkWidget      *widget,
                                                   GdkEventButton *bevent);


static guint  button_signals[LAST_SIGNAL] = { 0 };

static GtkDrawingAreaClass *parent_class = NULL;

GType
scim_color_button_get_type (void)
{
    static GType type = 0;

    if (!type) {
        static const GTypeInfo info = {
            sizeof (ScimChewingColorButtonClass),
            NULL,           /* base_init */
            NULL,           /* base_finalize */
            (GClassInitFunc) scim_color_button_class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (ScimChewingColorButton),
            0,              /* n_preallocs */
            (GInstanceInitFunc) scim_color_button_init,
        };

        type = g_type_register_static (GTK_TYPE_DRAWING_AREA,
                                       "ScimChewingColorButton",
                                       &info, (GTypeFlags) 0);
    }

    return type;
}

static void
scim_color_button_class_init (ScimChewingColorButtonClass *klass)
{
    GtkWidgetClass *widget_class     = GTK_WIDGET_CLASS (klass);
  
    parent_class = (GtkDrawingAreaClass *) g_type_class_peek_parent (klass);
  
    button_signals[COLOR_CHANGED] =
      g_signal_new ("color-changed",
  		  G_TYPE_FROM_CLASS (klass),
  		  G_SIGNAL_RUN_FIRST,
  		  G_STRUCT_OFFSET (ScimChewingColorButtonClass, color_changed),
  		  NULL, NULL,
  		  g_cclosure_marshal_VOID__VOID,
  		  G_TYPE_NONE, 0);
  
#if GTK_CHECK_VERSION(3, 0, 0)
    widget_class->destroy = scim_color_button_destroy;
    widget_class->draw = scim_color_button_draw;
#else
    GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (klass);
    gtk_object_class->destroy          = scim_color_button_destroy;
    widget_class->expose_event         = scim_color_button_expose;
#endif
  
    widget_class->button_press_event   = scim_color_button_button_press;
    widget_class->button_release_event = scim_color_button_button_release;
}

static void
scim_color_button_init (ScimChewingColorButton *object)
{
    /*set default color */ 
#if GTK_CHECK_VERSION(3, 0, 0)
    gdk_rgba_parse (&object->fg_color, "#000000");
    gdk_rgba_parse (&object->bg_color, "#ffffff");
#else
    gdk_color_parse ("#000000", &object->fg_color);
    gdk_color_parse ("#ffffff", &object->bg_color);
#endif

    gtk_widget_add_events (GTK_WIDGET (object),
                    GDK_BUTTON_PRESS_MASK |
                    GDK_BUTTON_RELEASE_MASK);
}


static void
#if GTK_CHECK_VERSION(3, 0, 0)
scim_color_button_destroy (GtkWidget *object)
#else
scim_color_button_destroy (GtkObject *object)
#endif
{
    ScimChewingColorButton *button = SCIM_COLOR_BUTTON (object);

    if (button->render_buf) {
        g_free (button->render_buf);
        button->render_buf = NULL;
        button->render_buf_size = 0;
    }

    if (button->swap_icon) {
        g_object_unref (button->swap_icon);
        button->swap_icon = NULL;
    }

#if GTK_CHECK_VERSION(3, 0, 0)
    GTK_WIDGET_CLASS (parent_class)->destroy (object);
#else
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
#endif
}

static void
scim_color_button_draw_rect (ScimChewingColorButton *button,
#if GTK_CHECK_VERSION(3, 0, 0)
                             GdkWindow      *drawable,
                             cairo_t        *cr,
#else
                             GdkDrawable    *drawable,
                             GdkGC          *gc,
#endif
                             gint            x,
                             gint            y,
                             gint            width,
                             gint            height,
#if GTK_CHECK_VERSION(3, 0, 0)
                             GdkRGBA         *color
#else
                             GdkColor        *color
#endif
                             )
{
#if GTK_CHECK_VERSION(3, 0, 0)
    gdk_cairo_set_source_rgba (cr, color);
    cairo_rectangle (cr, x, y, width, height);
    cairo_fill (cr);
#else
    gint    rowstride;
    guchar  r, g, b;
    gint    xx, yy;
    guchar *bp;
  
    g_return_if_fail (width > 0 && height > 0);
  
    r = (color->red >> 8);
    g = (color->green >> 8);
    b = (color->blue >> 8);
  
    rowstride = 3 * ((width + 3) & -4);
  
    if (! button->render_buf || button->render_buf_size < height * rowstride) {
        button->render_buf_size = rowstride * height;
  
        g_free (button->render_buf);
        button->render_buf = (guchar *) g_malloc (button->render_buf_size);
    }
  
    bp = button->render_buf;
    for (xx = 0; xx < width; xx++) {
        *bp++ = r;
        *bp++ = g;
        *bp++ = b;
    }
  
    bp = button->render_buf;
  
    for (yy = 1; yy < height; yy++) {
        bp += rowstride;
        memcpy (bp, button->render_buf, rowstride);
    }
  
    gdk_draw_rgb_image (drawable, gc, x, y, width, height,
                        GDK_RGB_DITHER_MAX,
                        button->render_buf,
                        rowstride);
#endif
}

static gboolean
#if GTK_CHECK_VERSION(3, 0, 0)
scim_color_button_draw (GtkWidget      *widget,
                          cairo_t *cr)
#else
scim_color_button_expose (GtkWidget      *widget,
                          GdkEventExpose *eevent)
#endif
{
    ScimChewingColorButton *button = SCIM_COLOR_BUTTON (widget);
    gint            width, height;
    gint            swap_w = 0, swap_h = 0;
    gint            rect_w, rect_h;
  
#if GTK_CHECK_VERSION(2, 18, 0)
    if (!gtk_widget_is_drawable (widget))
#else
    if (!GTK_WIDGET_DRAWABLE (widget))
#endif
        return FALSE;

#if GTK_CHECK_VERSION(3, 0, 0)
    width = gdk_window_get_width(gtk_widget_get_window (widget));
    height = gdk_window_get_height(gtk_widget_get_window (widget));
#else
    width  = widget->allocation.width;
    height = widget->allocation.height;
#endif

    /*  draw the swap colors pixbuf  */
    if (!button->swap_icon) {
        button->swap_icon
	  = gdk_pixbuf_new_from_file (SCIM_ICONDIR"/scim-chewing-swap-colors.png",
				      NULL);
    }

    if (button->swap_icon) {
      swap_w = gdk_pixbuf_get_width  (button->swap_icon);
      swap_h = gdk_pixbuf_get_height (button->swap_icon);
    }  

    if (swap_w < width / 2 && swap_h < height / 2) {
#if GTK_CHECK_VERSION(3, 0, 0)
        gdk_cairo_set_source_pixbuf (cr, button->swap_icon, width - swap_w, 0);
        cairo_paint (cr);
#else
        gdk_draw_pixbuf (widget->window, NULL, button->swap_icon,
                         0, 0, width - swap_w, 0, swap_w, swap_h,
                         GDK_RGB_DITHER_NORMAL, 0, 0);
#endif
    } else {
        swap_w = swap_h = 0;
    }

    rect_h = height - swap_h - 2;
    rect_w = width  - swap_w - 4;

    if (rect_h > (height * 3 / 4)) {
        rect_w = MAX (rect_w - (rect_h - ((height * 3 / 4))),
                      width * 2 / 3);
    }

    button->rect_width  = rect_w;
    button->rect_height = rect_h;
  
    /*  draw the background area  */
    scim_color_button_draw_rect (button,
#if GTK_CHECK_VERSION(2, 14, 0)
                                 gtk_widget_get_window (widget),
#else
                                 widget->window,
#endif
#if GTK_CHECK_VERSION(3, 0, 0)
                                 cr,
#else
                                 widget->style->fg_gc[0],
#endif
                                 (width - rect_w),
                                 (height - rect_h),
                                 rect_w, rect_h,
                                 &button->bg_color);
  
#if GTK_CHECK_VERSION(3, 0, 0)
    GtkStyleContext *style = gtk_widget_get_style_context(widget);
    gtk_style_context_save (style);
    gtk_style_context_set_state (style, GTK_STATE_FLAG_NORMAL);
    gtk_render_frame (style, cr, width - rect_w, height - rect_h, rect_w, rect_h);
#else
    gtk_paint_shadow (widget->style,
#if GTK_CHECK_VERSION(2, 14, 0)
                      gtk_widget_get_window (widget),
#else
                      widget->window,
#endif
                      GTK_STATE_NORMAL,
                      GTK_SHADOW_IN,
                      NULL, widget, NULL,
                      (width - rect_w),
                      (height - rect_h),
                      rect_w, rect_h);
#endif 
  
    /*  draw the foreground area  */
    scim_color_button_draw_rect (button,
#if GTK_CHECK_VERSION(2, 14, 0)
                                 gtk_widget_get_window (widget),
#else
                                 widget->window,
#endif
#if GTK_CHECK_VERSION(3, 0, 0)
                                 cr,
#else
                                 widget->style->fg_gc[0],
#endif
                                 0, 0,
                                 rect_w, rect_h,
                                 &button->fg_color);
    
#if GTK_CHECK_VERSION(3, 0, 0)
    style = gtk_widget_get_style_context(widget);
    gtk_style_context_save (style);
    gtk_style_context_set_state (style, GTK_STATE_FLAG_NORMAL);
    gtk_render_frame (style, cr, 0, 0, rect_w, rect_h);
#else
    gtk_paint_shadow (
                      widget->style,
#if GTK_CHECK_VERSION(2, 14, 0)
                      gtk_widget_get_window (widget),
#else
                      widget->window,
#endif
                      GTK_STATE_NORMAL,
                      GTK_SHADOW_IN,
                      NULL, widget, NULL,
                      0, 0,
                      rect_w, rect_h);
#endif
  
    return TRUE;
}

static FgBgTarget
scim_color_button_target (ScimChewingColorButton *button,
                          gint            x,
                          gint            y)
{
#if GTK_CHECK_VERSION(3, 0, 0)
    gint width = gdk_window_get_width(gtk_widget_get_window (GTK_WIDGET(button)));
    gint height = gdk_window_get_height(gtk_widget_get_window (GTK_WIDGET(button)));
#else
    gint width  = GTK_WIDGET (button)->allocation.width;
    gint height = GTK_WIDGET (button)->allocation.height;
#endif
    gint rect_w = button->rect_width;
    gint rect_h = button->rect_height;

    if (x > 0 && x < rect_w && y > 0 && y < rect_h)
        return FOREGROUND_AREA;
    else if (x > (width - rect_w)  && x < width  &&
             y > (height - rect_h) && y < height)
        return BACKGROUND_AREA;
    else if (x > 0      && x < (width - rect_w) &&
             y > rect_h && y < height)
        return DEFAULT_AREA;
    else if (x > rect_w && x < width &&
             y > 0      && y < (height - rect_h))
        return SWAP_AREA;
  
    return INVALID_AREA;
}

static void
scim_color_button_open_color_dialog (ScimChewingColorButton *button, gboolean fg)
{
    GtkWidget *dialog;
    const gchar *title;
#if GTK_CHECK_VERSION(3, 0, 0)
    GdkRGBA *color;
#else
    GdkColor *color;
#endif
    gint result;
  
    title = fg ? _("Foreground color") : _("Background color");
    color = fg ? &button->fg_color : &button->bg_color;
    
    dialog = gtk_color_selection_dialog_new (title);

#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_color_selection_set_current_rgba (
        GTK_COLOR_SELECTION (gtk_color_selection_dialog_get_color_selection (GTK_COLOR_SELECTION_DIALOG (dialog))),
        color);
#else
    gtk_color_selection_set_current_color (
#if GTK_CHECK_VERSION(2, 14, 0)
        GTK_COLOR_SELECTION (gtk_color_selection_dialog_get_color_selection (GTK_COLOR_SELECTION_DIALOG (dialog))),
#else
        GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dialog)->colorsel),
#endif
        color);
#endif
    gtk_widget_show (dialog);

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (result) {
        case GTK_RESPONSE_OK:
#if GTK_CHECK_VERSION(3, 0, 0)
            gtk_color_selection_get_current_rgba (
                GTK_COLOR_SELECTION (gtk_color_selection_dialog_get_color_selection (GTK_COLOR_SELECTION_DIALOG (dialog))),
                color);
#else
            gtk_color_selection_get_current_color (
#if GTK_CHECK_VERSION(2, 14, 0)
                GTK_COLOR_SELECTION (gtk_color_selection_dialog_get_color_selection (GTK_COLOR_SELECTION_DIALOG (dialog))),
#else
                GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dialog)->colorsel),
#endif
                color);
#endif
            g_signal_emit (button, button_signals[COLOR_CHANGED], 0);
            break;
        default:
            break;
    } 

    gtk_widget_destroy (dialog);
    gtk_widget_queue_draw (GTK_WIDGET (button));
}

static void
scim_color_button_swap_color (ScimChewingColorButton *button)
{
#if GTK_CHECK_VERSION(3, 0, 0)
    GdkRGBA tmp = button->fg_color;
    button->fg_color = button->bg_color;
    button->bg_color = tmp;
#else
    GdkColor tmp;
    tmp.red   = button->fg_color.red;
    tmp.green = button->fg_color.green;
    tmp.blue  = button->fg_color.blue;

    button->fg_color.red   = button->bg_color.red;
    button->fg_color.green = button->bg_color.green;
    button->fg_color.blue  = button->bg_color.blue;

    button->bg_color.red   = tmp.red;
    button->bg_color.green = tmp.green;
    button->bg_color.blue  = tmp.blue;
#endif
    g_signal_emit (button, button_signals[COLOR_CHANGED], 0);
}

static gboolean
scim_color_button_button_press (GtkWidget      *widget,
                                GdkEventButton *bevent)
{
    ScimChewingColorButton *button = SCIM_COLOR_BUTTON (widget);
    if (bevent->button == 1 && bevent->type == GDK_BUTTON_PRESS) {
        FgBgTarget target = scim_color_button_target (button,
                                                      (gint) bevent->x, (gint) bevent->y);
  
        button->click_target = INVALID_AREA;
  
        switch (target) {
            case FOREGROUND_AREA:
              button->click_target = FOREGROUND_AREA;
              scim_color_button_open_color_dialog (button, TRUE);
              break;
    
            case BACKGROUND_AREA:
              button->click_target = BACKGROUND_AREA;
              scim_color_button_open_color_dialog (button, FALSE);
              break;
  
            case SWAP_AREA:
              scim_color_button_swap_color (button);
              gtk_widget_queue_draw (GTK_WIDGET (button));
              break;
    
            case DEFAULT_AREA:
              break;
    
            default:
              break;
          }
      }
  
    return FALSE;
}

static gboolean
scim_color_button_button_release (GtkWidget      *widget,
                                  GdkEventButton *bevent)
{
    ScimChewingColorButton *button = SCIM_COLOR_BUTTON (widget);
  
    if (bevent->button == 1) {
        FgBgTarget target = scim_color_button_target (button,
                                                      (gint) bevent->x, (gint) bevent->y);
  
        if (target == button->click_target) {
            switch (target)
              {
              case FOREGROUND_AREA:
                break;
  
              case BACKGROUND_AREA:
                break;
  
              default:
                break;
              }
        }
  
        button->click_target = INVALID_AREA;
    }
  
    return FALSE;
}


/*  public functions  */

GtkWidget *
scim_color_button_new (void)
{
    return GTK_WIDGET(g_object_new (SCIM_TYPE_COLOR_BUTTON,
                                    NULL));
}

gboolean
scim_color_button_get_colors (ScimChewingColorButton *button,
                              String *fg_value,
                              String *bg_value)
{
    gchar fg_color_str[8], bg_color_str[8];

    g_snprintf (fg_color_str, G_N_ELEMENTS (fg_color_str),
                "#%02X%02X%02X", 
#if GTK_CHECK_VERSION(3, 0, 0)
                static_cast<unsigned int>(255*button->fg_color.red),
                static_cast<unsigned int>(255*button->fg_color.green),
                static_cast<unsigned int>(255*button->fg_color.blue)
#else
                ((button->fg_color.red)>>8),
                ((button->fg_color.green)>>8),
                ((button->fg_color.blue)>>8)
#endif
                );
    g_snprintf (bg_color_str, G_N_ELEMENTS (bg_color_str),
                "#%02X%02X%02X", 
#if GTK_CHECK_VERSION(3, 0, 0)
                static_cast<unsigned int>(255*button->bg_color.red),
                static_cast<unsigned int>(255*button->bg_color.green),
                static_cast<unsigned int>(255*button->bg_color.blue)
#else
                ((button->bg_color.red)>>8),
                ((button->bg_color.green)>>8),
                ((button->bg_color.blue)>>8)
#endif
                );

    *fg_value = String (fg_color_str); 
    *bg_value = String (bg_color_str); 

    return TRUE;
}

gboolean
scim_color_button_set_colors (ScimChewingColorButton *button,
                              const String &fg_value,
                              const String &bg_value)
{
#if GTK_CHECK_VERSION(3, 0, 0)
    gdk_rgba_parse (&button->fg_color, fg_value.c_str()); 
    gdk_rgba_parse (&button->bg_color, bg_value.c_str()); 
#else
    GdkColor fg_color, bg_color;
    gdk_color_parse (fg_value.c_str (), &fg_color);
    gdk_color_parse (bg_value.c_str (), &bg_color);
    button->fg_color.red   = fg_color.red;
    button->fg_color.green = fg_color.green;
    button->fg_color.blue  = fg_color.blue;

    button->bg_color.red   = bg_color.red;
    button->bg_color.green = bg_color.green;
    button->bg_color.blue  = bg_color.blue;
#endif

    return TRUE;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
