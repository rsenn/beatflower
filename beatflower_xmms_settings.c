/* BeatFlower - a XMMS Visualization Plug-In

 Copyright (C) 2002, Roman Senn.
 Email: <smoli@paranoya.ch>
 Project Homepage: <http://www.blah.ch/beatflower>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.
 If not, write to the Free Software Foundation,
 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 $Id: beatflower.c,v 1.4 2004/05/18 23:42:16 smoli Exp $ */

#include <gtk/gtk.h>

#include "beatflower.h"
#include "beatflower_xmms.h"

GtkWidget *beatflower_xmms_settings_win = NULL;
static beatflower_config_t     beatflower_newconfig;


static void on_fullscreen_checkbutton_clicked(GtkCheckButton *checkbutton);
static void on_width_spinbutton_changed(GtkSpinButton *spinbutton);
static void on_height_spinbutton_changed(GtkSpinButton *spinbutton);
static void on_mode_option_changed(GtkMenuItem *item, GtkMenu *menu);
static void on_draw_option_changed(GtkMenuItem *item, GtkMenu *menu);
static void on_samples_option_changed(GtkMenuItem *item, GtkMenu *menu);
static void on_amplification_option_changed(GtkMenuItem *item, GtkMenu *menu);
static void on_offset_option_changed(GtkMenuItem *item, GtkMenu *menu);
static void on_blur_checkbutton_clicked(GtkCheckButton *checkbutton);
static void on_apply_button_clicked(GtkButton *button);
static void on_cancel_button_clicked(GtkButton *button);
static void on_ok_button_clicked(GtkButton *button);
//static void on_decay_spinbutton_changed(GtkSpinButton *spinbutton);
static void on_zoom_checkbutton_clicked(GtkCheckButton *checkbutton);
static void on_factor_spinbutton_changed(GtkSpinButton *spinbutton);
static void on_rotate_checkbutton_clicked(GtkCheckButton *checkbutton);
static void on_angle_spinbutton_changed(GtkSpinButton *spinbutton);

void

