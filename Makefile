BUILD_DIR := build
NAME := lai

debug:
	@ $(MAKE) -f c.make NAME=$(NAME) MODE=debug SOURCE_DIR=c
	@ cp build/$(NAME) $(NAME)

clean:
	@ rm -rf $(BUILD_DIR)
	@ rm -rf gen

interpreter:
	@ $(MAKE) -f c.make NAME=$(NAME) MODE=release SOURCE_DIR=c
	@ cp build/$(NAME) $(NAME)


.PHONY: clean interpreter debug
