#include <stdio.h>

#define str(s)  #s
#define xstr(s) str(s)

const char tkcore_apps_commitid[] __attribute__((used)) = xstr(TKCORE_APPS_COMMITID);

const char tkcore_apps_project_patchlevel[] __attribute__((used)) = 
	"tkcore_apps_project_patchlevel-generic#0";