beatflower_xmms_settings(void)
{
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *notebook;
  GtkWidget *fullscreen_box;
  GtkWidget *fullscreen_checkbutton;
  GtkWidget *resolution_table;
  GtkWidget *width_label;
  GtkObject *width_spinbutton_adj;
  GtkWidget *width_spinbutton;
  GtkWidget *height_label;
  GtkObject *height_spinbutton_adj;
  GtkWidget *height_spinbutton;
  GtkWidget *renderer_label;
  GtkWidget *scope_box;
  GtkWidget *color_frame;
  GtkWidget *color_box;
  GtkWidget *mode_box;
  GtkWidget *mode_label;
  GtkWidget *mode_option;
  GtkWidget *mode_option_menu;
  GtkWidget *glade_menuitem;
  GtkWidget *color_table;
  GtkWidget *color1_label;
  GtkWidget *color2_label;
  GtkWidget *color3_label;
  /* GtkWidget *color3;
   GtkWidget *color2;
   GtkWidget *color1;*/
  GtkWidget *amplitude_frame;
  GtkWidget *amplitude_table;
  GtkWidget *draw_label;
  GtkWidget *draw_option;
  GtkWidget *draw_option_menu;
  GtkWidget *samples_label;
  GtkWidget *samples_option;
  GtkWidget *samples_option_menu;
  GtkWidget *amplification_label;
  GtkWidget *amplification_option;
  GtkWidget *amplification_option_menu;
  GtkWidget *offset_label;
  GtkWidget *offset_option;
  GtkWidget *offset_option_menu;
  GtkWidget *scope_label;
  GtkWidget *blur_box;
  GtkWidget *blur_checkbutton;
  GtkWidget *decay_box;
  GtkWidget *decay_label;
  GtkObject *decay_spinbutton_adj;
  GtkWidget *decay_spinbutton;
  GtkWidget *special_box;
  GtkWidget *zoom_frame;
  GtkWidget *zoom_box;
  GtkWidget *zoom_checkbutton;
  GtkWidget *zoom_rate_box;
  GtkWidget *zoom_rate_label;
  GtkObject *factor_spinbutton_adj;
  GtkWidget *factor_spinbutton;
  GtkWidget *rotate_frame;
  GtkWidget *rotate_box;
  GtkWidget *rotate_checkbutton;
  GtkWidget *angle_box;
  GtkWidget *angle_label;
  GtkObject *angle_spinbutton_adj;
  GtkWidget *angle_spinbutton;
  GtkWidget *blur_label;
  GtkWidget *hbuttonbox1;
  GtkWidget *ok_button;
  GtkWidget *apply_button;
  GtkWidget *cancel_button;

  if(beatflower_xmms_settings_win)
  {
    return;
  }

  pthread_mutex_lock(&beatflower_config_mutex);

  if(!beatflower_config_loaded)
  {
    beatflower_xmms_config_load(&beatflower_config);
  }

  beatflower_newconfig = beatflower_config;

  pthread_mutex_unlock(&beatflower_config_mutex);

  beatflower_xmms_settings_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data(GTK_OBJECT(beatflower_xmms_settings_win), "beatflower_xmms_settings_win", beatflower_xmms_settings_win);
  gtk_window_set_title(GTK_WINDOW(beatflower_xmms_settings_win), "beatflower_xmms_settings_win");

  vbox1 = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(vbox1);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "vbox1", vbox1,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(vbox1);
  gtk_container_add(GTK_CONTAINER(beatflower_xmms_settings_win), vbox1);

  notebook = gtk_notebook_new();
  gtk_widget_ref(notebook);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "notebook", notebook,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(notebook);
  gtk_box_pack_start(GTK_BOX(vbox1), notebook, TRUE, TRUE, 0);

  fullscreen_box = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(fullscreen_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "fullscreen_box", fullscreen_box,

                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(fullscreen_box);
  gtk_container_add(GTK_CONTAINER(notebook), fullscreen_box);

  /* GtkCheckButton - fullscreen */
  fullscreen_checkbutton = gtk_check_button_new_with_label("Fullscreen");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fullscreen_checkbutton), beatflower_newconfig.fullscreen?TRUE:FALSE);
  gtk_widget_ref(fullscreen_checkbutton);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "fullscreen_checkbutton", fullscreen_checkbutton,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(fullscreen_checkbutton);
  gtk_box_pack_start(GTK_BOX(fullscreen_box), fullscreen_checkbutton, FALSE, FALSE, 0);

  resolution_table = gtk_table_new(2, 2, FALSE);
  gtk_widget_ref(resolution_table);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "resolution_table", resolution_table,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(resolution_table);
  gtk_box_pack_start(GTK_BOX(fullscreen_box), resolution_table, TRUE, TRUE, 0);

  width_label = gtk_label_new("Width");
  gtk_widget_ref(width_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "width_label", width_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(width_label);
  gtk_table_attach(GTK_TABLE(resolution_table), width_label, 0, 1, 0, 1,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  gtk_misc_set_alignment(GTK_MISC(width_label), 0, 0.5);

  // GtkSpinButton: config.width
  width_spinbutton_adj = gtk_adjustment_new(beatflower_newconfig.width, 0, 1600, 1, 10, 10);
  width_spinbutton = gtk_spin_button_new(GTK_ADJUSTMENT(width_spinbutton_adj), 1, 0);
  gtk_widget_ref(width_spinbutton);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "width_spinbutton", width_spinbutton,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(width_spinbutton);
  gtk_table_attach(GTK_TABLE(resolution_table), width_spinbutton, 1, 2, 0, 1,
                   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(width_spinbutton), TRUE);

  height_label = gtk_label_new("Height");
  gtk_widget_ref(height_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "height_label", height_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(height_label);
  gtk_table_attach(GTK_TABLE(resolution_table), height_label, 0, 1, 1, 2,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  gtk_misc_set_alignment(GTK_MISC(height_label), 0, 0.5);

  // GtkSpinButton: config.height
  height_spinbutton_adj = gtk_adjustment_new(beatflower_newconfig.height, 0, 1400, 1, 10, 10);
  height_spinbutton = gtk_spin_button_new(GTK_ADJUSTMENT(height_spinbutton_adj), 1, 0);
  gtk_widget_ref(height_spinbutton);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "height_spinbutton", height_spinbutton,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(height_spinbutton);
  gtk_table_attach(GTK_TABLE(resolution_table), height_spinbutton, 1, 2, 1, 2,
                   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(height_spinbutton), TRUE);

  renderer_label = gtk_label_new("Renderer");
  gtk_widget_ref(renderer_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "renderer_label", renderer_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(renderer_label);
  gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), 0), renderer_label);

  scope_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(scope_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "scope_box", scope_box,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(scope_box);
  gtk_container_add(GTK_CONTAINER(notebook), scope_box);

  color_frame = gtk_frame_new("Color");
  gtk_widget_ref(color_frame);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "color_frame", color_frame,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(color_frame);
  gtk_box_pack_start(GTK_BOX(scope_box), color_frame, TRUE, TRUE, 4);

  color_box = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(color_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "color_box", color_box,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(color_box);
  gtk_container_add(GTK_CONTAINER(color_frame), color_box);

  mode_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(mode_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "mode_box", mode_box,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(mode_box);
  gtk_box_pack_start(GTK_BOX(color_box), mode_box, FALSE, FALSE, 4);

  mode_label = gtk_label_new("Mode");
  gtk_widget_ref(mode_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "mode_label", mode_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(mode_label);
  gtk_box_pack_start(GTK_BOX(mode_box), mode_label, FALSE, FALSE, 4);

  // GtkOptionMenu: config.color_mode
  mode_option = gtk_option_menu_new();
  gtk_widget_ref(mode_option);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "mode_option", mode_option,
                           (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show(mode_option);
  gtk_box_pack_start(GTK_BOX(mode_box), mode_option, FALSE, FALSE, 4);
  mode_option_menu = gtk_menu_new();
  glade_menuitem = gtk_menu_item_new_with_label("2 Color Gradient");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_mode_option_changed), GTK_MENU(mode_option_menu));
  gtk_menu_append(GTK_MENU(mode_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("3 Color Gradient");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_mode_option_changed), GTK_MENU(mode_option_menu));
  gtk_menu_append(GTK_MENU(mode_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("Random");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_mode_option_changed), GTK_MENU(mode_option_menu));
  gtk_menu_append(GTK_MENU(mode_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("From Frequencies");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_mode_option_changed), GTK_MENU(mode_option_menu));
  gtk_menu_append(GTK_MENU(mode_option_menu), glade_menuitem);
  gtk_option_menu_set_menu(GTK_OPTION_MENU(mode_option), mode_option_menu);

  //gtk_menu_set_active(GTK_MENU(mode_option_menu), beatflower_newconfig.color_mode);
  gtk_option_menu_set_history(GTK_OPTION_MENU(mode_option), beatflower_newconfig.color_mode);




  color_table = gtk_table_new(3, 2, FALSE);
  gtk_widget_ref(color_table);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "color_table", color_table,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(color_table);
  gtk_box_pack_start(GTK_BOX(color_box), color_table, TRUE, TRUE, 0);

  color1_label = gtk_label_new("Color 1");
  gtk_widget_ref(color1_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "color1_label", color1_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(color1_label);
  gtk_table_attach(GTK_TABLE(color_table), color1_label, 0, 1, 0, 1,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 3, 3);
  gtk_misc_set_alignment(GTK_MISC(color1_label), 0, 0.5);

  color2_label = gtk_label_new("Color 2 ");
  gtk_widget_ref(color2_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "color2_label", color2_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(color2_label);
  gtk_table_attach(GTK_TABLE(color_table), color2_label, 0, 1, 1, 2,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 3, 3);
  gtk_misc_set_alignment(GTK_MISC(color2_label), 0, 0.5);

  color3_label = gtk_label_new("Color 3");
  gtk_widget_ref(color3_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "color3_label", color3_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(color3_label);
  gtk_table_attach(GTK_TABLE(color_table), color3_label, 0, 1, 2, 3,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  gtk_misc_set_alignment(GTK_MISC(color3_label), 0, 0.5);

  /*
     color3 = gnome_color_picker_new ();
     gtk_widget_ref (color3);
     gtk_object_set_data_full (GTK_OBJECT (beatflower_xmms_settings_win), "color3", color3,
        (GtkDestroyNotify) gtk_widget_unref);
     gtk_widget_show (color3);
     gtk_table_attach (GTK_TABLE (color_table), color3, 1, 2, 2, 3,
       (GtkAttachOptions) (GTK_FILL),
       (GtkAttachOptions) (0), 4, 4);

     color2 = gnome_color_picker_new ();
     gtk_widget_ref (color2);
     gtk_object_set_data_full (GTK_OBJECT (beatflower_xmms_settings_win), "color2", color2,
        (GtkDestroyNotify) gtk_widget_unref);
     gtk_widget_show (color2);
     gtk_table_attach (GTK_TABLE (color_table), color2, 1, 2, 1, 2,
       (GtkAttachOptions) (GTK_FILL),
       (GtkAttachOptions) (0), 4, 4);

     color1 = gnome_color_picker_new ();
     gtk_widget_ref (color1);
     gtk_object_set_data_full (GTK_OBJECT (beatflower_xmms_settings_win), "color1", color1,
        (GtkDestroyNotify) gtk_widget_unref);
     gtk_widget_show (color1);
     gtk_table_attach (GTK_TABLE (color_table), color1, 1, 2, 0, 1,
       (GtkAttachOptions) (GTK_FILL),
       (GtkAttachOptions) (0), 4, 4);
   */
  amplitude_frame = gtk_frame_new("Amplitude");
  gtk_widget_ref(amplitude_frame);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "amplitude_frame", amplitude_frame,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(amplitude_frame);
  gtk_box_pack_start(GTK_BOX(scope_box), amplitude_frame, TRUE, TRUE, 0);

  amplitude_table = gtk_table_new(4, 2, FALSE);
  gtk_widget_ref(amplitude_table);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "amplitude_table", amplitude_table,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(amplitude_table);
  gtk_container_add(GTK_CONTAINER(amplitude_frame), amplitude_table);

  draw_label = gtk_label_new("Draw Mode");
  gtk_widget_ref(draw_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "draw_label", draw_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(draw_label);
  gtk_table_attach(GTK_TABLE(amplitude_table), draw_label, 0, 1, 0, 1,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  gtk_misc_set_alignment(GTK_MISC(draw_label), 1, 0);

  // GtkOptionMenu: config.draw_mode
  draw_option = gtk_option_menu_new();
  gtk_widget_ref(draw_option);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "draw_option", draw_option,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(draw_option);
  gtk_table_attach(GTK_TABLE(amplitude_table), draw_option, 1, 2, 0, 1,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  draw_option_menu = gtk_menu_new();
  glade_menuitem = gtk_menu_item_new_with_label("Dots");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_draw_option_changed), GTK_MENU(draw_option_menu));
  gtk_menu_append(GTK_MENU(draw_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("Balls");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_draw_option_changed), GTK_MENU(draw_option_menu));
  gtk_menu_append(GTK_MENU(draw_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("Lines");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_draw_option_changed), GTK_MENU(draw_option_menu));
  gtk_menu_append(GTK_MENU(draw_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("Lines from Center");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_draw_option_changed), GTK_MENU(draw_option_menu));
  gtk_menu_append(GTK_MENU(draw_option_menu), glade_menuitem);
  gtk_option_menu_set_menu(GTK_OPTION_MENU(draw_option), draw_option_menu);

  //gtk_menu_set_active(GTK_MENU(draw_option_menu), beatflower_newconfig.draw_mode);
  gtk_option_menu_set_history(GTK_OPTION_MENU(draw_option), beatflower_newconfig.draw_mode);




  samples_label = gtk_label_new("Samples");
  gtk_widget_ref(samples_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "samples_label", samples_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(samples_label);
  gtk_table_attach(GTK_TABLE(amplitude_table), samples_label, 0, 1, 1, 2,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  gtk_misc_set_alignment(GTK_MISC(samples_label), 1, 0.5);

  // GtkOptionMenu: config.samples_mode
  samples_option = gtk_option_menu_new();
  gtk_widget_ref(samples_option);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "samples_option", samples_option,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(samples_option);
  gtk_table_attach(GTK_TABLE(amplitude_table), samples_option, 1, 2, 1, 2,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  samples_option_menu = gtk_menu_new();
  glade_menuitem = gtk_menu_item_new_with_label("32");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_samples_option_changed), GTK_MENU(samples_option_menu));
  gtk_menu_append(GTK_MENU(samples_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("64");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_samples_option_changed), GTK_MENU(samples_option_menu));
  gtk_menu_append(GTK_MENU(samples_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("128");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_samples_option_changed), GTK_MENU(samples_option_menu));
  gtk_menu_append(GTK_MENU(samples_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("256");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_samples_option_changed), GTK_MENU(samples_option_menu));
  gtk_menu_append(GTK_MENU(samples_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("512");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_samples_option_changed), GTK_MENU(samples_option_menu));
  gtk_menu_append(GTK_MENU(samples_option_menu), glade_menuitem);
  gtk_option_menu_set_menu(GTK_OPTION_MENU(samples_option), samples_option_menu);

  //gtk_menu_set_active(GTK_MENU(samples_option_menu), beatflower_newconfig.samples_mode);
  gtk_option_menu_set_history(GTK_OPTION_MENU(samples_option), beatflower_newconfig.samples_mode);




  amplification_label = gtk_label_new("Amplification");
  gtk_widget_ref(amplification_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "amplification_label", amplification_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(amplification_label);
  gtk_table_attach(GTK_TABLE(amplitude_table), amplification_label, 0, 1, 2, 3,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  gtk_misc_set_alignment(GTK_MISC(amplification_label), 1, 0.5);

  // GtkOptionMenu: config.amplification_mode
  amplification_option = gtk_option_menu_new();
  gtk_widget_ref(amplification_option);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "amplification_option", amplification_option,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(amplification_option);
  gtk_table_attach(GTK_TABLE(amplitude_table), amplification_option, 1, 2, 2, 3,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  amplification_option_menu = gtk_menu_new();
  glade_menuitem = gtk_menu_item_new_with_label("0.5");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_amplification_option_changed), GTK_MENU(amplification_option_menu));
  gtk_menu_append(GTK_MENU(amplification_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("1");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_amplification_option_changed), GTK_MENU(amplification_option_menu));
  gtk_menu_append(GTK_MENU(amplification_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("2");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_amplification_option_changed), GTK_MENU(amplification_option_menu));
  gtk_menu_append(GTK_MENU(amplification_option_menu), glade_menuitem);
  gtk_option_menu_set_menu(GTK_OPTION_MENU(amplification_option), amplification_option_menu);

  //gtk_menu_set_active(GTK_MENU(amplification_option_menu), beatflower_newconfig.amplification_mode);
  gtk_option_menu_set_history(GTK_OPTION_MENU(amplification_option), beatflower_newconfig.amplification_mode);




  offset_label = gtk_label_new("Offset");
  gtk_widget_ref(offset_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "offset_label", offset_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(offset_label);
  gtk_table_attach(GTK_TABLE(amplitude_table), offset_label, 0, 1, 3, 4,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  gtk_misc_set_alignment(GTK_MISC(offset_label), 1, 0.5);

  // GtkOptionMenu: config.offset_mode
  offset_option = gtk_option_menu_new();
  gtk_widget_ref(offset_option);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "offset_option", offset_option,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(offset_option);
  gtk_table_attach(GTK_TABLE(amplitude_table), offset_option, 1, 2, 3, 4,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 4, 4);
  offset_option_menu = gtk_menu_new();
  glade_menuitem = gtk_menu_item_new_with_label("-32768");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_offset_option_changed), GTK_MENU(offset_option_menu));
  gtk_menu_append(GTK_MENU(offset_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("+/-0");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_offset_option_changed), GTK_MENU(offset_option_menu));
  gtk_menu_append(GTK_MENU(offset_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label("+32768");
  gtk_widget_show(glade_menuitem);
  gtk_signal_connect(GTK_OBJECT(glade_menuitem), "activate", GTK_SIGNAL_FUNC(on_offset_option_changed), GTK_MENU(offset_option_menu));
  gtk_menu_append(GTK_MENU(offset_option_menu), glade_menuitem);
  gtk_option_menu_set_menu(GTK_OPTION_MENU(offset_option), offset_option_menu);

  //gtk_menu_set_active(GTK_MENU(offset_option_menu), beatflower_newconfig.offset_mode);
  gtk_option_menu_set_history(GTK_OPTION_MENU(offset_option), beatflower_newconfig.offset_mode);





  scope_label = gtk_label_new("Scope");
  gtk_widget_ref(scope_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "scope_label", scope_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(scope_label);
  gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK
                             (notebook), 1), scope_label);

  blur_box = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(blur_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "blur_box", blur_box,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(blur_box);
  gtk_container_add(GTK_CONTAINER(notebook), blur_box);

  vbox2 = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(vbox2);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "vbox2", vbox2,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(vbox2);
  gtk_box_pack_start(GTK_BOX(blur_box), vbox2, TRUE, TRUE, 0);

  // GtkCheckButton: config.blur
  blur_checkbutton = gtk_check_button_new_with_label("Enable Blur");
  gtk_widget_ref(blur_checkbutton);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "blur_checkbutton", blur_checkbutton,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(blur_checkbutton);
  gtk_box_pack_start(GTK_BOX(vbox2), blur_checkbutton, FALSE, FALSE, 4);


  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(blur_checkbutton), beatflower_newconfig.blur ? TRUE:FALSE);



  decay_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(decay_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "decay_box", decay_box,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(decay_box);
  gtk_box_pack_start(GTK_BOX(vbox2), decay_box, TRUE, TRUE, 0);

  decay_label = gtk_label_new("Decay Rate");
  gtk_widget_ref(decay_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "decay_label", decay_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(decay_label);
  gtk_box_pack_start(GTK_BOX(decay_box), decay_label, FALSE, FALSE, 4);


  // GtkSpinButton: config.decay
  decay_spinbutton_adj = gtk_adjustment_new(beatflower_newconfig.decay, 0, 100, 1, 10, 10);
  decay_spinbutton = gtk_spin_button_new(GTK_ADJUSTMENT(decay_spinbutton_adj), 1, 0);
  gtk_widget_ref(decay_spinbutton);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "decay_spinbutton", decay_spinbutton,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start(GTK_BOX(decay_box), decay_spinbutton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(decay_spinbutton), TRUE);
  gtk_widget_show(decay_spinbutton);


  special_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(special_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "special_box", special_box,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(special_box);
  gtk_box_pack_start(GTK_BOX(blur_box), special_box, TRUE, TRUE, 0);

  zoom_frame = gtk_frame_new("Zoom");
  gtk_widget_ref(zoom_frame);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "zoom_frame", zoom_frame,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(zoom_frame);
  gtk_box_pack_start(GTK_BOX(special_box), zoom_frame, TRUE, TRUE, 4);

  zoom_box = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(zoom_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "zoom_box", zoom_box,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(zoom_box);
  gtk_container_add(GTK_CONTAINER(zoom_frame), zoom_box);

  // GtkCheckButton: config.zoom
  zoom_checkbutton = gtk_check_button_new_with_label("Zoom in/out by beat");
  gtk_widget_ref(zoom_checkbutton);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "zoom_checkbutton", zoom_checkbutton,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(zoom_checkbutton);
  gtk_box_pack_start(GTK_BOX(zoom_box), zoom_checkbutton, FALSE, FALSE, 4);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(zoom_checkbutton), beatflower_newconfig.zoombeat ? TRUE:FALSE);



  zoom_rate_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(zoom_rate_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "zoom_rate_box", zoom_rate_box,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(zoom_rate_box);
  gtk_box_pack_start(GTK_BOX(zoom_box), zoom_rate_box, TRUE, TRUE, 0);

  zoom_rate_label = gtk_label_new("Rate");
  gtk_widget_ref(zoom_rate_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "zoom_rate_label", zoom_rate_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(zoom_rate_label);
  gtk_box_pack_start(GTK_BOX(zoom_rate_box), zoom_rate_label, FALSE, FALSE, 4);

  // GtkSpinButton: config.factor
  factor_spinbutton_adj = gtk_adjustment_new(beatflower_newconfig.factor, 0, 100, 1, 10, 10);
  factor_spinbutton = gtk_spin_button_new(GTK_ADJUSTMENT(factor_spinbutton_adj), 1, 3);
  gtk_widget_ref(factor_spinbutton);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "factor_spinbutton", factor_spinbutton,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(factor_spinbutton);
  gtk_box_pack_start(GTK_BOX(zoom_rate_box), factor_spinbutton, TRUE, TRUE, 4);
  gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(factor_spinbutton), TRUE);
  gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(factor_spinbutton), GTK_UPDATE_IF_VALID);



  rotate_frame = gtk_frame_new("Rotate");
  gtk_widget_ref(rotate_frame);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "rotate_frame", rotate_frame,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(rotate_frame);
  gtk_box_pack_start(GTK_BOX(special_box), rotate_frame, TRUE, TRUE, 4);

  rotate_box = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(rotate_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "rotate_box", rotate_box,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(rotate_box);
  gtk_container_add(GTK_CONTAINER(rotate_frame), rotate_box);

  // GtkCheckButton: config.rotate
  rotate_checkbutton = gtk_check_button_new_with_label("Rotate left/right by beat");
  gtk_widget_ref(rotate_checkbutton);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "rotate_checkbutton", rotate_checkbutton,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(rotate_checkbutton);
  gtk_box_pack_start(GTK_BOX(rotate_box), rotate_checkbutton, FALSE, FALSE, 4);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rotate_checkbutton), beatflower_newconfig.rotatebeat ? TRUE:FALSE);





  angle_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(angle_box);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "angle_box", angle_box,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(angle_box);
  gtk_box_pack_start(GTK_BOX(rotate_box), angle_box, TRUE, TRUE, 0);

  angle_label = gtk_label_new("Angle");
  gtk_widget_ref(angle_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "angle_label", angle_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(angle_label);
  gtk_box_pack_start(GTK_BOX(angle_box), angle_label, FALSE, FALSE, 4);

  // GtkSpinButton: config.angle
  angle_spinbutton_adj = gtk_adjustment_new(beatflower_newconfig.angle, -180, 180, 1, 10, 10);
  angle_spinbutton = gtk_spin_button_new(GTK_ADJUSTMENT(angle_spinbutton_adj), 1, 3);
  gtk_widget_ref(angle_spinbutton);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "angle_spinbutton", angle_spinbutton,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(angle_spinbutton);
  gtk_box_pack_start(GTK_BOX(angle_box), angle_spinbutton, TRUE, TRUE, 4);






  blur_label = gtk_label_new("Blur");
  gtk_widget_ref(blur_label);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "blur_label", blur_label,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(blur_label);
  gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), gtk_notebook_get_nth_page(GTK_NOTEBOOK
                             (notebook), 2), blur_label);

  hbuttonbox1 = gtk_hbutton_box_new();
  gtk_widget_ref(hbuttonbox1);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "hbuttonbox1", hbuttonbox1,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(hbuttonbox1);
  gtk_box_pack_start(GTK_BOX(vbox1), hbuttonbox1, TRUE, TRUE, 0);

  ok_button = gtk_button_new_with_label("Ok");
  gtk_widget_ref(ok_button);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "ok_button", ok_button,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(ok_button);
  gtk_container_add(GTK_CONTAINER(hbuttonbox1), ok_button);
  GTK_WIDGET_SET_FLAGS(ok_button, GTK_CAN_DEFAULT);

  apply_button = gtk_button_new_with_label("Apply");
  gtk_widget_ref(apply_button);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "apply_button", apply_button,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(apply_button);
  gtk_container_add(GTK_CONTAINER(hbuttonbox1), apply_button);
  GTK_WIDGET_SET_FLAGS(apply_button, GTK_CAN_DEFAULT);

  cancel_button = gtk_button_new_with_label("Cancel");
  gtk_widget_ref(cancel_button);
  gtk_object_set_data_full(GTK_OBJECT(beatflower_xmms_settings_win), "cancel_button", cancel_button,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(cancel_button);
  gtk_container_add(GTK_CONTAINER(hbuttonbox1), cancel_button);
  GTK_WIDGET_SET_FLAGS(cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect(GTK_OBJECT(width_spinbutton), "changed", GTK_SIGNAL_FUNC(on_width_spinbutton_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(height_spinbutton), "changed", GTK_SIGNAL_FUNC(on_height_spinbutton_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(factor_spinbutton), "changed", GTK_SIGNAL_FUNC(on_factor_spinbutton_changed), NULL);
  gtk_signal_connect(GTK_OBJECT(angle_spinbutton), "changed", GTK_SIGNAL_FUNC(on_angle_spinbutton_changed), NULL);
  //gtk_signal_connect (GTK_OBJECT (fullscreen_checkbutton), "toggled", GTK_SIGNAL_FUNC (on_fullscreen_checkbutton_changed), NULL);
//
//	gtk_signal_connect (GTK_OBJECT (mode_option), "clicked", GTK_SIGNAL_FUNC (on_mode_option_changed), GTK_MENU(NULL));
//  gtk_signal_connect (GTK_OBJECT (draw_option), "clicked", GTK_SIGNAL_FUNC (on_draw_option_changed), GTK_MENU(NULL));
//  gtk_signal_connect (GTK_OBJECT (samples_option), "clicked", GTK_SIGNAL_FUNC (on_samples_option_changed), GTK_MENU(NULL));
//  gtk_signal_connect (GTK_OBJECT (amplification_option), "clicked", GTK_SIGNAL_FUNC (on_amplification_option_changed), GTK_MENU(NULL));
//  gtk_signal_connect (GTK_OBJECT (offset_option), "clicked", GTK_SIGNAL_FUNC (on_offset_option_changed), GTK_MENU(NULL));
//	gtk_signal_connect (GTK_OBJECT (mode_option_menu), "changed", GTK_SIGNAL_FUNC (on_mode_option_changed), GTK_MENU(NULL));
//  gtk_signal_connect (GTK_OBJECT (draw_option_menu), "changed", GTK_SIGNAL_FUNC (on_draw_option_changed), GTK_MENU(NULL));
//  gtk_signal_connect (GTK_OBJECT (samples_option_menu), "changed", GTK_SIGNAL_FUNC (on_samples_option_changed), GTK_MENU(NULL));
//  gtk_signal_connect (GTK_OBJECT (amplification_option_menu), "changed", GTK_SIGNAL_FUNC (on_amplification_option_changed), GTK_MENU(NULL));
//  gtk_signal_connect (GTK_OBJECT (offset_option_menu), "changed", GTK_SIGNAL_FUNC (on_offset_option_changed), GTK_MENU(NULL));
//
  gtk_signal_connect(GTK_OBJECT(ok_button), "clicked", GTK_SIGNAL_FUNC(on_ok_button_clicked), NULL);
  gtk_signal_connect(GTK_OBJECT(apply_button), "clicked", GTK_SIGNAL_FUNC(on_apply_button_clicked), NULL);
  gtk_signal_connect(GTK_OBJECT(cancel_button), "clicked", GTK_SIGNAL_FUNC(on_cancel_button_clicked), NULL);

  gtk_signal_connect(GTK_OBJECT(blur_checkbutton), "clicked", GTK_SIGNAL_FUNC(on_blur_checkbutton_clicked), NULL);
  gtk_signal_connect(GTK_OBJECT(zoom_checkbutton), "clicked", GTK_SIGNAL_FUNC(on_zoom_checkbutton_clicked), NULL);
  gtk_signal_connect(GTK_OBJECT(fullscreen_checkbutton), "clicked", GTK_SIGNAL_FUNC(on_fullscreen_checkbutton_clicked), NULL);


  /* gtk_signal_connect (GTK_OBJECT (color3), "color_set",
     GTK_SIGNAL_FUNC (on_color3_color_set),
     NULL);
   gtk_signal_connect (GTK_OBJECT (color2), "color_set",
     GTK_SIGNAL_FUNC (on_color2_color_set),
     NULL);
   gtk_signal_connect (GTK_OBJECT (color1), "color_set",
     GTK_SIGNAL_FUNC (on_color1_color_set),
     NULL);*/

  gtk_widget_show((GtkWidget *)beatflower_xmms_settings_win);
  gtk_widget_grab_default(ok_button);
}

/************************ Gtk+ event handlers *******************************/

void 
on_fullscreen_checkbutton_clicked(GtkCheckButton *checkbutton)
{
  gboolean fullscreen = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton));

  g_message("%s: fullscreen = %s", __PRETTY_FUNCTION__, fullscreen?"TRUE":"FALSE");
  beatflower_newconfig.fullscreen = fullscreen ;
}

void 
on_width_spinbutton_changed(GtkSpinButton *spinbutton)
{
  int value;

  value = gtk_spin_button_get_value_as_int(spinbutton);

  if(value > 160 && value < 1600)
  {
    g_message("%s: width = %u", __PRETTY_FUNCTION__, value);
    beatflower_newconfig.width = value;
  }
}

void 
on_height_spinbutton_changed(GtkSpinButton *spinbutton)
{
  int value;

  value = gtk_spin_button_get_value_as_int(spinbutton);

  if(value > 160 && value < 1400)
  {
    g_message("%s: height = %u", __PRETTY_FUNCTION__, value);
    beatflower_newconfig.height = value;
  }
}

/*void on_color1_color_set(GnomeColorPicker *gnomecolorpicker,
                         guint             arg1,
                         guint             arg2,
                         guint             arg3,
                         guint             arg4)
{
  unsigned char red, green, blue;

  gnome_color_picker_get_i8(gnomecolorpicker, &red, &green, &blue, NULL);
  beatflower_newconfig.color1 = (red << RED_SHIFT) | (green << GREEN_SHIFT) | (blue << BLUE_SHIFT);
}

void on_color2_color_set(GnomeColorPicker *gnomecolorpicker,
                         guint             arg1,
                         guint             arg2,
                         guint             arg3,
                         guint             arg4)
{
  unsigned char red, green, blue;

  gnome_color_picker_get_i8(gnomecolorpicker, &red, &green, &blue, NULL);
  beatflower_newconfig.color2 = (red << RED_SHIFT) | (green << GREEN_SHIFT) | (blue << BLUE_SHIFT);
}

void on_color3_color_set(GnomeColorPicker *gnomecolorpicker,
                         guint             arg1,
                         guint             arg2,
                         guint             arg3,
                         guint             arg4)
{
  unsigned char red, green, blue;

  gnome_color_picker_get_i8(gnomecolorpicker, &red, &green, &blue, NULL);
  beatflower_newconfig.color3 = (red << RED_SHIFT) | (green << GREEN_SHIFT) | (blue << BLUE_SHIFT);
}*/

void 
on_mode_option_changed(GtkMenuItem *item, GtkMenu *menu)
{
  GtkWidget *active;
  gint32     index;

  active = gtk_menu_get_active(GTK_MENU(menu));
  index = g_list_index(gtk_container_children(GTK_CONTAINER(menu)), active);
  g_message("%s: color_mode = %i", __PRETTY_FUNCTION__, index);
  beatflower_newconfig.color_mode = index;
}

void 
on_draw_option_changed(GtkMenuItem *item, GtkMenu *menu)
{
  GtkWidget *active;
  gint32     index;

  active = gtk_menu_get_active(GTK_MENU(menu));
  index = g_list_index(gtk_container_children(GTK_CONTAINER(menu)), active);
  g_message("%s: draw_mode = %i", __PRETTY_FUNCTION__, index);
  beatflower_newconfig.draw_mode = index;
}

void 
on_samples_option_changed(GtkMenuItem *item, GtkMenu *menu)
{
  GtkWidget *active;
  gint32     index;

  active = gtk_menu_get_active(GTK_MENU(menu));
  index = g_list_index(gtk_container_children(GTK_CONTAINER(menu)), active);
  g_message("%s: samples_mode = %i", __PRETTY_FUNCTION__, index);
  beatflower_newconfig.samples_mode = index;
}

void 
on_amplification_option_changed(GtkMenuItem *item, GtkMenu *menu)
{
  GtkWidget *active;
  gint32     index;

  active = gtk_menu_get_active(GTK_MENU(menu));
  index = g_list_index(gtk_container_children(GTK_CONTAINER(menu)), active);
  g_message("%s: amplification_mode = %i", __PRETTY_FUNCTION__, index);
  beatflower_newconfig.amplification_mode = index;
}

void 
on_offset_option_changed(GtkMenuItem *item, GtkMenu *menu)
{
  GtkWidget *active;
  gint32     index;

  active = gtk_menu_get_active(GTK_MENU(menu));
  index = g_list_index(gtk_container_children(GTK_CONTAINER(menu)), active);
  g_message("%s: beatflower.offset_mode = %i", __PRETTY_FUNCTION__, index);
  beatflower_newconfig.offset_mode = index;
}

void 
on_blur_checkbutton_clicked(GtkCheckButton *checkbutton)
{
  gboolean flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton));
  g_message("%s: blur = %i", __PRETTY_FUNCTION__, flag);
  beatflower_newconfig.blur = flag;
}

void 
on_apply_button_clicked(GtkButton *button)
{
  beatflower_xmms_config_save(&beatflower_newconfig);

  //pthread_mutex_lock(&beatflower_config_mutex);
  beatflower_config = beatflower_newconfig;
  //pthread_mutex_unlock(&beatflower_config_mutex);

  //beatflower_xmms_config_save(&beatflower_config);

  pthread_mutex_lock(&beatflower_status_mutex);

  if(beatflower_playing)
  {
    beatflower_reset = TRUE;
  }

  pthread_mutex_unlock(&beatflower_status_mutex);
}

void 
on_cancel_button_clicked(GtkButton *button)
{
  gtk_widget_destroy((GtkWidget *)beatflower_xmms_settings_win);

  beatflower_newconfig = beatflower_config;
}

void 
on_ok_button_clicked(GtkButton *button)
{
  on_apply_button_clicked(button);
  on_cancel_button_clicked(button);
}

/*void on_decay_spinbutton_changed(GtkSpinButton *spinbutton)
{
  int value;

  value = gtk_spin_button_get_value_as_int(spinbutton);

  if(value > 0)
  {
    beatflower_newconfig.decay = value;
  }
}
*/
void 
on_zoom_checkbutton_clicked(GtkCheckButton *checkbutton)
{
  gboolean flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton));

  beatflower_newconfig.zoombeat = flag;
}

void 
on_factor_spinbutton_changed(GtkSpinButton *spinbutton)
{
  double value;

  value = gtk_spin_button_get_value_as_float(spinbutton);

  if(value > 0)
  {
    beatflower_newconfig.factor = value;

  }
}

void 
on_rotate_checkbutton_clicked(GtkCheckButton *checkbutton)
{
  beatflower_newconfig.rotatebeat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton));
}

void 
on_angle_spinbutton_changed(GtkSpinButton *spinbutton)
{
  double value;

  value = gtk_spin_button_get_value_as_float(spinbutton);

  if(value <= 180 && value >= -180)
  {
    beatflower_newconfig.angle = value;
  }
}


