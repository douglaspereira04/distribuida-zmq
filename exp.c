#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <zmq.h>
#include <czmq.h>

//Avaliação de expressão com ponto flutuante em C adaptada do código para inteiros em C++ de https://www.geeksforgeeks.org/expression-evaluation/
//Autor original https://auth.geeksforgeeks.org/user/nik1996

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))
#define top(sp) (*(sp-1))

void *context;
void *publisher;
void *subscriber_operacoes;
void *subscriber_contas;
char str_id[2];

int precedence(char op){
    if(op == '+'||op == '-')
    	return 1;
    if(op == '*'||op == '/')
    	return 2;
    return 0;
}

float applyOp(float a, float b, char op);

float evaluate(char* tokens, int len){
    int i;
     

    float v[1000];
    float* values = v;
    char o[1000];
    char* ops = o;
     
    for(i = 0; i < len; i++){
         
        if(tokens[i] == ' '){
            continue;
        } else if(tokens[i] == '('){
        	push(ops, tokens[i]);
        } else if(isdigit(tokens[i])){

            int integer = 0;
            int decimal = 0;
            float val = 0;
            
            while(i < len &&
                        isdigit(tokens[i]))
            {
                integer = (integer*10) + (tokens[i]-'0');
                i++;
            }

            int k = 0;
            if(tokens[i] == '.'){
            	i++;
            	while(i < len &&
                        isdigit(tokens[i]))
	            {
	                decimal = (decimal*10) + ((tokens[i]-'0'));
	                i++;
	                k++;
	            }

            }

            val = ((float) integer);
            if(k > 0){
            	val = val + ((float)decimal/(10.0*k));
            }

            push(values, val);
           
            i--;

        } else if(tokens[i] == ')') {
            while(ops != o && top(ops) != '(') {
                float val2 = pop(values);
                 
                float val1 = pop(values);
                 
                char op = pop(ops);
                 

            	push(values, applyOp(val1, val2, op));
            }
          
            if(ops != o){
                pop(ops);
        	}
        } else {
            while(ops != o && precedence(top(ops))
                                >= precedence(tokens[i])){
                float val2 = pop(values);
                 
                float val1 = pop(values);
                 
                char op = pop(ops);

                push(values, applyOp(val1, val2, op));
            }
             
            push(ops, tokens[i]);
        }
    }

    while(ops != o){

        float val2 = pop(values);
         
        float val1 = pop(values);
         
        char op = pop(ops);
                 
        push(values, applyOp(val1, val2, op));
    }
 
    return top(values);
}

float applyOp(float a, float b, char op){
	char str[128];
	float r = 0;
    switch(op){
        case '+': 
        case '-': 
        	//envia o str_id para saber para quem retornar
        	sprintf(str, "sum: %.6f %c %.6f %s", a, op, b, str_id);
  			break;
        case '*': 
        case '/': 
        	//envia o str_id para saber para quem retornar
        	sprintf(str, "mul: %.6f %c %.6f %s", a, op, b, str_id);
        	break;
    }
	fprintf(stderr, "publicado: %s\n", str);
	int rc = zmq_send(publisher, str, strlen(str)+1, 0);
	rc = zmq_recv(subscriber_operacoes, str, 128, 0);
	sscanf(str, "%*d: %f", &r);
	return r;
}
 
int main(int argc, char** argv) {

	//obtem o parametro que deve ser 1 ou 2
	sprintf(str_id, "%s", argv[1]);
	char char_id;
	sscanf(str_id, "%c", &char_id);
	str_id[1] = '\0';
	char filter[3];
	sprintf(filter, "%s:", str_id); 
	filter[2] = '\0';


    context = zmq_ctx_new();
	publisher = zmq_socket(context, ZMQ_PUB);
	char address[] = "tcp://127.0.0.1:555x\0"; 
	address[19] = char_id+7;
	//exp.out 1 será 5558 e exp.out 2 será 5559
	int rc = zmq_bind(publisher, address);
	
	subscriber_operacoes = zmq_socket(context, ZMQ_SUB);
	subscriber_contas = zmq_socket(context, ZMQ_SUB);
	zmq_connect(subscriber_operacoes, "tcp://127.0.0.1:5556");
	zmq_connect(subscriber_operacoes, "tcp://127.0.0.1:5557");
	zmq_connect(subscriber_contas, "tcp://127.0.0.1:5555");
	zmq_setsockopt(subscriber_operacoes, ZMQ_SUBSCRIBE, filter, 2);
	zmq_setsockopt(subscriber_contas, ZMQ_SUBSCRIBE, filter, 2);
	
	char str[128];
	char str_no_filter[512];
	while(1)
	{	
		usleep(10000);
		zmq_recv(subscriber_contas, str, 128, 0);
		printf("\nlido: %s\n", str);
		printf("\n%s =  %f\n",&str[3], evaluate(&str[3], strlen(&str[3])));
		fflush(stdout);
	}

	zmq_close(publisher);
	zmq_close(subscriber_operacoes);
	zmq_close(subscriber_contas);
	zmq_ctx_destroy(context);

	return 0;
}
