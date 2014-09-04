/**
 * mmal demo utility for showing leakage issue
 *
 * cross build:
 * arm-linux-gcc \
 *   --sysroot=/path/to/your/sysroot \
 *   -lbcm_host -lmmal -lmmal_core -o mmal-leak mmal-leak.c
 */

#include <stdio.h>
#include <unistd.h>

#include <interface/mmal/mmal.h>
#include <interface/mmal/util/mmal_util.h>

#define MMAL_COMPONENT_DEFAULT_DEINTERLACE "vc.ril.image_fx"

static void output_port_cb(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
	printf("Output port cb\n");
}

int main()
{
	MMAL_COMPONENT_T *image_fx = NULL;
	MMAL_POOL_T *pool = NULL;
	MMAL_STATUS_T status;
	MMAL_PORT_T *output;
	MMAL_PORT_T *input;
	unsigned i;

	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_DEINTERLACE, &image_fx);
	if (status != MMAL_SUCCESS) {
		printf("Could not create image_fx component %s (status=%"PRIx32" %s)\n",
				MMAL_COMPONENT_DEFAULT_DEINTERLACE, status,
				mmal_status_to_string(status));
		return EXIT_FAILURE;
	}

	output = image_fx->output[0];
	input = image_fx->input[0];

	input->format->encoding = MMAL_ENCODING_OPAQUE;
	status = mmal_port_format_commit(input);
	if (status != MMAL_SUCCESS) {
		printf("Failed to commit format for input port %s (status=%"PRIx32" %s)",
			input->name, status, mmal_status_to_string(status));
		return EXIT_FAILURE;
	}

	status = mmal_port_enable(input, output_port_cb);
	if (status != MMAL_SUCCESS) {
		printf("Could not enable output port on %s (status=%"PRIx32" %s)\n",
				MMAL_COMPONENT_DEFAULT_DEINTERLACE, status,
				mmal_status_to_string(status));
		return EXIT_FAILURE;
	}

	mmal_format_full_copy(output->format, input->format);
	status = mmal_port_format_commit(output);
	if (status != MMAL_SUCCESS) {
		printf("Failed to commit format for input port %s (status=%"PRIx32" %s)",
			output->name, status, mmal_status_to_string(status));
		return EXIT_FAILURE;
	}

	output->buffer_size = output->buffer_size_recommended;
	output->buffer_num = output->buffer_num_recommended;
	status = mmal_port_enable(output, output_port_cb);
	if (status != MMAL_SUCCESS) {
		printf("Could not enable output port on %s (status=%"PRIx32" %s)\n",
				MMAL_COMPONENT_DEFAULT_DEINTERLACE, status,
				mmal_status_to_string(status));
		return EXIT_FAILURE;
	}

	status = mmal_component_enable(image_fx);
	if (status != MMAL_SUCCESS) {
		printf("Could not enable component %s (status=%"PRIx32" %s)\n",
				MMAL_COMPONENT_DEFAULT_DEINTERLACE, status,
				mmal_status_to_string(status));
		return EXIT_FAILURE;
	}

	printf("Components initialised...\n");
	usleep(10 * 1000 * 1000);

	printf("Allocate and free 10000 pools using default allocator...\n");
	for(i = 0; i < 10000; i++) {
		printf("Create pool %u, size %dx%d\n", i,
				output->buffer_num,
				output->buffer_size);
		if (pool)
			mmal_pool_destroy(pool);

		pool = mmal_pool_create(output->buffer_num,
				output->buffer_size);
		if (!pool) {
			printf("Could not create pool\n");
			return EXIT_FAILURE;
		}
	}
	printf("Done allocating and freeing 10000 pools using default allocator...\n");
	usleep(10 * 1000 * 1000);

	printf("Allocate and free 10000 pools using port payload allocator...\n");
	for(i = 0; i < 10000; i++) {
		printf("Create pool %u, size %dx%d\n", i,
				output->buffer_num,
				output->buffer_size);
		if (pool)
			mmal_pool_destroy(pool);

		pool = mmal_pool_create_with_allocator(output->buffer_num,
				output->buffer_size, output,
				(mmal_pool_allocator_alloc_t)mmal_port_payload_alloc,
				(mmal_pool_allocator_free_t)mmal_port_payload_free);
		if (!pool) {
			printf("Could not create pool\n");
			return EXIT_FAILURE;
		}
	}
	printf("Done allocating and freeing 10000 pools using port payload allocator...\n");
	usleep(10 * 1000 * 1000);

	return 0;
}
