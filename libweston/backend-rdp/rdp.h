/*
 * Copyright © 2013 Hardening <rdp.effort@gmail.com>
 * Copyright © 2020 Microsoft
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef RDP_H
#define RDP_H

/* Workaround an issue with clang and freerdp 3 headers. Another
 * option would be to build with --std=c11 but weston itself isn't
 * quite ready for that
 */
#if USE_FREERDP_VERSION >= 3 && defined(__clang__)
#pragma clang diagnostic ignored "-Wtypedef-redefinition"
#endif

#define FREERDP_SETTINGS_INTERNAL_USE /* We access freerdp internal settings */
#include <freerdp/version.h>

#include <freerdp/freerdp.h>
#include <freerdp/listener.h>
#include <freerdp/update.h>
#include <freerdp/input.h>
#include <freerdp/codec/color.h>
#include <freerdp/codec/rfx.h>
#include <freerdp/codec/nsc.h>
#include <freerdp/locale/keyboard.h>
#include <freerdp/channels/wtsvc.h>
#include <freerdp/server/cliprdr.h>
#include <freerdp/server/rail.h>
#include <freerdp/server/drdynvc.h>
#include <freerdp/server/rdpgfx.h>
#include <freerdp/server/disp.h>
#include <freerdp/server/rdpsnd.h>
#include <freerdp/server/audin.h>
#include <freerdp/server/cliprdr.h>
#ifdef HAVE_FREERDP_GFXREDIR_H
#include <freerdp/server/gfxredir.h>
#endif // HAVE_FREERDP_GFXREDIR_H
#ifdef HAVE_FREERDP_RDPAPPLIST_H
#include <rdpapplist/rdpapplist_config.h>
#include <rdpapplist/rdpapplist_protocol.h>
#include <rdpapplist/rdpapplist_server.h>
#endif // HAVE_FREERDP_RDPAPPLIST_H

#include <libweston/libweston.h>
#include <libweston/backend-rdp.h>
#include <libweston/weston-log.h>
#include <libweston/helpers.h>

#include <winpr/string.h>

#include "shared/hash.h"
#include "backend.h"

#include "shared/string-helpers.h"
#include "shared/timespec-util.h"

#define MAX_FREERDP_FDS 32
#define RDP_MAX_MONITOR 16
#define DEFAULT_AXIS_STEP_DISTANCE 10
#define DEFAULT_PIXEL_FORMAT PIXEL_FORMAT_BGRA32

/* https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeyboardtype
 * defines a keyboard type that isn't currently defined in FreeRDP, but is
 * available for RDP connections */
#ifndef KBD_TYPE_KOREAN
#define KBD_TYPE_KOREAN 8
#endif

/* WinPR's GetVirtualKeyCodeFromVirtualScanCode() can't handle hangul/hanja keys */
/* 0x1f1 and 0x1f2 keys are only exists on Korean 103 keyboard (Type 8:SubType 6) */

/* From Linux's keyboard driver at drivers/input/keyboard/atkbd.c */
#define ATKBD_RET_HANJA 0xf1
#define ATKBD_RET_HANGEUL 0xf2

/* freerdp2 vs 3 compat */
#if USE_FREERDP_VERSION >= 3
#define FORM_DATA_RESP_COMM(r, f)	(r).common.f
#define XF_KEV_CODE_TYPE		UINT8
#else
#define FORM_DATA_RESP_COMM(r, f)	(r).f
#define WINPR_KBD_TYPE_JAPANESE		KBD_TYPE_JAPANESE
#define WINPR_KEYCODE_TYPE_XKB		KEYCODE_TYPE_EVDEV
#define XF_KEV_CODE_TYPE		UINT16
#endif

struct rdp_output;
struct rdp_clipboard_data_source;
struct rdp_backend;

struct rdp_id_manager {
	struct rdp_backend *rdp_backend;
	UINT32 id;
	UINT32 id_low_limit;
	UINT32 id_high_limit;
	UINT32 id_total;
	UINT32 id_used;
	pthread_mutex_t mutex;
	pid_t mutex_tid;
	struct hash_table *hash_table;
};

