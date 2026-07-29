PYTHON ?= python3
VIDEO ?= ntsc
JOBS ?= 2

.PHONY: all toolchain build clean

all: build

toolchain:
	$(PYTHON) tools/bootstrap_ps2dev.py

build:
	$(PYTHON) tools/build.py --video $(VIDEO) --jobs $(JOBS)

clean:
	$(PYTHON) tools/build.py --video $(VIDEO) --clean --jobs $(JOBS)
