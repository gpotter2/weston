/*
 * Copyright © 2010-2012 Intel Corporation
 * Copyright © 2011-2012 Collabora, Ltd.
 * Copyright © 2013 Raspberry Pi Foundation
 * Copyright © 2020 Microsoft
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef WESTON_DESKTOP_SHELL_H
#define WESTON_DESKTOP_SHELL_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <libweston/libweston.h>
#include <libweston/xwayland-api.h>

#include "shared/image-loader.h"

#ifdef BUILD_RAIL
#include "rdprail/rdprail.h"
#include <libweston/backend-rdp.h>
#endif

#include "weston-desktop-shell-server-protocol.h"

struct focus_state {
	struct desktop_shell *shell;
	struct weston_seat *seat;
	struct workspace *ws;
	struct weston_surface *keyboard_focus;
	struct wl_list link;
	struct wl_listener seat_destroy_listener;
	struct wl_listener surface_destroy_listener;
};

/*
 * Surface stacking and ordering.
 *
 * This is handled using several linked lists of surfaces, organised into
 * ‘layers’. The layers are ordered, and each of the surfaces in one layer are
 * above all of the surfaces in the layer below. The set of layers is static and
 * in the following order (top-most first):
 *  • Lock layer (only ever displayed on its own)
 *  • Cursor layer
 *  • Input panel layer
 *  • Fullscreen layer
 *  • Panel layer
 *  • Workspace layers
 *  • Background layer
 *
 * The list of layers may be manipulated to remove whole layers of surfaces from
 * display. For example, when locking the screen, all layers except the lock
 * layer are removed.
 *
 * A surface’s layer is modified on configuring the surface, in
 * set_surface_type() (which is only called when the surface’s type change is
 * _committed_). If a surface’s type changes (e.g. when making a window
 * fullscreen) its layer changes too.
 *
 * In order to allow popup and transient surfaces to be correctly stacked above
 * their parent surfaces, each surface tracks both its parent surface, and a
 * linked list of its children. When a surface’s layer is updated, so are the
 * layers of its children. Note that child surfaces are *not* the same as
 * subsurfaces — child/parent surfaces are purely for maintaining stacking
 * order.
 *
 * The children_link list of siblings of a surface (i.e. those surfaces which
 * have the same parent) only contains weston_surfaces which have a
 * shell_surface. Stacking is not implemented for non-shell_surface
 * weston_surfaces. This means that the following implication does *not* hold:
 *	 (shsurf->parent != NULL) ⇒ !wl_list_is_empty(shsurf->children_link)
 */

struct shell_surface {
	struct wl_signal destroy_signal;

	struct weston_desktop_surface *desktop_surface;
	struct weston_view *view;
	struct weston_surface *wsurface_anim_fade;
	struct weston_view *wview_anim_fade;
	int32_t last_width, last_height;

	struct desktop_shell *shell;

	struct shell_surface *parent;
	struct wl_list children_list;
	struct wl_list children_link;

	struct weston_coord_global saved_pos;
	bool saved_position_valid;
	uint32_t saved_showstate;
	bool saved_showstate_valid;
	bool saved_rotation_valid;
	int unresponsive, grabbed;
	uint32_t resize_edges;
	uint32_t orientation;

	struct {
		struct weston_transform transform;
		struct weston_matrix rotation;
	} rotation;

	struct {
		struct weston_curtain *black_view;
	} fullscreen;

	struct {
		bool grab_unmaximized;
		int32_t saved_surface_width;
		int32_t saved_width;
		int32_t saved_height;
	} maximized;

	struct weston_output *fullscreen_output;
	struct weston_output *output;
	struct wl_listener output_destroy_listener;

	struct surface_state {
		bool fullscreen;
		bool maximized;
		bool lowered;
	} state;

	struct {
		bool is_set;
		struct weston_coord_global pos;
	} xwayland;

	struct {
		bool is_snapped;
		bool is_maximized_requested;
		struct weston_coord_global pos;
		int width;
		int height;
		struct weston_coord_global saved_pos;
		int saved_surface_width;
		int saved_width; // based on window geometry
		int saved_height; // based on window geometry
		struct weston_coord_global last_grab;
	} snapped;

