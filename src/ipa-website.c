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

void handle_home(const http_request_t *req, http_response_t *res) {
	char *body = NULL;
	size_t body_len = 0;

	body = load_html(STATIC_DIR "index.html", &body_len);

	if (body) {
		res->status_code = 200;
		snprintf(res->headers[0].key, MAX_HEADER_KEY_LEN, "Context-Length");
		snprintf(res->headers[0].val, MAX_HEADER_VAL_LEN, "%zu", body_len);
		res->body = body;
		res->body_len = body_len;
	}
}

int main(void) {
	route_t routes[] = {
		{"GET", "/", handle_home}
	};
	size_t route_count = 1;
	
	app(routes, route_count);
}
