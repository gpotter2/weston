if not get_option('backend-rdp')
        subdir_done()
endif

config_h.set('BUILD_RDP_COMPOSITOR', '1')

dep_frdp3 = dependency('freerdp3', version: '>= 3.0.0', required: false)
dep_frdp2 = dependency('freerdp2', version: '>= 2.3.0', required: false)

if dep_frdp3.found()
	config_h.set('USE_FREERDP_VERSION', '3')
	dep_frdp = dep_frdp3
	dep_frdp_server = dependency('freerdp-server3', version: '>= 3.0.0', required: false)
		if not dep_frdp_server.found()
			error('RDP-backend v3 requires freerdp-server3 >= 3.0.0 which was not found. Or, you can use \'-Dbackend-rdp=false\'.')
		endif

	dep_wpr = dependency('winpr3', version: '>= 3.0.0', required: false)
	if not dep_wpr.found()
		error('RDP-backend v3 requires winpr >= 3.0.0 which was not found. Or, you can use \'-Dbackend-rdp=false\'.')
	endif
	message('Using FreeRDP 3.x for RDP backend')
elif dep_frdp2.found()
	config_h.set('USE_FREERDP_VERSION', '2')
	dep_frdp = dep_frdp2
	dep_frdp_server = dependency('freerdp-server2', version: '>= 2.3.0', required: false)
		if not dep_frdp_server.found()
			error('RDP-backend v2 requires freerdp-server2 >= 2.3.0 which was not found. Or, you can use \'-Dbackend-rdp=false\'.')
		endif

	dep_wpr = dependency('winpr2', version: '>= 2.3.0', required: false)
	if not dep_wpr.found()
		error('RDP-backend requires v2 winpr >= 2.3.0 which was not found. Or, you can use \'-Dbackend-rdp=false\'.')
	endif
	message('Using FreeRDP 2.x for RDP backend')
else
	error('RDP-backend requires freerdp3 >= 3.0.0 or freerdp2 >= 2.3.0 which was not found. Or, you can use \'-Dbackend-rdp=false\'.')
endif

dep_rdpapplist = dependency('rdpapplist', version: '>= 2.0.0', required: false)
if dep_rdpapplist.found()
	config_h.set('HAVE_FREERDP_RDPAPPLIST_H', '1')
endif

if cc.has_header('freerdp/channels/gfxredir.h', dependencies: dep_frdp)
	config_h.set('HAVE_FREERDP_GFXREDIR_H', '1')
endif

if cc.has_member(
	'rdpsnd_server_context', 'use_dynamic_virtual_channel',
	dependencies : dep_frdp,
	prefix : '#include <freerdp/server/rdpsnd.h>'
)
	config_h.set('HAVE_RDPSND_DYNAMIC_VIRTUAL_CHANNEL', '1')
endif

deps_rdp = [
        dep_frdp,
        dep_frdp_server,
        dep_libdrm_headers,
        dep_libshared,
        dep_libweston_private,
        dep_rdpapplist,
        dep_wpr,
]
srcs_rdp = [
        'rdp.c',
        'rdpclip.c',
        'rdpdisp.c',
        'rdprail.c',
        'rdputil.c',
]

plugin_rdp = shared_library(
        'rdp-backend',
        srcs_rdp,
        include_directories: common_inc,
        dependencies: deps_rdp,
        name_prefix: '',
        install: true,
        install_dir: dir_module_libweston
)
env_modmap += 'rdp-backend.so=@0@;'.format(plugin_rdp.full_path())
install_headers(backend_rdp_h, subdir: dir_include_libweston_install)
