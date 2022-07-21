#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include <zmq.h>
#include <czmq.h>

int main()
{	
	void *context = zmq_ctx_new();
	void *publisher = zmq_socket(context, ZMQ_PUB);
	int rc = zmq_bind(publisher, "tcp://127.0.0.1:5557");
	assert(rc == 0);
	
	void *subscriber = zmq_socket(context, ZMQ_SUB);
	zmq_connect(subscriber, "tcp://127.0.0.1:5558");//endereço de exp 1
	zmq_connect(subscriber, "tcp://127.0.0.1:5559");//endereço de exp 2
	zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "mul:", 4);

	char str[128];
	while(1)
	{	
		float v1,v2, r;
		int id;
		char op;
		zmq_recv(subscriber, str, 128, 0);
		fprintf(stderr,"lido: %s\n", str);
		sscanf(str, "mul: %f %c %f %d", &v1, &op, &v2, &id);

		
		if(op == '*'){
			r = v1 * v2;
		}else if(op == '/'){
			r = v1 / v2;
		}

		fprintf(stderr,"calculado: %f %c %f =  %f\n", v1, op, v2, r);
		sprintf(str, "%d: %f",id,r);
		fprintf(stderr,"publicado: %s\n\n",str);
		rc = zmq_send(publisher, str, strlen(str)+1, 0);
		
	}

	zmq_close(publisher);
	zmq_close(subscriber);
	zmq_ctx_destroy(context);

	return 0;
}
