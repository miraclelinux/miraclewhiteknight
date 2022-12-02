# install systemd service file
systemd_service_dir = join_paths (get_option ('prefix'), get_option ('libdir'), 'systemd', 'system')
service_data = configuration_data ({
		'bindir': join_paths (get_option ('prefix'), get_option ('bindir')),
		'requires': dep_teec.found () ? 'tee-supplicant' : '',
	})
service_file = configure_file (
		input: '@0@.service.in'.format (meson.project_name()),
		install: true,
		install_dir: systemd_service_dir,
		install_mode: 'r--r--r--',
		output: '@BASENAME@',
		configuration: service_data
	)
