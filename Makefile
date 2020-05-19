.PHONY: all
all:
	platformio run -e m5stack-core-esp32

.PHONY: upload
upload:
	platformio run -e m5stack-core-esp32 -t upload

.PHONY: test
test:
	platformio test -e native
