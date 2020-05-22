.PHONY: upload
upload:
	platformio run -t upload -e ota || platformio run -t upload -e serial
