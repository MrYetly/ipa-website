#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "server.h"

#define DEFAULT_STATIC_DIR "/home/deploy/dev/ipa-website/static/"
static char static_dir[2048] = DEFAULT_STATIC_DIR;

static void build_static_path(char *out, size_t out_size, const char *relative) {
	snprintf(out, out_size, "%s%s", static_dir, relative);
}
#define MAX_VIEWBOX_LEN 50
#define VIEWBOX_MATCH "{{viewbox_width_height}}"
#define ASCII_ART_MATCH "{{ascii_art}}"
#define TSPAN_OPEN "<tspan x=\"0\" dy=\"1em\">"
#define TSPAN_CLOSE "</tspan>"
#define TSPAN_OVERHEAD_PER_LINE (sizeof(TSPAN_OPEN) - 1 + sizeof(TSPAN_CLOSE) - 1)

char *load_text_file(const char *filename, size_t *file_size) {
	FILE *f = NULL;
	char *buffer = NULL;
	long size;

	f = fopen(filename, "rb");
	if (!f) goto cleanup;

	//get file length
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	*file_size = (size_t) size;
	buffer = malloc(size + 1);
	if (!buffer) goto cleanup;

	long bytes_read = fread(buffer, 1, *file_size, f);
	if (bytes_read < (long) *file_size) {
		goto cleanup;
	}
	buffer[size] = '\0';

	fclose(f);
	return buffer;

cleanup:
	perror("\nError loading HTML file");
	if (f) fclose(f);
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}
	return buffer;
}

void static_response(const char *path, const char *content_type, http_response_t *res) {
	char *body = NULL;
	size_t body_size = 0;

	//body freed by server
	body = load_text_file(path, &body_size);

	if (body) {
		res->status_code = 200;
		snprintf(res->reason_phrase, MAX_PHRASE_LEN, "OK");
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Length");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "%zu", body_size);
		++res->next_header_idx;
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Type");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "%s", content_type);
		++res->next_header_idx;
		res->body = body;
		res->body_size = body_size;
	} else {
		res->status_code = 500;
		snprintf(res->reason_phrase, MAX_PHRASE_LEN, "But why male models?");
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Length");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "0");
		++res->next_header_idx;
	}
}

int is_htmx_request(const http_request_t *req) {
	for (int i = 0; i < req->next_header_idx; ++i) {
	if (strcasecmp(req->headers[i].key, "HX-Request") == 0 &&
	    strcasecmp(req->headers[i].val, "true") == 0) {
			return 1;
		}
	}
	return 0;
}

void page_response(const http_request_t *req, http_response_t *res, const char *snippet_path) {
	if (is_htmx_request(req)) {
		static_response(snippet_path, "text/html", res);
	} else {
		char path[2048];
		build_static_path(path, sizeof(path), "index.html");
		static_response(path, "text/html", res);
	}
}

void handle_index(const http_request_t *req, http_response_t *res) {
	res->status_code = 302;
	snprintf(res->reason_phrase, MAX_PHRASE_LEN, "Found");
	snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Location");
	snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "/landing");
	++res->next_header_idx;
	snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Length");
	snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "0");
	++res->next_header_idx;
}

void handle_landing(const http_request_t *req, http_response_t *res) {
	char path[2048];
	build_static_path(path, sizeof(path), "hx/landing.html");
	page_response(req, res, path);
}

void handle_home(const http_request_t *req, http_response_t *res) {
	char path[2048];
	build_static_path(path, sizeof(path), "hx/home.html");
	page_response(req, res, path);
}

void handle_this_website(const http_request_t *req, http_response_t *res) {
	char path[2048];
	build_static_path(path, sizeof(path), "hx/this-website.html");
	page_response(req, res, path);
}

void handle_my_interests(const http_request_t *req, http_response_t *res) {
	char path[2048];
	build_static_path(path, sizeof(path), "hx/my-interests.html");
	page_response(req, res, path);
}

void get_ascii_art_metadata(char *buffer, size_t buffer_size, int *width, int *height) {
	*width = 0;
	*height = 0;
	if (!buffer || buffer_size == 0) return;

	size_t i = 0;
	while (i < buffer_size && buffer[i] != '\n') {
		(*width)++;
		i++;
	}
	if (*width == 0) return;
	*height = (int)(buffer_size / *width);
}

