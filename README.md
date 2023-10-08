Instalação do ZeroMQ:
(ubuntu/mint)	
apt-get install libzmq3-dev
apt-get install libczmq-dev

	Instruções do site:
	https://zeromq.org/download/
	https://zeromq.org/languages/c/

Compilar:
gcc -o mul.out mul.c -lzmq
gcc -o sum.out sum.c -lzmq
gcc -o exp.out exp.c -lzmq
gcc -o conta.out conta.c -lzmq

Executar (cada um em um terminal diferente):
./mul.out
./sum.out
./exp.out 1
./exp.out 2
./conta.out <repetições>

repetições é o número de vezes que serão publicadas as contas do arquivo
	- ./conta.out 1 vai publicar uma vez todas as contas;
	- ./conta.out 100 vai publicar cem vezes todas as contas do arquivo.

Executar nesta ordem, já que exp precisa estar ativo quando conta publicar, e mul e sum precisam estar ativos quanto exp precisar resolver uma conta.

Os sistema resolve contas. Uma "conta" é uma expressão consistindo de somas, subtrações, multiplicações e divisões de operandos. A expressão é em formato infixo e possui prioridade de tipo de operador, ainda podendo apresentar parenteses.

Foram obtidas expressões através do código da ferramenta encontrada https://github.com/dshepsis/ExpressionGenerator

O sistema é composto por 5 processos:
	
	- Processo que gera contas, definido em "conta.c". Esse processo publica contas à serem resolvidas. A publicação tem um identificador numérico no início, para indicar a publicação à um resolvedor.
	
	- Processos que querem resolver as expressões, definidos em "exp.c". Esses processos sabem parcialmente resolver uma expressão em notação infixa. Eles tem um identificador cada (foram definidos somente 1 e 2) que, estando inscritas nas publicações de "conta.c", os permitem filtrar as expressões direcionadas à eles. Esses processos não sabem somar, subtrair, multiplicar ou dividir, somente sabem interpretar a notação infixa e organizar quais e em que ordem operações de 2 operandos deve ser feitas. Para efetivamente resolverem suas contas, as operações com 2 operandos são publicadas para que quem souber resolver, resolva e publique então a resposta.
	
	- Processo de soma. Sabe somar/subtrair uma expressão simples com 2 operandos. Está inscrito nas publicações dos processos resolvedores de expressão. Ele filtra por expressões de soma, sendo que a mensagem deve conter um identificador de quem enviou. Depois de resolver, a publicação será composta pelo identificador de quem solicitou a resposta, para permitir filtragem, e da resposta em sí.
	
	- Processo de multiplicação. Faz o mesmo que o processo de soma, mas com operadores multiplicativos.
