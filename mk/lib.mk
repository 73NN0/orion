ifeq ($(strip $(LIB)),)
$(error LIB is not defined before including mk/lib.mk)
endif

OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)
ARCHIVE := $(LIB_DIR)/lib$(LIB).a

$(ARCHIVE): $(OBJS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

# Tests : binaires à côté des sources de test
TEST_BINS := $(TESTS:.c=)

$(TEST_BINS): %: %.c $(ARCHIVE)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@

ALL_TESTS := $(ALL_TESTS) $(TEST_BINS)

-include $(DEPS)