	struct {
		bool is_default_icon_used;
		bool is_icon_set;
	} icon;

	struct {
		bool is_window_app_id_associated;
	} app_id;

	int focus_count;

	bool destroying;
	struct wl_list link;	/** desktop_shell::shsurf_list */

	struct wl_listener metadata_listener;
};

struct shell_grab {
	struct weston_pointer_grab grab;
	struct shell_surface *shsurf;
	struct wl_listener shsurf_destroy_listener;
};

struct shell_touch_grab {
	struct weston_touch_grab grab;
	struct shell_surface *shsurf;
	struct wl_listener shsurf_destroy_listener;
	struct weston_touch *touch;
};

struct shell_tablet_tool_grab {
	struct weston_tablet_tool_grab grab;
	struct shell_surface *shsurf;
	struct wl_listener shsurf_destroy_listener;
	struct weston_tablet_tool *tool;
};

struct weston_move_grab {
	struct shell_grab base;
	struct weston_coord_global delta;
	bool client_initiated;
};

struct weston_touch_move_grab {
	struct shell_touch_grab base;
	int active;
	struct weston_coord_global delta;
};

struct weston_tablet_tool_move_grab {
	struct shell_tablet_tool_grab base;
	wl_fixed_t dx, dy;
};

struct rotate_grab {
	struct shell_grab base;
	struct weston_matrix rotation;
	struct {
		float x;
		float y;
	} center;
};

struct shell_seat {
	struct weston_seat *seat;
	struct wl_listener seat_destroy_listener;
	struct weston_surface *focused_surface;

	struct wl_listener caps_changed_listener;
	struct wl_listener pointer_focus_listener;
	struct wl_listener keyboard_focus_listener;
	struct wl_listener tablet_tool_added_listener;

	struct wl_list link;	/** shell::seat_list */
};

struct tablet_tool_listener {
	struct wl_listener base;
	struct wl_listener removed_listener;
};

enum animation_type {
	ANIMATION_NONE,

	ANIMATION_ZOOM,
	ANIMATION_FADE,
	ANIMATION_DIM_LAYER,
};

enum fade_type {
	FADE_IN,
	FADE_OUT
};

struct focus_surface {
	struct weston_curtain *curtain;
};

struct workspace {
	struct weston_layer layer;

	struct wl_list focus_list;
	struct wl_listener seat_destroyed_listener;

	struct focus_surface *fsurf_front;
	struct focus_surface *fsurf_back;
	struct weston_view_animation *focus_animation;
};

struct shell_workarea_change {
	struct weston_output *output;
	pixman_rectangle32_t old_workarea;
	pixman_rectangle32_t new_workarea;
};

struct shell_output {
	struct desktop_shell  *shell;
	struct weston_output  *output;
	struct wl_listener	destroy_listener;
	struct wl_list		link;

	struct weston_surface *panel_surface;
	struct weston_view *panel_view;
	struct wl_listener panel_surface_listener;
	struct weston_coord_global panel_offset;

	struct weston_surface *background_surface;
	struct weston_view *background_view;
	struct wl_listener background_surface_listener;

	struct weston_curtain *temporary_curtain;

	pixman_rectangle32_t  desktop_workarea;
};

struct weston_desktop;
struct desktop_shell {
	bool rail;  // Important parameter as this sets if this is RAIL or not.

	struct weston_compositor *compositor;
	struct weston_desktop *desktop;
	const struct weston_xwayland_surface_api *xwayland_surface_api;

	struct wl_listener idle_listener;
	struct wl_listener wake_listener;
	struct wl_listener transform_listener;
	struct wl_listener resized_listener;
	struct wl_listener destroy_listener;
	struct wl_listener show_input_panel_listener;
	struct wl_listener hide_input_panel_listener;
	struct wl_listener update_input_panel_listener;
	struct wl_listener session_listener;

	struct weston_layer fullscreen_layer;
	struct weston_layer panel_layer;
	struct weston_layer background_layer;
	struct weston_layer lock_layer;
	struct weston_layer input_panel_layer;

	struct wl_listener pointer_focus_listener;
	struct weston_surface *grab_surface;

	struct {
		struct wl_client *client;
		struct wl_resource *desktop_shell;
		struct wl_listener client_destroy_listener;