struct rdp_backend {
	struct weston_backend base;
	struct weston_compositor *compositor;

	freerdp_listener *listener;
	struct wl_event_source *listener_events[MAX_FREERDP_FDS];
	struct wl_list output_list; // rdp_output::link
	struct weston_log_scope *debug;
	struct weston_log_scope *verbose;

	struct weston_log_scope *clipboard_debug;
	struct weston_log_scope *clipboard_verbose;

	struct wl_list peers;

	char *server_cert;
	char *server_key;
	char *server_cert_content;
	char *server_key_content;
	char *rdp_key;
	int tls_enabled;
	int resizeable;
	int force_no_compression;
	bool remotefx_codec;
	int external_listener_fd;
	int rdp_monitor_refresh_rate;
	pid_t compositor_tid;

        rdp_audio_in_setup audio_in_setup;
        rdp_audio_in_teardown audio_in_teardown;
        rdp_audio_out_setup audio_out_setup;
        rdp_audio_out_teardown audio_out_teardown;

	uint32_t head_index;

	const struct pixel_format_info **formats;
	unsigned int formats_count;

	const struct weston_rdprail_shell_api *rdprail_shell_api;
	void *rdprail_shell_context;
	char *rdprail_shell_name;
	bool enable_copy_warning_title;
	bool enable_distro_name_title;

	freerdp_peer *rdp_peer; // this points a single instance of RAIL RDP peer.

	struct weston_binding *debug_binding_M;
	struct weston_binding *debug_binding_W;

	struct wl_listener create_window_listener;

	bool enable_window_zorder_sync;
	bool enable_window_snap_arrange;
	bool enable_window_shadow_remoting;

	bool enable_display_power_by_screenupdate;

	bool enable_hi_dpi_support;
	bool enable_fractional_hi_dpi_support;
	bool enable_fractional_hi_dpi_roundup;
	uint32_t debug_desktop_scaling_factor; /* must be between 100 to 500 */

	struct weston_surface *proxy_surface;

#ifdef HAVE_FREERDP_RDPAPPLIST_H
	/* import from libfreerdp-server2.so */
	RdpAppListServerContext *(*rdpapplist_server_context_new)(HANDLE vcm);
	void (*rdpapplist_server_context_free)(RdpAppListServerContext* context);

	void *libRDPApplistServer;
	bool use_rdpapplist;
#endif // HAVE_FREERDP_RDPAPPLIST_H

#ifdef HAVE_FREERDP_GFXREDIR_H
	/* import from libfreerdp-server2.so */
	GfxRedirServerContext *(*gfxredir_server_context_new)(HANDLE vcm);
	void (*gfxredir_server_context_free)(GfxRedirServerContext* context);

	void *libFreeRDPServer;
	bool use_gfxredir;
	char *shared_memory_mount_path;
	size_t shared_memory_mount_path_size;
#endif // HAVE_FREERDP_GFXREDIR_H
};

enum peer_item_flags {
	RDP_PEER_ACTIVATED      = (1 << 0),
	RDP_PEER_OUTPUT_ENABLED = (1 << 1),
};

struct rdp_peers_item {
	int flags;
	freerdp_peer *peer;
	struct weston_seat *seat;

	struct wl_list link;
};

struct rdp_head {
	struct weston_head base;
	uint32_t index;
	bool matched;
	rdpMonitor config;
	/*TODO: these region/rectangles can be moved to rdp_output */
	pixman_rectangle32_t workareaClient; // in client coordinate.
	pixman_rectangle32_t workarea; // in weston coordinate.
};

struct rdp_output {
	struct weston_output base;
	struct rdp_backend *backend;
	struct wl_event_source *finish_frame_timer;
	struct weston_renderbuffer *renderbuffer;
	pixman_image_t *shadow_surface;

