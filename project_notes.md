# Project Dev

- implement callback/handler interface design
- learn some styling with tailwind
- get templates working
- get htmx working (do I need to get the server to be async first?)

## callback/handler interface design

**Callback**: a function pointer passed as an argument into another piece of code so it can be executed later. Allows the main server loop to just pass data onto the function to be executed, instead of implementing that logic in the loop.

```C
// Define standard HTTP request and response structures
typedef struct { /* path, method, headers, body */ } http_request_t;
typedef struct { /* status_code, headers, body */ } http_response_t;

// Define a function pointer type for all route handlers
typedef void (*http_handler_t)(const http_request_t *req, http_response_t *res);
```

The first two typedefs are an example of how we structure variables that contain HTTP requests and responses. We define new types, which are structs, for these variables, to ensure those variables have a certain structure. For example, a request should have a path and method of type character array, headers of type array of character arrays, and a body that could be any number of types.

The third typedef is the type of the callback itself. We create a type of function pointer named `http_handler_t`. Function pointers of this type point to functions that always take two pointers as arguments, `req` and `res`, and these pointers always point to structsof type `http_request_t` and `http_response_t`, respectively.

**Handler**: the function associated with the callback, the function pointer. Here is where the backend logic is implemented.

```C
void handle_home(const http_request_t *req, http_response_t *res) {
    // Logic for GET /
}

void handle_not_found(const http_request_t *req, http_response_t *res) {
    // Logic for 404 Not Found
}
```

These are the functions that handle the server's response to a request. When the user accesses a certain path, like `/` or `/login`, a function exists for that route and handles it. If there is an error, another function handles surfacing that response to the user. 


**Route Table**: map URI paths to callback functions.

```C
typedef struct {
    const char *method;
    const char *path;
    http_handler_t handler;
} Route;

Route routes[] = {
    {"GET", "/", handle_home},
    {"GET", "/about", handle_about}
};
```

The route table is an array of `Route`s. Each `Route` tells how to process a kind of request sent to a certain path, by specifying what handler is to be called.

**Dispatch Loop**: the while loop that monitors the socket for client connections and requests, and brings responses from a route to the client connection.

