.PHONY = all
all:
	platformio run

.PHONY = upload
upload:
	platformio run -t upload
