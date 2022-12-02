src_wk = ['main.c', 'list.c', 'verify.c', 'file.c', 'handle.c', 'init.c', 'common.c', 'log.c']
src_ag = ['autogen.c', 'list.c', 'file.c', 'common.c']
src_mgr = ['manager.c', 'tee.c', 'log.c', 'list.c']

dep_crypto = dependency ('libcrypto', version: '> 1.0')
deps_wk = [dep_crypto]
deps_mgr = []

dep_syslog = dependency ('syslog-ng', required: get_option ('syslog'))
if dep_syslog.found()
	deps_wk += dep_syslog
	deps_mgr += dep_syslog
endif

dep_systemd = dependency ('libsystemd', required: get_option ('systemd'))
if dep_systemd.found()
	deps_wk += dep_systemd
	deps_mgr += dep_systemd
endif

dep_teec = meson.get_compiler ('c').find_library ('teec', required: get_option ('tee'), has_headers: ['tee_client_api.h'])
if dep_teec.found()
	src_wk += ['tee.c']
	deps_wk += dep_teec
	deps_mgr += dep_teec
	subdir ('ta')
	executable ('wkmgr', src_mgr, dependencies: deps_mgr, install: true)
endif

cfgdata = configuration_data()
cfgdata.set_quoted ('PACKAGE_NAME', meson.project_name())
cfgdata.set_quoted ('CONFIGDIR', join_paths (get_option ('prefix'), get_option ('sysconfdir')))
cfgdata.set_quoted ('DATAROOTDIR', join_paths (get_option ('prefix'), get_option ('datadir')))
cfgdata.set ('CONFIG_SYSLOG', dep_syslog.found())
cfgdata.set ('CONFIG_SYSTEMD', dep_systemd.found())
cfgdata.set ('DEBUG', get_option ('debug'))
cfgdata.set ('CONFIG_TEE', dep_teec.found())
configure_file (output: 'config.h', configuration: cfgdata)

executable ('whiteknight', src_wk, dependencies: deps_wk, install: true)
executable ('wkag', src_ag, dependencies: dep_crypto)
