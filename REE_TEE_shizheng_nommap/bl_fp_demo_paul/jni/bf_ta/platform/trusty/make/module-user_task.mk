
MODULE_DEPS_STATIC +=  version

MODULE_DEPS_STATIC := $(addsuffix .mod.o,$(addprefix prebuilts/lib/,$(MODULE_DEPS_STATIC)))
MODULE_DEPS_STATIC += prebuilts/lib/crtbegin.o prebuilts/lib/crtend.o
