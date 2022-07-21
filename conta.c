#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include <zmq.h>
#include <czmq.h>

#include <stdlib.h>

int main(int argc, char** argv)
{	

	int loop_times;
	sscanf(argv[1], "%d", &loop_times);

	void *context = zmq_ctx_new();
	void *publisher = zmq_socket(context, ZMQ_PUB);
	int rc = zmq_bind(publisher, "tcp://127.0.0.1:5555");
	assert(rc == 0);

	//'evitar' q send ocorra antes de bind
	sleep(1);


	for (int i = 0; i < loop_times; ++i)
	{
		FILE * fp;
		char * line = NULL;
		size_t len = 0;
		ssize_t read;

		fp = fopen("contas", "r");
		if (fp == NULL){
			exit(EXIT_FAILURE);
		}

		while ((read = getline(&line, &len, fp)) != -1) {
			usleep(10000);
			line[read-1] = '\0';
			fprintf(stderr,"publicado: %s\n", line);
			rc = zmq_send(publisher, line, read, 0);
		}

		fclose(fp);
		if (line){
			free(line);
		}
	}

	zmq_close(publisher);
	zmq_ctx_destroy(context);

	return 0;
}