	uint32_t index;
	struct wl_list link; // rdp_backend::output_list
};

struct rdp_peer_context {
	rdpContext _p;

	struct rdp_backend *rdpBackend;
	struct wl_event_source *events[MAX_FREERDP_FDS + 1]; /* +1 for WTSVirtualChannelManagerGetFileDescriptor */
	RFX_CONTEXT *rfx_context;
	wStream *encode_stream;
	RFX_RECT *rfx_rects;
	NSC_CONTEXT *nsc_context;

	struct rdp_peers_item item;

	bool button_state[5];

	bool mouseButtonSwap;
	int verticalAccumWheelRotationPrecise;
	int verticalAccumWheelRotationDiscrete;
	int horizontalAccumWheelRotationPrecise;
	int horizontalAccumWheelRotationDiscrete;

	HANDLE vcm;

	/* Clipboard support */
	CliprdrServerContext *clipboard_server_context;

	// RAIL support
	RailServerContext *rail_server_context;
	DrdynvcServerContext *drdynvc_server_context;
	DispServerContext *disp_server_context;
	RdpgfxServerContext *rail_grfx_server_context;
#ifdef HAVE_FREERDP_GFXREDIR_H
	GfxRedirServerContext *gfxredir_server_context;
#endif // HAVE_FREERDP_GFXREDIR_H
#ifdef HAVE_FREERDP_RDPAPPLIST_H
	RdpAppListServerContext *applist_server_context;
#endif // HAVE_FREERDP_RDPAPPLIST_H
	bool handshakeCompleted;
	bool activationRailCompleted;
	bool activationGraphicsCompleted;
	bool activationGraphicsRedirectionCompleted;
	uint32_t clientStatusFlags;
	struct rdp_id_manager windowId;
	struct rdp_id_manager surfaceId;
#ifdef HAVE_FREERDP_GFXREDIR_H
	struct rdp_id_manager poolId;
	struct rdp_id_manager bufferId;
#endif // HAVE_FREERDP_GFXREDIR_H
	uint32_t currentFrameId;
	uint32_t acknowledgedFrameId;
	bool isAcknowledgedSuspended;
	struct wl_client *clientExec;
	struct wl_listener clientExec_destroy_listener;
	struct weston_surface *cursorSurface;

	// list of outstanding event_source sent from FreeRDP thread to display loop.
	int loop_task_event_source_fd;
	struct wl_event_source *loop_task_event_source;
	pthread_mutex_t loop_task_list_mutex;
	struct wl_list loop_task_list; // struct rdp_loop_task::link

	// RAIL power management.
	struct wl_listener idle_listener;
	struct wl_listener wake_listener;

	bool is_window_zorder_dirty;

	void *audio_in_private;
	void *audio_out_private;

	struct rdp_clipboard_data_source *clipboard_client_data_source;
	struct rdp_clipboard_data_source *clipboard_inflight_client_data_source;

	struct wl_listener clipboard_selection_listener;

	/* Multiple monitor support (monitor topology) */
	int32_t desktop_top, desktop_left;
	int32_t desktop_width, desktop_height;
	
    // Application List support
	BOOL isAppListEnabled;
};

typedef struct rdp_peer_context RdpPeerContext;

typedef void (*rdp_loop_task_func_t)(bool freeOnly, void *data);

struct rdp_loop_task {
	struct wl_list link;
	RdpPeerContext *peerCtx;
	rdp_loop_task_func_t func;
};

#define RDP_RAIL_MARKER_WINDOW_ID  0xFFFFFFFE
#define RDP_RAIL_DESKTOP_WINDOW_ID 0xFFFFFFFF

#define rdp_debug_verbose(b, ...) \
	rdp_debug_print(b->verbose, false, __VA_ARGS__)
#define rdp_debug_verbose_continue(b, ...) \
	rdp_debug_print(b->verbose, true,  __VA_ARGS__)
#define rdp_debug(b, ...) \
	rdp_debug_print(b->debug, false, __VA_ARGS__)
