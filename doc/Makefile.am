nodist_man_MANS = \
	adwm.1 \
	adwm-config.5 \
	adwm-dock.5 \
	adwm-keys.5 \
	adwm-buttons.5 \
	adwm-rules.5 \
	adwm-style.5 \
	adwm-theme.5

adwm_pod_files = \
	adwm.pod \
	adwm-config.pod \
	adwm-keys.pod \
	adwm-buttons.pod \
	adwm-rules.pod \
	adwm-style.pod \
	adwm-theme.pod

adwm_man1_files = \
	adwm.man

adwm_man5_files = \
	adwm-keys.man \
	adwm-buttons.man \
	adwm-rules.man \
	adwm-style.man \
	adwm-theme.man \
	adwm-config.man

# dist_man1_MANS = $(adwm_man1_files)
# dist_man5_MANS = $(adwm_man5_files)

SUFFIXES = .man .pod

adwm.man: TITLE = User Commands
adwm.man: SECTION = 1
adwm.man: adwm.pod

adwm-config.man: TITLE = File Formats
adwm-config.man: SECTION = 5
adwm-config.man: adwm-config.pod

adwm-keys.man: TITLE = File Formats
adwm-keys.man: SECTION = 5
adwm-keys.man: adwm-keys.pod

adwm-buttons.man: TITLE = File Formats
adwm-buttons.man: SECTION = 5
adwm-buttons.man: adwm-buttons.pod

adwm-rules.man: TITLE = File Formats
adwm-rules.man: SECTION = 5
adwm-rules.man: adwm-rules.pod

adwm-style.man: TITLE = File Formats
adwm-style.man: SECTION = 5
adwm-style.man: adwm-style.pod

adwm-theme.man: TITLE = File Formats
adwm-theme.man: SECTION = 5
adwm-theme.man: adwm-theme.pod


.pod.man:
	$(AM_V_GEN)pod2man \
		--center="$(TITLE)" \
		--date="$(DATE)" \
		--errors="none" \
		--fixed="CW" \
		--fixedbold="CB" \
		--fixeditalic="CI" \
		--fixedbolditalic="CX" \
		--nourls \
		--release="$(PACKAGE) $(VERSION)" \
		--section="$(SECTION)" \
		$< >$@

# CLEANFILES = $(adwm_man1_files) $(adwm_man5_files)

# DISTCLEANFILES = $(adwm_man1_files) $(adwm_man5_files)

# EXTRA_DIST = $(adwm_pod_files)

