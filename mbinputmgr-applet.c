/* 
   Input Manager GNOME2 Applet

   Copyright 2003 Matthew Allum <mallum@openedhand.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include <panel-applet-gconf.h>
#include <panel-applet.h>
#include <gnome.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "config.h"
#include "mbinputmgr.h"

typedef struct GmbinpmgrData
{
  GtkWidget              *applet;
  GtkWidget              *frame;
  GtkWidget              *image;
  
  GtkWidget              *menu;
  GSList                 *menu_items;

  MBInpmgrState          *state;

  gint                    size;

} GmbinputmgrData; 

static void 
gmbinpmgr_dialog_about (BonoboUIComponent *uic,
			GmbinputmgrData   *gmbinpmgr,
			const gchar       *verbname );

static void
gmbinpmgr_applet_build_menu (GmbinputmgrData  *gmbinputmgr);

static const char Context_menu_xml [] =					       
  "<popup name=\"button3\">\n"						
  "   <menuitem name=\"About Item\" "
  "             verb=\"VerbAbout\" "
  "           _label=\"About ...\"\n" 
  "          pixtype=\"stock\" "
  "          pixname=\"gnome-stock-about\"/>\n"  
  "</popup>\n";

static const BonoboUIVerb Context_menu_verbs [] = {
  BONOBO_UI_UNSAFE_VERB ("VerbAbout", gmbinpmgr_dialog_about ),
  BONOBO_UI_VERB_END  						
};

BonoboGenericFactory *Applet_pointer;	// Pointer to the Applet

static void 
gmbinpmgr_dialog_about (BonoboUIComponent *uic,
			GmbinputmgrData   *gmbinpmgr,
			const gchar       *verbname )
{
  static GtkWidget *about = NULL;
  GdkPixbuf        *pixbuf = NULL;
  gchar            *file;

  static const gchar *authors[] =
    {
      "Matthew Allum <mallum@openedhand.com>",
      NULL
    };
    
  const char *documenters [] = {
    NULL
  };
  
  const char *translator_credits = NULL;

  if (about) 
    {
      gtk_window_set_screen (GTK_WINDOW (about),
			     gtk_widget_get_screen (gmbinpmgr->applet));
      gtk_widget_show (about);
      gtk_window_present (GTK_WINDOW (about));
      return;
    }

  about = gnome_about_new ("Input Manager", VERSION,
			   "Copyright \xc2\xa9 2003 OpenedHand Ltd.",
			   "Easily manage software input devices.",
			   authors,
			   documenters,
			   translator_credits,
			   pixbuf );

  gtk_window_set_screen (GTK_WINDOW (about),
			 gtk_widget_get_screen (gmbinpmgr->applet));

  if (pixbuf) {
    gtk_window_set_icon (GTK_WINDOW (about), pixbuf);
    g_object_unref (pixbuf);
  }

  g_signal_connect (G_OBJECT(about), "destroy",
		    (GCallback)gtk_widget_destroyed, &about);

  gtk_widget_show (about);
}

void 
menu_item_selected_cb (GtkMenuItem *menu_item,
		       GmbinputmgrData  *gmbinputmgr )
{
  /*
  gmbinputmgr->xr_current_size = (SizeID)gtk_object_get_data(GTK_OBJECT(menu_item), 
							"size_index");
  xmbinputmgr_set_config( gmbinputmgr );
  */
}


void 				/* XXX Currently unused */
applet_menu_position (GtkMenu     *menu,
		      gint        *x,
		      gint        *y,
		      gboolean    *push_in,
		      gpointer    *ptr_data )
{
  int xx, yy, rootx, rooty;
  GmbinputmgrData  *data = (GmbinputmgrData  *)ptr_data;

  gdk_window_get_root_origin(data->applet->window, &rootx, &rooty);
  gdk_window_get_origin(data->applet->window, &xx, &yy);

  *y = rooty - data->menu->allocation.height;
  *x = xx    + data->applet->allocation.x;
}

static gboolean
applet_button_release_event_cb (GtkWidget         *widget, 
				GdkEventButton    *event, 
				GmbinputmgrData   *gmbinputmgr   )
{
  if (event->button == 1) 
    {
      /* XXX should ccheck button position */

      gtk_menu_popup (GTK_MENU(gmbinputmgr->menu), 
		      NULL, NULL, 
		      NULL, 
		      gmbinputmgr, 
		      event->button,
		      event->time);
      return TRUE;
    }
  return FALSE;
}

