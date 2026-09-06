# AGENTS.md — ipa-website

## Build & Run
- `make` — compiles `bin/ipa-website` from `src/ipa-website.c` and `vine-street/server.c` via `gcc -g -O0`.
- `./bin/ipa-website` — runs the server on `127.0.0.1:1362`.
- There are **no tests, no CI, and no package manager**. Verify manually with `curl`.

## Architecture
- **`src/ipa-website.c`** — application layer: defines routes, handlers, and static file responses. Edit this to add pages or change routing.
- **`vine-street/server.c`** and **`vine-street/server.h`** — custom async HTTP server runtime (epoll + eventfd + pthreads). Hardcoded to `PORT 1362`, `NUM_WORKER_THREADS 7`, `ADDRESS INADDR_LOOPBACK`.
- `src/ipa-website.c` defines a **hardcoded absolute path** for static assets:
  ```c
  #define STATIC_DIR "/home/deploy/dev/ipa-website/static/"
  ```
  If the repository is moved to a different directory, the binary will fail to load static files unless this macro is updated and the project is rebuilt.

## Frontend Architecture
- **HTMX forward approach**: `.page` is the persistent shell. Navigation swaps content into `#content-container` via `hx-target`. The ASCII art background and page shell remain intact during HTMX requests.

## vine-street Repository Boundary
- `vine-street/` is a **nested git repository**, not a submodule. It has its own `.git` directory and is ignored by the parent repo (`ipa-website/.gitignore`).
- **Changes inside `vine-street/` are tracked by the `vine-street` repo, not `ipa-website`.** Do not assume parent-repo commits will include `vine-street` source changes.

## Adding a Route
1. Implement a handler in `src/ipa-website.c` matching `void handle_x(const http_request_t *req, http_response_t *res)`.
2. Add a `route_t` entry to the `routes[]` array in `main()`.
3. Run `make`.
4. Test with `curl http://127.0.0.1:1362/<path>`.
