BUILD_DIR := build
NAME := lai

interpreter:
	@ $(MAKE) -f c.make NAME=$(NAME) MODE=release SOURCE_DIR=c
	@ cp build/$(NAME) $(NAME)

debug:
	@ $(MAKE) -f c.make NAME=$(NAME) MODE=debug SOURCE_DIR=c
	@ cp build/$(NAME) $(NAME)

clean:
	@ rm -rf $(BUILD_DIR)
	@ rm -rf gen

.PHONY: clean interpreter debug
