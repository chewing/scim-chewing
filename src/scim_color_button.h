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
 * The original code is gimpfgbgeditor.h in GIMP-2.3.2.
 * Copyright (C) 2004 Michael Natterer <mitch@gimp.org>
 */

#ifndef __SCIM_COLOR_BUTTON_H__
#define __SCIM_COLOR_BUTTON_H__


#include <gtk/gtk.h>
#include <scim.h>

using namespace scim;

#define SCIM_TYPE_COLOR_BUTTON            (scim_color_button_get_type ())
#define SCIM_COLOR_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCIM_TYPE_COLOR_BUTTON, ScimColorButton))
#define SCIM_COLOR_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SCIM_TYPE_COLOR_BUTTON, ScimColorButtonClass))
#define SCIM_IS_COLOR_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCIM_TYPE_COLOR_BUTTON))
#define SCIM_IS_COLOR_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SCIM_TYPE_COLOR_BUTTON))
#define SCIM_COLOR_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SCIM_TYPE_COLOR_BUTTON, ScimColorButtonClass))


typedef struct _ScimColorButtonClass ScimColorButtonClass;
typedef struct _ScimColorButton ScimColorButton;

struct _ScimColorButton
{
  GtkDrawingArea   parent_instance;

  guchar          *render_buf;
  gint             render_buf_size;

  GdkPixbuf       *swap_icon;

  gint             rect_width;
  gint             rect_height;
  gint             click_target;

  GdkColor         fg_color;
  GdkColor	   bg_color;
};

struct _ScimColorButtonClass
{
  GtkDrawingAreaClass  parent_class;

  /*  signals  */

  void (* color_changed) (ScimColorButton  *button);
};


GType       scim_color_button_get_type    (void) G_GNUC_CONST;

GtkWidget * scim_color_button_new         (void);

gboolean    scim_color_button_get_colors  (ScimColorButton *button,
					   String *fg_value,
					   String *bg_value);
gboolean    scim_color_button_set_colors  (ScimColorButton *button,
					   const String &fg_value,
					   const String &bg_value);

#endif  /*  __SCIM_COLOR_BUTTON_H__  */
