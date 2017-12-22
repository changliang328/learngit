VERSION_SRC := $(realpath $(shell dirname $(lastword $(MAKEFILE_LIST)))/version.c)
LAST_COMMITID_PATH := $(realpath $(shell dirname $(lastword $(MAKEFILE_LIST)))/..)

TKCORE_APPS_COMMITID := tkcore_apps_commit-2c6560c-release

# check whether we need a rebuild
$(shell if [ -f $(LAST_COMMITID_PATH)/.last_commitid ]; then \
		if [ x"$$(cat $(LAST_COMMITID_PATH)/.last_commitid)" != x"$(TKCORE_APPS_COMMITID)" ]; then \
			echo "$(TKCORE_APPS_COMMITID)" > $(LAST_COMMITID_PATH)/.last_commitid; \
			touch $(VERSION_SRC); \
		fi \
	else echo "$(TKCORE_APPS_COMMITID)" > $(LAST_COMMITID_PATH)/.last_commitid; \
		touch $(VERSION_SRC); \
fi)
