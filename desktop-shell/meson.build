# This provides two different shells: shell-desktop and shell-rdprail (which
# is an extension of shell-desktop).

if get_option('shell-desktop')
	config_h.set_quoted('WESTON_SHELL_CLIENT', get_option('desktop-shell-client-default'))

	srcs_shell_desktop = [
		'shell.c',
		'input-panel.c',
		weston_desktop_shell_server_protocol_h,
		weston_desktop_shell_protocol_c,
		input_method_unstable_v1_server_protocol_h,
		input_method_unstable_v1_protocol_c,
	]
	deps_shell_desktop = [
		dep_libm,
		dep_libexec_weston,
		dep_libshared,
		dep_libweston_public,
		dep_lib_cairo_shared,
	]

	if get_option('shell-rdprail')
		config_h.set('BUILD_RAIL', '1')
	
		subdir('rdprail')
	
		dep_winpr = dependency(
			'winpr3',
			version: '>= 3.0.0',
			required: false
		)
		if dep_winpr.found()
			config_h.set('HAVE_WINPR', '1')
		else
			dep_winpr = dependency(
				'winpr2',
				version: '>= 2.0.0',
				required: false
			)
			if dep_winpr.found()
				config_h.set('HAVE_WINPR', '1')
			endif
		endif

		dep_glib = dependency(
			'glib-2.0',
			version : '>= 2.0.0',
			required: false
		)
		if dep_glib.found()
			config_h.set('HAVE_GLIB', 1)
		endif

		srcs_shell_desktop += [
		  weston_rdprail_shell_server_protocol_h,
		  weston_rdprail_shell_protocol_c,
		]

		deps_shell_desktop += [
			dep_winpr,
			dep_glib,
			dep_frdp_server,
		]
	endif

	plugin_shell_desktop = shared_library(
		'desktop-shell',
		srcs_shell_desktop,
		include_directories: common_inc,
		dependencies: deps_shell_desktop,
		name_prefix: '',
		install: true,
		install_dir: dir_module_weston,
		install_rpath: '$ORIGIN'
	)
	env_modmap += 'desktop-shell.so=@0@;'.format(plugin_shell_desktop.full_path())

    install_symlink(
        'rdprail-shell.so',
        install_dir: dir_module_weston,
        pointing_to: 'desktop-shell.so',
    )
endif
