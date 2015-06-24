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
#include "interface/mmal/util/mmal_default_components.h"

#define MMAL_COMPONENT_DEFAULT_DEINTERLACE "vc.ril.image_fx"

int main()
{
	MMAL_COMPONENT_T *comp = NULL;
	MMAL_POOL_T *pool = NULL;
	MMAL_STATUS_T status;
	MMAL_PORT_T *port;
	unsigned i;

	status = mmal_component_create("null_sink", &comp);
	if (status != MMAL_SUCCESS) {
		printf("Could not create image_fx component %s (status=%"PRIx32" %s)\n",
				MMAL_COMPONENT_DEFAULT_DEINTERLACE, status,
				mmal_status_to_string(status));
		return EXIT_FAILURE;
	}
    assert(comp);

	port = comp->input[0];

    int count = 10;
	printf("Allocate and free %d pools using port payload allocator...\n", count);
	for(i = 0; i < count; i++) {
		if (pool)
			mmal_pool_destroy(pool);

		pool = mmal_pool_create_with_allocator(10, 256, port,
				(mmal_pool_allocator_alloc_t)mmal_port_payload_alloc,
				(mmal_pool_allocator_free_t)mmal_port_payload_free);
		if (!pool) {
			printf("Could not create pool");
			return EXIT_FAILURE;
		}
	}
    mmal_pool_destroy(pool);
	printf("Done allocating and freeing %d pools using port payload allocator...\n", count);
    mmal_component_destroy(comp);
    printf("Destroyed component\n");

	return 0;
}
