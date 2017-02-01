/*
 * h2p based HTTP2 client. 
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <h2p/h2p.h>

#define LOG_AND_RETURN(m,v) { printf("ERROR: %s\n", m); return v; }
#define LOG_AND_EXIT(m) { printf("FATAL ERROR: %s\n", m); exit(1); }

typedef struct {
  const char *host;
  const char *path;
  size_t pathlen;
  const char *hostport;
  size_t hostlen;
  size_t hostportlen;
  uint16_t port;
} URI;

void h2_frame_cb(h2p_context *connection, uint32_t stream_id,
               h2p_frame_type type, const h2p_frame *frame) {

}

void h2_headers_cb(h2p_context *connection, uint32_t stream_id,
                 const nghttp2_headers *headers) {

}

int h2_data_started_cb(h2p_context *connection, uint32_t stream_id) {
	return 0;
}

void h2_data_cb(h2p_context *connection, uint32_t stream_id,
              const h2p_frame_data *data) {

}

void h2_data_finished_cb(h2p_context *connection, uint32_t stream_id,
              /*const h2p_frame *data,*/ uint32_t status) {

}

void h2_error_cb(h2p_context *context, h2p_error_type type, const char *msg) {

}

h2p_callbacks parser_callbacks = {
  h2_frame_cb,
  h2_headers_cb,
  h2_data_started_cb,
  h2_data_cb,
  h2_data_finished_cb,
  h2_error_cb
};

h2p_context *init_parser() {
  int status;
  h2p_context *parser;
  h2p_callbacks *callbacks = &parser_callbacks;

  status = h2p_init (callbacks, 1, &parser);
  if (status != 0) {
    LOG_AND_RETURN("Parser cannot be initialized.", NULL)
  }

	return parser;
}

void print_uri(const URI *uri) {
  if (uri == NULL) return;

	printf ("host: %s\n", uri->host);
	printf ("path: %s\n", uri->path);
	printf ("hostport: %s\n", uri->hostport);
	printf ("port: %d\n", uri->port);
}

int connect_to(const char *host, uint16_t port) {
  struct addrinfo hints;
  int fd = -1;
  int rv;
  char service[NI_MAXSERV];
  struct addrinfo *res, *rp;
  snprintf(service, sizeof(service), "%u", port);
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  rv = getaddrinfo(host, service, &hints, &res);
  if (rv != 0) {
    LOG_AND_EXIT(gai_strerror(rv))
  }
  for (rp = res; rp; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd == -1) {
      continue;
    }
    while ((rv = connect(fd, rp->ai_addr, rp->ai_addrlen)) == -1 &&
           errno == EINTR)
      ;
    if (rv == 0) {
      break;
    }
    close(fd);
    fd = -1;
  }
  freeaddrinfo(res);
  return fd;
}

int get_uri(const URI *uri) {
  int fd;
  int status;

  fd = connect_to(uri->host, uri->port);
  if (fd < 0) {
    LOG_AND_EXIT("Connection to host failed\n")
  }

  shutdown(fd, SHUT_WR);
  close(fd);

  return status;
}

int parse_uri(URI *res, const char *uri) {
  /* We only interested in http */
  size_t len, i, offset;
  int ipv6addr = 0;
  memset(res, 0, sizeof(URI));
  len = strlen(uri);
  if (len < 9 || memcmp("http://", uri, 7) != 0) {
    return -1;
  }
  offset = 7;
  res->host = res->hostport = &uri[offset];
  res->hostlen = 0;
  if (uri[offset] == '[') {
    /* IPv6 literal address */
    ++offset;
    ++res->host;
    ipv6addr = 1;
    for (i = offset; i < len; ++i) {
      if (uri[i] == ']') {
        res->hostlen = i - offset;
        offset = i + 1;
        break;
      }
    }
  } else {
    const char delims[] = ":/?#";
    for (i = offset; i < len; ++i) {
      if (strchr(delims, uri[i]) != NULL) {
        break;
      }
    }
    res->hostlen = i - offset;
    offset = i;
  }
  if (res->hostlen == 0) {
    return -1;
  }
  /* Assuming http */
  res->port = 80;
  if (offset < len) {
    if (uri[offset] == ':') {
      /* port */
      const char delims[] = "/?#";
      int port = 0;
      ++offset;
      for (i = offset; i < len; ++i) {
        if (strchr(delims, uri[i]) != NULL) {
          break;
        }
        if ('0' <= uri[i] && uri[i] <= '9') {
          port *= 10;
          port += uri[i] - '0';
          if (port > 65535) {
            return -1;
          }
        } else {
          return -1;
        }
      }
      if (port == 0) {
        return -1;
      }
      offset = i;
      res->port = (uint16_t)port;
    }
  }
  res->hostportlen = (size_t)(uri + offset + ipv6addr - res->host);
  for (i = offset; i < len; ++i) {
    if (uri[i] == '#') {
      break;
    }
  }
  if (i - offset == 0) {
    res->path = "/";
    res->pathlen = 1;
  } else {
    res->path = &uri[offset];
    res->pathlen = i - offset;
  }
  return 0;
}

int main(int argc, char *argv[]) {
	int status;
	URI uri;

	if (argc != 2) {
		LOG_AND_EXIT("Invalid parameter count.")
	}

	status = parse_uri(&uri, argv[1]);
	if (status != 0) {
		LOG_AND_EXIT("Invalid parameter.")
	}

	return get_uri(&uri);
}