static void
applet_change_size_cb ( PanelApplet *applet, 
                        gint         size, 
                        GmbinputmgrData  *gmbinputmgr )
{
  if (gmbinputmgr->size != size) 
    {
      GdkPixbuf *pixbuf;
      GdkPixbuf *scaled;

      gmbinputmgr->size = size;

      pixbuf = gdk_pixbuf_new_from_file (DATADIR "pixmaps/mbinputmgr.png", 
					 NULL);
      if (pixbuf) 
	{
	  scaled = gdk_pixbuf_scale_simple (pixbuf, size, size, 
					    GDK_INTERP_BILINEAR);

	  gtk_image_set_from_pixbuf (GTK_IMAGE (gmbinputmgr->image), scaled);
	  gtk_widget_show (gmbinputmgr->image);

	  g_object_unref (G_OBJECT (pixbuf));
	}
    }
}

static void
gmbinputmgr_applet_build_menu (GmbinputmgrData  *gmbinputmgr)
{
  GtkWidget    *menu_item;
  GSList        *group = NULL, *group_rotations = NULL;
  gint          i;
  gchar         tmp_buf[128];

  gmbinputmgr->menu = gtk_menu_new();
  gtk_menu_set_screen (GTK_MENU (gmbinputmgr->menu), 
		       gtk_widget_get_screen (gmbinputmgr->applet));

  for (i=0; i<gmbinputmgr->state->NMethods; i++)
    {

      menu_item = gtk_radio_menu_item_new_with_label (group, 
						      gmbinputmgr->state->Methods[i].name);
      group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menu_item));

      /*
      if (i == gmbinputmgr->xr_current_size)
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
      */

      gtk_object_set_data(GTK_OBJECT (menu_item), "method", (gpointer)&gmbinputmgr->state->Methods[i]);

      g_signal_connect (menu_item, "activate",
			G_CALLBACK (menu_item_selected_cb), gmbinputmgr );

      gtk_menu_shell_append (GTK_MENU_SHELL (gmbinputmgr->menu), menu_item);

      gmbinputmgr->menu_items 
	= g_slist_append (gmbinputmgr->menu_items, (gpointer)menu_item );

      gtk_widget_show (menu_item);
    }

  gtk_widget_show (gmbinputmgr->menu);

} 

static void 
gmbinputmgr_applet_new ( PanelApplet *applet)
{
  GtkWidget *hbox, *button;
  GmbinputmgrData   *gmbinputmgr;

  gmbinputmgr = g_new0 (GmbinputmgrData, 1);

  /* Defaults */
  gmbinputmgr->size = 0;
  gmbinputmgr->menu = NULL;

  /* Applet Widgets  */
  gmbinputmgr->frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gmbinputmgr->frame), GTK_SHADOW_NONE);
  gtk_container_add (GTK_CONTAINER (applet), gmbinputmgr->frame);
	
  hbox = gtk_hbox_new(False, 2);
  button = gtk_button_new_with_label("^");

  gmbinputmgr->image = gtk_image_new ();

  gtk_box_pack_start (GTK_BOX(hbox),
		      gmbinputmgr->image,
		      True, False, 0);

  gtk_box_pack_end (GTK_BOX(hbox),
		    button,
		    True, False, 0);

  gtk_container_add (GTK_CONTAINER (gmbinputmgr->frame), hbox);

  gmbinputmgr->applet = GTK_WIDGET (applet);

  gmbinputmgr->state = mbinpmgr_init();

  gmbinputmgr_applet_build_menu (gmbinputmgr);

  /* Signals */
  g_signal_connect (gmbinputmgr->applet, "button-release-event",
		    G_CALLBACK (applet_button_release_event_cb), gmbinputmgr );

  g_signal_connect (gmbinputmgr->applet, "change_size",
		    G_CALLBACK (applet_change_size_cb), gmbinputmgr );


  panel_applet_setup_menu ( PANEL_APPLET (applet),	
			    Context_menu_xml,         
			    Context_menu_verbs,       
			    gmbinputmgr );                  

  /* Make sure the panel image gets painted */
  applet_change_size_cb ( applet, panel_applet_get_size (applet), gmbinputmgr );

  gtk_widget_show_all(GTK_WIDGET(applet));
}

static gboolean
gmbinputmgr_applet_factory ( PanelApplet  *applet, 
                        const char   *id, 
                        gpointer      data )
{
  gmbinputmgr_applet_new(applet);		
}

PANEL_APPLET_BONOBO_FACTORY ( "OAFIID:GmbinputmgrApplet_factory", 
			      PANEL_TYPE_APPLET, 
			      "gmbinputmgr",
			      "0",
			      gmbinputmgr_applet_factory,
			      NULL ) ;