		unsigned deathcount;
		struct timespec deathstamp;
	} child;

	bool locked;
	bool showing_input_panels;
	bool prepare_event_sent;

	struct text_backend *text_backend;

	struct {
		struct weston_surface *surface;
		pixman_box32_t cursor_rectangle;
	} text_input;

	struct weston_surface *lock_surface;
	struct wl_listener lock_surface_listener;
	struct weston_view *lock_view;

	struct workspace workspace;

	struct {
		struct wl_resource *binding;
		struct wl_list surfaces;
	} input_panel;

	struct {
		struct weston_curtain *curtain;
		struct weston_view_animation *animation;
		enum fade_type type;
		struct wl_event_source *startup_timer;
	} fade;

	bool allow_zap;
	bool allow_alt_f4_to_close_app;
	uint32_t binding_modifier;
	enum animation_type win_animation_type;
	enum animation_type win_close_animation_type;
	enum animation_type startup_animation_type;
	enum animation_type focus_animation_type;

	struct weston_layer minimized_layer;

	struct wl_listener seat_create_listener;
	struct wl_listener output_create_listener;
	struct wl_listener output_move_listener;
	struct wl_list output_list;
	struct wl_list seat_list;
	struct wl_list shsurf_list;

	enum weston_desktop_shell_panel_position panel_position;

	char *client;

	struct timespec startup_time;

	// RAIL-only
	bool is_localmove_supported;
	bool is_localmove_pending;

	void *app_list_context;
	char *distroName;
	size_t distroNameLength;
	bool is_appid_with_distro_name;

	struct weston_image *image_default_app_icon;
	struct weston_image *image_default_app_overlay_icon;

	bool is_blend_overlay_icon_taskbar;
	bool is_blend_overlay_icon_app_list;

	struct weston_surface *focus_proxy_surface;

#ifdef BUILD_RAIL
	const struct weston_rdprail_api *rdprail_api;
	void *rdp_backend;
#endif

	bool use_wslpath;

	struct weston_log_scope *debug;
	uint32_t debugLevel;
};

// shell.c functions used by rdprail/*

static const struct weston_pointer_grab_interface move_grab_interface;

struct weston_view *
get_default_view(struct weston_surface *surface);

struct shell_surface *
get_shell_surface(struct weston_surface *surface);

struct workspace *
get_current_workspace(struct desktop_shell *shell);

static struct shell_output *
find_shell_output_from_weston_output(struct desktop_shell *shell,
					 struct weston_output *output);

static void
shell_workarea_changed_layer(struct desktop_shell *shell,
				struct weston_layer *layer,
				void *data);

static void launch_desktop_shell_process(void *data);

void
get_output_work_area(struct desktop_shell *shell,
			 struct weston_output *output,
			 pixman_rectangle32_t *area);

void
lower_fullscreen_layer(struct desktop_shell *shell,
			   struct weston_output *lowering_output);

void
activate(struct desktop_shell *shell, struct weston_view *view,
	 struct weston_seat *seat, uint32_t flags);

int
input_panel_setup(struct desktop_shell *shell);
void
input_panel_destroy(struct desktop_shell *shell);

typedef void (*shell_for_each_layer_func_t)(struct desktop_shell *,
						struct weston_layer *, void *);

void
shell_for_each_layer(struct desktop_shell *shell,
			 shell_for_each_layer_func_t func,
			 void *data);

static void
shell_send_minmax_info(struct weston_surface *surface);

static void
set_minimized(struct weston_surface *surface);

static void
set_maximized(struct shell_surface *shsurf, bool maximized);

static void
set_unminimized(struct weston_surface *surface);

static void
shell_surface_set_output(struct shell_surface *shsurf,
						 struct weston_output *output);

static void
activate_binding(struct weston_seat *seat,
		 struct desktop_shell *shell,
		 struct weston_view *focus_view,
		 uint32_t flags);

static void
shell_surface_set_window_icon(struct weston_desktop_surface *desktop_surface,
				int32_t width, int32_t height, int32_t bpp,
				void *bits, void *user_data);

void
shell_blend_overlay_icon(struct desktop_shell *shell,
		pixman_image_t *app_image,
		pixman_image_t *overlay_image);

#endif /* WESTON_DESKTOP_SHELL_H */
