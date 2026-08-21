#include <stdio.h>
#include <stdlib.h>

#include "server.h"

#define STATIC_DIR "/home/deploy/dev/ipa-website/static/"

char *load_html(const char *filename, size_t *file_size) {
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
		if (feof(f)) {
			printf("End of file unexpectedly found.\n");
		} else if (ferror) {
			goto cleanup;
		}
	}
	buffer[size] = '\0';

	fclose(f);
	return buffer;

cleanup:
	perror("Error loading HTML file");
	if (f) fclose(f);
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}
	return buffer;
}

void static_response(char *path, char *content_type,  http_response_t *res) {
	char *body = NULL;
	size_t body_size = 0;

	body = load_html(path, &body_size);

	if (body) {
		res->status_code = 200;
		snprintf(res->reason_phrase, MAX_PHRASE_LEN, "OK");
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Context-Length");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "%zu", body_size);
		++res->next_header_idx;
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Context-Type");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "%s", content_type);
		++res->next_header_idx;
		res->body = body;
		res->body_size = body_size;
	} else {
		res->status_code = 500;
		snprintf(res->reason_phrase, MAX_PHRASE_LEN, "But why male models?");
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Context-Length");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "0");
		++res->next_header_idx;
	}
}

void handle_landing(const http_request_t *req, http_response_t *res) {
	static_response(STATIC_DIR "index.html", "text/html", res);
}

void handle_home(const http_request_t *req, http_response_t *res) {
	static_response(STATIC_DIR "hx/work_play.html", "text/html", res);
}

void handle_style(const http_request_t *req, http_response_t *res) {
	static_response(STATIC_DIR "style.css", "text/css", res);
}

int main(void) {
	route_t routes[] = {
		{"/", "GET", handle_landing},
		{"/hx/home", "GET", handle_home},
		{"/style.css", "GET", handle_style}
	};
	size_t route_count = sizeof(routes)/sizeof(route_t);
	app_init_t app_init = {
		.routes = routes,
		.route_count = route_count,
	};
	
	app(&app_init);
}