#define rdp_debug_continue(b, ...) \
	rdp_debug_print(b->debug, true,  __VA_ARGS__)

#define rdp_debug_clipboard_verbose(b, ...) \
	rdp_debug_print(b->clipboard_verbose, false, __VA_ARGS__)
#define rdp_debug_clipboard_verbose_continue(b, ...) \
	rdp_debug_print(b->clipboard_verbose, true,  __VA_ARGS__)
#define rdp_debug_clipboard(b, ...) \
	rdp_debug_print(b->clipboard_debug, false, __VA_ARGS__)
#define rdp_debug_clipboard_continue(b, ...) \
	rdp_debug_print(b->clipboard_debug, true,  __VA_ARGS__)

/* rdpdisp.c */
bool
handle_adjust_monitor_layout(freerdp_peer *client,
			     int monitor_count, rdpMonitor *monitors);

struct weston_output *
to_weston_coordinate(RdpPeerContext *peerContext,
		     int32_t *x, int32_t *y,
		     uint32_t *width, uint32_t *height);

bool
handle_adjust_monitor_layout(freerdp_peer *client,
        int monitor_count, rdpMonitor *monitors);

void
to_client_coordinate(RdpPeerContext *peerContext,
        struct weston_output *output,
        int32_t *x, int32_t *y,
        uint32_t *width, uint32_t *height);

float
disp_get_client_scale_from_monitor(
        struct rdp_backend *b,
        const rdpMonitor *config);

int
disp_get_output_scale_from_monitor(
        struct rdp_backend *b,
        const rdpMonitor *config);

void
disp_monitor_validate_and_compute_layout(
		struct weston_compositor *ec,
        uint32_t default_width,
        uint32_t default_height);

/* rdputil.c */
void
rdp_debug_print(struct weston_log_scope *log_scope, bool cont, char *fmt, ...);

int
rdp_wl_array_read_fd(struct wl_array *array, int fd);

void
convert_rdp_keyboard_to_xkb_rule_names(UINT32 KeyboardType, UINT32 KeyboardSubType, UINT32 KeyboardLayout, struct xkb_rule_names *xkbRuleNames);

void
assert_compositor_thread(struct rdp_backend *b);

void
assert_not_compositor_thread(struct rdp_backend *b);

bool
rdp_event_loop_add_fd(struct wl_event_loop *loop,
		      int fd, uint32_t mask,
		      wl_event_loop_fd_func_t func,
		      void *data,
		      struct wl_event_source **event_source);

void
rdp_dispatch_task_to_display_loop(RdpPeerContext *peerCtx,
				  rdp_loop_task_func_t func,
				  struct rdp_loop_task *task);

bool
rdp_initialize_dispatch_task_event_source(RdpPeerContext *peerCtx);

void
rdp_destroy_dispatch_task_event_source(RdpPeerContext *peerCtx);

/* rdpclip.c */
int
rdp_clipboard_init(freerdp_peer *client);

void
rdp_clipboard_destroy(RdpPeerContext *peerCtx);

/* rdp.c */
void
rdp_head_create(struct rdp_backend *backend, rdpMonitor *config);

void
rdp_destroy(struct weston_backend *backend);

void
rdp_head_destroy(struct weston_head *base);

struct weston_output*
rdp_output_get_primary(struct weston_compositor *compositor);

void
rdp_output_destroy(struct weston_output *base);

#ifdef HAVE_FREERDP_GFXREDIR_H
BOOL rdp_allocate_shared_memory(struct rdp_backend *b, struct weston_rdp_shared_memory *shared_memory);
void rdp_free_shared_memory(struct rdp_backend *b, struct weston_rdp_shared_memory *shared_memory);
#endif // HAVE_FREERDP_GFXREDIR_H

