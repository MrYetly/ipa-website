#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

#define STATIC_DIR "/home/deploy/dev/ipa-website/static/"

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
		if (strcmp(req->headers[i].key, "HX-Request") == 0 &&
		    strcmp(req->headers[i].val, "true") == 0) {
			return 1;
		}
	}
	return 0;
}

void page_response(const http_request_t *req, http_response_t *res, const char *snippet_path) {
	if (is_htmx_request(req)) {
		static_response(snippet_path, "text/html", res);
	} else {
		static_response(STATIC_DIR "index.html", "text/html", res);
	}
}

char *create_ascii_art_background_div(char *ascii_art_txt_path, size_t *file_size) {
	size_t div_size;
	size_t ascii_art_size;
	char *div = load_file(STATIC_DIR "hx/ascii-art-background.html", &div_size);
	char *ascii_art = load_file(STATIC_DIR "ascii-art/portrait-for-background-ascii-art.txt", &ascii_art_size);

	*file_size = div_size + ascii_art_size;
	char *hx_element = malloc(file_size + 1); //leave room for \0 incase we want to print

	char *div_ptr = div;
	char *hx_element_ptr = hx_element;

	for (int i = 0; i < (int *)div_size; i++) {
		*hx_element_ptr = *div_ptr;
		if (*div_ptr == '>' && (div_ptr-div) != (int *)div_size) {
			memcpy(++hx_element_ptr, ascii_art, ascii_art_size);
			hx_element_ptr += (int *)ascii_art_size;
		}  else {
			hx_element_ptr++;
		}
		div_ptr++;
	}
	*(hx_element + filesize) = '\0';
	free(div);
	free(ascii_art);
	return hx_element
}

void ascii_art_background_response(const char *art_path, http_response_t *res) {
	char *body = NULL;
	size_t body_size = 0;

	body = create_ascii_art_background_div(art_path, &body_size);

	if (body) {
		res->status_code = 200;
		snprintf(res->reason_phrase, MAX_PHRASE_LEN, "OK");
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Length");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "%zu", body_size);
		++res->next_header_idx;
		snprintf(res->headers[res->next_header_idx].key, MAX_HEADER_KEY_LEN, "Content-Type");
		snprintf(res->headers[res->next_header_idx].val, MAX_HEADER_VAL_LEN, "%s", "text/html");
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
	page_response(req, res, STATIC_DIR "hx/landing.html");
}

void handle_home(const http_request_t *req, http_response_t *res) {
	page_response(req, res, STATIC_DIR "hx/home.html");
}

void handle_work(const http_request_t *req, http_response_t *res) {
	page_response(req, res, STATIC_DIR "hx/work.html");
}

void handle_play(const http_request_t *req, http_response_t *res) {
	page_response(req, res, STATIC_DIR "hx/play.html");
}

void handle_this_website(const http_request_t *req, http_response_t *res) {
	page_response(req, res, STATIC_DIR "hx/this-website.html");
}

void handle_my_interests(const http_request_t *req, http_response_t *res) {
	page_response(req, res, STATIC_DIR "hx/my-interests.html");
}

void handle_ascii_art_portrait(const http_request_t *req, http_response_t *res) {
	static_response(STATIC_DIR "ascii-art/portrait-for-background-ascii-art.txt", "text/plain", res);
}

void handle_style(const http_request_t *req, http_response_t *res) {
	static_response(STATIC_DIR "style.css", "text/css", res);
}

void handle_script(const http_request_t *req, http_response_t *res) {
	static_response(STATIC_DIR "script.js", "text/javascript", res);
}

int main(void) {
	route_t routes[] = {
		{"/", "GET", handle_index},
		{"/landing", "GET", handle_landing},
		{"/home", "GET", handle_home},
		{"/work", "GET", handle_work},
		{"/play", "GET", handle_play},
		{"/this-website", "GET", handle_this_website},
		{"/my-interests", "GET", handle_my_interests},
		{"/ascii-art/portrait", "GET", handle_portrait},
		{"/ascii-art/this-website", "GET", handle_ascii_art},
		{"/script.js", "GET", handle_script},
		{"/style.css", "GET", handle_style}
	};
	size_t route_count = sizeof(routes)/sizeof(route_t);
	app_init_t app_init = {
		.routes = routes,
		.route_count = route_count,
	};
	
	app(&app_init);
}