void handle_ascii_art_portrait(const http_request_t *req, http_response_t *res) {
	char *ascii_art = NULL;
	size_t ascii_art_size = 0;
	char *div = NULL;
	size_t div_size = 0;
	char *body = NULL;
	size_t body_size = 0;
	int width = 0;
	int height = 0;
	char viewbox_width_height[MAX_VIEWBOX_LEN];
	char ascii_path[2048];
	char div_path[2048];
	build_static_path(ascii_path, sizeof(ascii_path), "ascii-art/portrait-for-background-ascii-art.txt");
	build_static_path(div_path, sizeof(div_path), "hx/ascii-art.html");

	ascii_art = load_text_file(ascii_path, &ascii_art_size);
	if (!ascii_art) {
		res->status_code = 500;
		snprintf(res->reason_phrase, MAX_PHRASE_LEN, "Internal Server Error");
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Length");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "0");
		++res->next_header_idx;
		return;
	}
	get_ascii_art_metadata(ascii_art, ascii_art_size, &width, &height);

	div = load_text_file(div_path, &div_size);
	if (!div) {
		free(ascii_art);
		res->status_code = 500;
		snprintf(res->reason_phrase, MAX_PHRASE_LEN, "Internal Server Error");
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Length");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "0");
		++res->next_header_idx;
		return;
	}
	
	//monospace character boxes typically have an aspect ratio around 0.6:1 (e.g. Courier New)
	snprintf(
			viewbox_width_height,
			MAX_VIEWBOX_LEN,
			"%d %d",
			(width * 6) / 10, height); 

	int line_count = 0;
	for (size_t i = 0; i < ascii_art_size; i++) {
		if (ascii_art[i] == '\n') line_count++;
	}
	if (ascii_art_size > 0 && ascii_art[ascii_art_size - 1] != '\n') {
		line_count++;
	}

	body_size = ascii_art_size + div_size + strlen(viewbox_width_height) + (line_count * TSPAN_OVERHEAD_PER_LINE);
	body = malloc(body_size + 1);
	if (!body) {
		free(ascii_art);
		free(div);
		res->status_code = 500;
		snprintf(res->reason_phrase, MAX_PHRASE_LEN, "Internal Server Error");
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Length");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "0");
		++res->next_header_idx;
		return;
	}
	char *ptr = body;

	//build div
	for (size_t i = 0; i < div_size; i++) {
		if (i + 1 < div_size && *(div + i) == '{' && *(div + i + 1) == '{') {
			if (memcmp(VIEWBOX_MATCH, div + i, strlen(VIEWBOX_MATCH)) == 0) {
				size_t len = strlen(viewbox_width_height);
				memcpy(ptr, viewbox_width_height, len);
				ptr += len;
				i += strlen(VIEWBOX_MATCH) - 1;
			} else if (memcmp(ASCII_ART_MATCH, div + i, strlen(ASCII_ART_MATCH)) == 0) {
				size_t art_idx = 0;
				int in_tspan = 0;
				while (art_idx < ascii_art_size) {
					if (!in_tspan) {
						memcpy(ptr, TSPAN_OPEN, sizeof(TSPAN_OPEN) - 1);
						ptr += sizeof(TSPAN_OPEN) - 1;
						in_tspan = 1;
					}
					char c = ascii_art[art_idx++];
					if (c == '\n') {
						memcpy(ptr, TSPAN_CLOSE, sizeof(TSPAN_CLOSE) - 1);
						ptr += sizeof(TSPAN_CLOSE) - 1;
						in_tspan = 0;
					} else if (c != '\r') {
						*ptr++ = c;
					}
				}
				if (in_tspan) {
					memcpy(ptr, TSPAN_CLOSE, sizeof(TSPAN_CLOSE) - 1);
					ptr += sizeof(TSPAN_CLOSE) - 1;
				}
				i += strlen(ASCII_ART_MATCH) - 1;
			} else {
				*ptr = *(div + i);
				ptr++;
			}
		} else {
			*ptr = *(div + i);
			ptr++;
		}
	}
	*ptr = '\0';
	size_t actual_body_size = ptr - body;

	free(ascii_art);
	free(div);

	res->status_code = 200;
	snprintf(res->reason_phrase, MAX_PHRASE_LEN, "OK");
	snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Length");
	snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "%zu", actual_body_size);
	++res->next_header_idx;
	snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Type");
	snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "text/html");
	++res->next_header_idx;
	res->body = body;
	res->body_size = actual_body_size;
}

void handle_style(const http_request_t *req, http_response_t *res) {
	char path[2048];
	build_static_path(path, sizeof(path), "style.css");
	static_response(path, "text/css", res);
}

void handle_script(const http_request_t *req, http_response_t *res) {
	char path[2048];
	build_static_path(path, sizeof(path), "script.js");
	static_response(path, "text/javascript", res);
}

int main(int argc, char *argv[]) {
	int opt;
	int port = 0;

	while ((opt = getopt(argc, argv, "p:s:h")) != -1) {
		switch (opt) {
			case 'p':
				port = atoi(optarg);
				break;
			case 's': {
				size_t len = strlen(optarg);
				if (len >= sizeof(static_dir) - 1) {
					fprintf(stderr, "Static directory path too long\n");
					exit(EXIT_FAILURE);
				}
				strcpy(static_dir, optarg);
				if (len > 0 && static_dir[len - 1] != '/') {
					static_dir[len] = '/';
					static_dir[len + 1] = '\0';
				}
				break;
			}
			case 'h':
				printf("Usage: %s [-p port] [-s static_dir]\n", argv[0]);
				exit(EXIT_SUCCESS);
			default:
				fprintf(stderr, "Usage: %s [-p port] [-s static_dir]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	route_t routes[] = {
		{"/", "GET", handle_index},
		{"/landing", "GET", handle_landing},
		{"/home", "GET", handle_home},
		{"/this-website", "GET", handle_this_website},
		{"/my-interests", "GET", handle_my_interests},
		{"/ascii-art-portrait", "GET", handle_ascii_art_portrait},
		{"/script.js", "GET", handle_script},
		{"/style.css", "GET", handle_style}
	};
	size_t route_count = sizeof(routes)/sizeof(route_t);
	app_init_t app_init = {
		.routes = routes,
		.route_count = route_count,
		.port = port,
	};

	app(&app_init);
}