// rdprail.c
int rdp_rail_backend_create(struct rdp_backend *b, struct weston_rdp_backend_config *config);
void rdp_rail_destroy(struct rdp_backend *b);
bool rdp_rail_peer_activate(freerdp_peer *client);
void rdp_rail_sync_window_status(freerdp_peer *client);
bool rdp_rail_peer_init(freerdp_peer *client, RdpPeerContext *peerCtx);
void rdp_rail_peer_context_free(freerdp_peer *client, RdpPeerContext *context);
void rdp_rail_output_repaint(struct weston_output *output, pixman_region32_t *damage);
bool rdp_drdynvc_init(freerdp_peer *client);
void rdp_drdynvc_destroy(RdpPeerContext *context);

BOOL rdp_id_manager_init(struct rdp_backend *rdp_backend, struct rdp_id_manager *id_manager, UINT32 low_limit, UINT32 high_limit);
void rdp_id_manager_free(struct rdp_id_manager *id_manager);
void rdp_id_manager_lock(struct rdp_id_manager *id_manager);
void rdp_id_manager_unlock(struct rdp_id_manager *id_manager);
void *rdp_id_manager_lookup(struct rdp_id_manager *id_manager, UINT32 id);
void rdp_id_manager_for_each(struct rdp_id_manager *id_manager, hash_table_iterator_func_t func, void *data);
BOOL rdp_id_manager_allocate_id(struct rdp_id_manager *id_manager, void *object, UINT32 *new_id);
void rdp_id_manager_free_id(struct rdp_id_manager *id_manager, UINT32 id);
void dump_id_manager_state(FILE *fp, struct rdp_id_manager *id_manager, char* title);
bool rdp_defer_rdp_task_to_display_loop(RdpPeerContext *peerCtx, wl_event_loop_fd_func_t func, void *data, struct wl_event_source **event_source);
void rdp_defer_rdp_task_done(RdpPeerContext *peerCtx);

static inline struct rdp_head *
to_rdp_head(const struct weston_head *base)
{
	return container_of(base, struct rdp_head, base);
}

static inline struct rdp_output *
to_rdp_output(struct weston_output *base)
{
	return container_of(base, struct rdp_output, base);
}

static inline struct rdp_backend *
to_rdp_backend(struct weston_compositor *base)
{
	return container_of(base->primary_backend, struct rdp_backend, base);
}

static inline void
rdp_matrix_transform_position(struct weston_matrix *matrix, int *x, int *y)
{
	struct weston_vector v;
	if (matrix->type != 0) {
		v.f[0] = *x;
		v.f[1] = *y;
		v.f[2] = 0.0f;
		v.f[3] = 1.0f;
		weston_matrix_transform(matrix, &v);
		*x = v.f[0] / v.f[3];
		*y = v.f[1] / v.f[3];
	}
}

static inline void
rdp_matrix_transform_scale(struct weston_matrix *matrix, int *sx, int *sy)
{
	struct weston_vector v;
	if (matrix->type != 0) {
		v.f[0] = *sx;
		v.f[1] = *sy;
		v.f[2] = 0.0f;
		v.f[3] = 0.0f;
		weston_matrix_transform(matrix, &v);
		*sx = v.f[0]; // / v.f[3];
		*sy = v.f[1]; // / v.f[3];
	}
}

#define RDP_RAIL_WINDOW_RESIZE_MARGIN 8

static inline bool
is_window_shadow_remoting_disabled(RdpPeerContext *peerCtx)
{
	struct rdp_backend *b = peerCtx->rdpBackend;

	/* When shadow is not remoted, window geometry must be able to queried from shell to clip
	   shadow area, and resize margin must be supported by client. When remoting window shadow,
	   the shadow area is used as resize margin, but without it, window can't be resizable,
	   thus window margin must be added by client side. */
	return (!b->enable_window_shadow_remoting &&
			b->rdprail_shell_api && b->rdprail_shell_api->get_window_geometry &&
			(peerCtx->clientStatusFlags & TS_RAIL_CLIENTSTATUS_WINDOW_RESIZE_MARGIN_SUPPORTED));
}

#endif
