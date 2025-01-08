.PHONY: all build run

all: build


build:
	cargo build --release

install:
	cargo install --path .

run: build
	./target/debug/http_server_rs --files www/
