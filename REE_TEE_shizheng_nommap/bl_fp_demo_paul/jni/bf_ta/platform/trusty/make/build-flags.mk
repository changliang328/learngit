
LKINC ?=\
  headers/external/headers/include \
  headers/external/lk/arch/arm/arm/include \
  headers/external/lk/arch/arm/include \
  headers/external/lk/include \
  headers/external/lk/lib/fixed_point/include \
  headers/lib/include \
  headers/lk/trusty/include \
  headers/lk/trusty/lib/sm/include \
  headers/lk/trusty/lib/syscall/include \
  headers/lk/trusty/lib/trusty/include \
  headers/lk/trusty/lib/uthread/arch/arm/include \
  headers/lk/trusty/lib/uthread/include \
  headers/lk/trusty/lib/version/include \
  headers/system/gatekeeper/include \
  headers/system/keymaster/include \


LIB_INCLUDES ?=\
  headers/lib/include \
  headers/lib/interface/hwkey/include \
  headers/lib/interface/hwrng/include \
  headers/lib/interface/keymaster/include \
  headers/lib/interface/storage/include \
  headers/lib/lib/hwkey/include \
  headers/lib/lib/keymaster/include \
  headers/lib/lib/libc-trusty/include \
  headers/lib/lib/libstdc++-trusty/include \
  headers/lib/lib/openssl-engine/include \
  headers/lib/lib/openssl-stubs/include \
  headers/lib/lib/rng/include \
  headers/lib/lib/storage/include \
  headers/external/lk/lib/libm/include \
  headers/external/openssl/src/include \
  headers/external/lk/platform/sprd/fingerprint/default/include \


EXTRA_BUILDRULES := make/user-tasks.mk make/version.mk

GLOBAL_OPTFLAGS   := -O2
ARCH              := arm
ARCH_$(ARCH)_COMPILEFLAGS := -mabi=aapcs-linux -mcpu=cortex-a7 -mfpu=vfpv3 -mfloat-abi=softfp
ARCH_COMPILEFLAGS :=  -mabi=aapcs-linux -mcpu=cortex-a7 -mfpu=vfpv3 -mfloat-abi=softfp
THUMBCFLAGS       := -mthumb -D__thumb__ -mthumb-interwork
WITH_LINKER_GC    := 1
LIBGCC := $(shell $(TOOLCHAIN_PREFIX)gcc $(GLOBAL_COMPILEFLAGS) $(ARCH_COMPILEFLAGS) $(THUMBCFLAGS) -print-libgcc-file-name)
TRUSTY_USER_ARCH := arm

ARCH_$(ARCH)_TOOLCHAIN_PREFIX := $(TOOLCHAIN_PREFIX)

