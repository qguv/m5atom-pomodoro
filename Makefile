.PHONY: all
all:
	platformio run -e featheresp32

.PHONY: upload
upload:
	platformio run -e featheresp32 -t upload

.PHONY: test
test:
	platformio test -e native
