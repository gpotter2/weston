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

#ifndef WESTON_DESKTOP_RDPRAIL_H
#define WESTON_DESKTOP_RDPRAIL_H

#include "../shell.h"

#define is_system_distro() (getenv("WSL2_VM_ID") != NULL)

struct desktop_shell; // from ../shell.h

// Shell Backend implementation

static void
shell_backend_request_window_minimize(struct weston_surface *surface);

static void
shell_backend_request_window_maximize(struct weston_surface *surface);

static void
shell_backend_request_window_restore(struct weston_surface *surface);

static void
shell_backend_request_window_move(struct weston_surface *surface, int x, int y, int width, int height);

static void
shell_backend_request_window_snap(struct weston_surface *surface, int x, int y, int width, int height);

static void
shell_backend_request_window_activate(void *shell_context, struct weston_seat *seat, struct weston_surface *surface);

static void
shell_backend_request_window_close(struct weston_surface *surface); 

static void
shell_backend_set_desktop_workarea(struct weston_output *output, void *context, pixman_rectangle32_t *workarea);

static pid_t
shell_backend_get_app_id(void *shell_context, struct weston_surface *surface, char *app_id, size_t app_id_size, char *image_name, size_t image_name_size);

static bool
shell_backend_start_app_list_update(void *shell_context, char *clientLanguageId);

static void
shell_backend_stop_app_list_update(void *shell_context);

static void
shell_backend_request_window_icon(struct weston_surface *surface);

static struct wl_client *
shell_backend_launch_shell_process(void *shell_context, char *exec_name);

static void
shell_backend_get_window_geometry(struct weston_surface *surface, struct weston_geometry *geometry);

static const struct weston_rdprail_shell_api rdprail_shell_api;

// App-list

pixman_image_t*
app_list_load_icon_file(struct desktop_shell *shell, const char *key);

void app_list_init(struct desktop_shell *shell);

void app_list_find_image_name(struct desktop_shell *shell,
		pid_t pid,
		char *image_name, size_t image_name_size,
		bool is_wayland);

void app_list_associate_window_app_id(
		struct desktop_shell *shell,
		pid_t pid,
		char *app_id,
		uint32_t window_id);

bool app_list_start_backend_update(struct desktop_shell *shell,
		char *clientLanguageId);

void app_list_stop_backend_update(struct desktop_shell *shell);

void app_list_destroy(struct desktop_shell *shell);

#endif /* WESTON_DESKTOP_RDPRAIL_H */
