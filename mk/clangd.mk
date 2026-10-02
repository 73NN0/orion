.PHONY: clangd-flags
clangd-flags:
	@{ \
		for flag in $(CPPFLAGS) $(CFLAGS); do \
			printf '%s\n' "$$flag"; \
		done; \
	} > compile_flags.txt
	@printf '%s\n' "generated compile_flags.txt"


.PHONY: compdb 
compdb:
	@command -v bear >/dev/null 2>&1 || { \
		printf '%s\n' "error: bear is required to generate compile_commands.json"; \
		exit 1; \
	}
	@$(MAKE) clean
	@bear -- $(MAKE) all
	@printf '%s\n' "generated compile_commands.json"


.PHONY : clangd-clean
clangd-clean:
	rm -f compile_flags.txt compile_commands.json