## HTTP Server - Networks (06-26951) 
This is my submission for the first Networks assignment, which was to build a server that would correctly interact with most standard (modern) browsers. 

### Usage
> make
> ./server < port > 

This will compile the code and start a server on the provided port. Any pages to be served should be added on a new line of _files.txt_  - any URI not listed in _files.txt_ will **not** be served.

The following HTTP verbs are supported by the server:
- GET 

### Behaviour
- When a null URI is requested, the server will return a 200 (OK) response with the index.html page as the body. With any other (servable) resource, the server will return a 200 (OK) response with the requested resource as the body.
- If a nonexistent (or non-servable) resource is requested, a 404 (File Not Found) response will be returned.
- If a request cannot be handled or is malformed, a 400 (Bad Request) response is returned.

In the future, there will be some differentiation implemented between nonexistent and non-servable resources, and a 403 error will be returned if the resource exists, but isn't whitelisted (as opposed to a 404).
