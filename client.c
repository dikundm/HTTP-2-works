/*
 * h2p based HTTP2 client. 
 */
#include <h2p.h>

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

int set_callbacks() {
	return 0;
}


int main(int argc, char *argv[]) {
	return 0;
}