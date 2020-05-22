.PHONY: all
all:
	platformio run -e serial

.PHONY: upload
upload:
	platformio run -e serial -t upload

.PHONY: test
test:
	platformio test -e native
