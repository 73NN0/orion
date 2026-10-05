.DEFAULT_GOAL := all

.PHONY: all clean test

LIBS := arena parser
TARGET := orionctl
include mk/common.mk

# ------------------------------------------------------------
# Bibliothèques indépendantes : sys/src/lib*
# ------------------------------------------------------------

define BUILD_LIB
LIB :=
SRCS :=
TESTS :=
include sys/src/lib$(1)/sources.mk
include mk/lib.mk
endef

$(foreach L,$(LIBS),$(eval $(call BUILD_LIB,$(L))))

# ------------------------------------------------------------
# Backend de plateforme : linux/, bsd/, etc.
# ------------------------------------------------------------

LIB :=
SRCS :=
TESTS :=

include $(PLATFORM)/sources.mk

ifneq ($(strip $(SRCS)),)
include mk/lib.mk
LINK_LIBS := $(LIBS) $(PLATFORM)
else
LINK_LIBS := $(LIBS)
endif

# ------------------------------------------------------------
# Outils de développement
# ------------------------------------------------------------

include mk/clangd.mk

# ------------------------------------------------------------
# Commandes
# ------------------------------------------------------------

all: $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET): \
	$(LINK_LIBS:%=$(LIB_DIR)/lib%.a) \
	sys/src/cmd/$(TARGET)/main.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) \
		sys/src/cmd/$(TARGET)/main.c \
		$(LDLIBS) \
		-o $@

test: $(ALL_TESTS)
	@for t in $^; do \
		echo "→ $$t"; \
		$$t || exit 1; \
	done

clean:
	rm -rf bin
	find sys -type f \( -name '*.o' -o -name '*.d' \) \
		-exec rm -f {} +
	@if [ -d "$(PLATFORM)" ]; then \
		find "$(PLATFORM)" -type f \
			\( -name '*.o' -o -name '*.d' \) \
			-exec rm -f {} +; \
	fi
