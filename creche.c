/*
	Problema da creche descrito no livro The Little Book of Semaphores, Allen B. Downey
	Código desenvolvido pelo aluno Luiz Henrique Yuji Delgado Oda, RA 247255
*/

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N_PESSOAS 10

/*
	Variáveis que guardam informações sobre crianças e adultos.
	crianca_creche - quantidade de crianças dentro da creche no momento.
	adulto_creche - quantidade de adultos dentro da creche no momento.
	adulto_esperando - quantidade de adultos esperando para sair da creche.
	crianca_esperando - quantidade de crianças esperando para entrar na creche.
	id_pessoas[N_PESSOAS] - guarda o id de cada pessoa.
	tipo_pessoas[N_PESSOAS] - guarda se a pessoa é uma criança(1) ou um adulto(0).
	status[N_PESSOAS] - guarda se a pessoa está esperando para entrar na creche(1), dentro da creche(2),
	se ela já entrou e saiu da creche(3) ou se ela nem mesmo apareceu ainda(0). 
*/

int crianca_creche, adulto_creche, adulto_esperando, crianca_esperando;

int id_pessoas[N_PESSOAS], tipo_pessoas[N_PESSOAS];

int status[N_PESSOAS];

/*
	Semáforos com diversas funções
	protect - proteje a região crítica da thread
	fila_adulto - sinaliza quando uma criança sai da creche e possibilita a saída de um adulto que estava esperando para sair
	fila_crianca - sinaliza quando um adulto entra na creche e possibilita a entrada de uma criança que estava esperando para entrar
	imprime - controla a impressão do estado da aplicação
*/

sem_t protect, fila_adulto, fila_crianca, imprime;

/* Função para impressão da fila de crianças esperando para entrar na creche e dos adultos e crianças dentro da creche */
void prt() {
	/* Imprime adultos como A + id e crianças como C + id */
	/* Imprime as crianças na fila para entrar na creche */
	printf("Fila:\n");
	for(int i = 0; i < N_PESSOAS; i++) {
		if(status[i] == 1) {
			if(tipo_pessoas[i] == 0) printf("A%d ", i);
			else printf("C%d ", i);
		}
	}
	printf("\n");
	/* Imprime a cerca da creche */
	printf("Creche:\n");
	/* Calcula o tamanho do maior id e padroniza a impressão para id's menores */
	int sz = 1;
	int at = N_PESSOAS - 1;
	while(at > 0) {
		sz++; at /= 10;
	}
	for(int i = 0; i < N_PESSOAS * sz + N_PESSOAS + 2; i++) {
		printf("-");
	}
	printf("\n");
	printf("|");
	/* Imprime adultos */
	for(int i = 0; i < N_PESSOAS; i++) {
		if(status[i] == 2) {
			if(tipo_pessoas[i] == 0) {
				printf("A%d ", i);
				int size = sz - 1;
				at = i;
				if(at == 0) {
					size--;
				}
				while(at > 0) {
					at /= 10; size--;
				}
				/* Imprime espaços em branco para manter o retângulo correto */
				for(int j = 0; j < size; j++) {
					printf(" ");
				}
			}
			else {
				/* Imprime espaços em branco para manter o retângulo correto */
				printf(".");
				for(int j = 0; j < sz; j++) {
					printf(" ");
				}
			}
		}
		else {
			/* Imprime espaços em branco para manter o retângulo correto */
			printf(".");
			for(int j = 0; j < sz; j++) {
				printf(" ");
			}
		}
	}
	printf("|");
	printf("\n");
	printf("|");
	/* Imprime crianças */
	for(int i = 0; i < N_PESSOAS; i++) {
		if(status[i] == 2) {
			if(tipo_pessoas[i] == 1) {
				printf("C%d ", i);
				int size = sz - 1;
				at = i;
				if(at == 0) {
					size--;
				}
				while(at > 0) {
					at /= 10; size--;
				}
				/* Imprime espaços em branco para manter o retângulo correto */
				for(int j = 0; j < size; j++) {
					printf(" ");
				}
			}
			else {
				/* Imprime espaços em branco para manter o retângulo correto */
				printf(".");
				for(int j = 0; j < sz; j++) {
					printf(" ");
				}
			}
		}
		else {
			/* Imprime espaços em branco para manter o retângulo correto */
			printf(".");
			for(int j = 0; j < sz; j++) {
				printf(" ");
			}
		}
	}
	printf("|");
	printf("\n");
	for(int i = 0; i < N_PESSOAS * sz + N_PESSOAS + 2; i++) {
		printf("-");
	}
	printf("\n");
	/* Impressão daqueles que já passaram pela creche e foram embora */
	printf("Foi embora:\n");
	for(int i = 0; i < N_PESSOAS; i++) {
		if(status[i] == 3) {
			if(tipo_pessoas[i] == 0) printf("A%d ", i);
			else printf("C%d ", i);
		}
	}
	printf("\n");
	printf("\n\n");
	/* Imprime possíveis prints que ficaram no buffer */
	fflush(stdout);
}

/*Thread dos adultos*/
void* f_adulto(void *v) {
	int id = *(int*) v;
	sem_wait(&protect);

	/* Insere o adulto na creche, muda estado e imprime estado da creche */
	adulto_creche++;
	sem_wait(&imprime);
	status[id] = 2;
	prt();
	sem_post(&imprime);
	
	/* Verifica se existem crianças na fila e se existirem, sinaliza que elas podem entrar (no máximo 3) por meio
	do semáforo fila_crianca */
	if(crianca_esperando > 0) {
		int mn = crianca_esperando;
		if(mn > 3) mn = 3;
		for(int i = 0; i < mn; i++) {
			crianca_esperando--;
			crianca_creche++;
			sem_post(&fila_crianca);
		}
	}
	sem_post(&protect);

	sem_wait(&protect);

	/* Verifica se o adulto pode sair da creche, ou seja, se mesmo com ele saindo ainda temos pelo menos 3
	   adultos para cada criança. Caso seja possível, saímos da creche e imprimimos o estado dela. Caso não
	   seja, esperamos sinalização do semáforo fila_adulto e após receber sinal saímos da creche e imprimimos.*/
	if(crianca_creche <= 3 * (adulto_creche - 1)) {
		adulto_creche--;
		sem_wait(&imprime);
		status[id] = 3;
		prt();
		sem_post(&imprime);
		sem_post(&protect);
	}
	else {
		adulto_esperando++;
		sem_post(&protect);
		sem_wait(&fila_adulto);
		sem_wait(&imprime);
		status[id] = 3;
		prt();
		sem_post(&imprime);
	}
}

void* f_crianca(void *v) {
	int id = *(int*)v;
	sem_wait(&protect);

	/* Verifica se a criança pode entrar na creche, ou seja, se após a entrada desta criança teremos pelo 
	menos 3 adultos para cada criança. Caso seja possível, inserimos a criança na creche. Caso não seja,
	esperamos na fila a sinalização do semáforo fila_criança, e após receber sinal entramos na creche e imprimimos. */
	if(crianca_creche < 3 * adulto_creche) {
		crianca_creche++;
		sem_wait(&imprime);
		status[id] = 2;
		prt();
		sem_post(&imprime);
		sem_post(&protect);
	}
	else {
		crianca_esperando++;
		sem_wait(&imprime);
		status[id] = 1;
		prt();
		sem_post(&imprime);
		sem_post(&protect);
		sem_wait(&fila_crianca);
		sem_wait(&imprime);
		status[id] = 2;
		prt();
		sem_post(&imprime);
	}

	sem_wait(&protect);

	/* Saímos da creche */
	crianca_creche--;
	sem_wait(&imprime);
	status[id] = 3;
	prt();
	sem_post(&imprime);

	/* Verificamos se nossa saída possibilita a saída de algum adulto da creche e se existe algum adulto
	querendo sair. Caso isso aconteça, sinalizamos a possibilidade de saída por meio do semáforo fila_adulto */
	if(adulto_esperando > 0 && 3 * (adulto_esperando - 1) >= crianca_creche) {
		adulto_esperando--;
		adulto_creche--;
		sem_post(&fila_adulto);
	}
	sem_post(&protect);
}

int main() {
	/* Inicializamos os semáforos */
	sem_init(&protect, 0, 1); sem_init(&fila_adulto, 0, 0); sem_init(&fila_crianca, 0, 0);
	sem_init(&imprime, 0, 1);
	crianca_creche = 0; adulto_creche = 0;
	adulto_esperando = 0; crianca_esperando = 0;

	/* criamos as thread das pessoas, definimos se elas são crianças ou adultos e inicializamos os id's e status */
	pthread_t thr_pessoas[N_PESSOAS];
	for(int i = 0; i < N_PESSOAS; i++) {
		int r = rand() % 3;
		if(r == 0) tipo_pessoas[i] = 0;
		else tipo_pessoas[i] = 1;
		id_pessoas[i] = i;
		status[i] = 0;
	}

	/* Cria  as threads */
	for(int i = 0; i < N_PESSOAS; i++) {
		if(tipo_pessoas[i] == 0) pthread_create(&thr_pessoas[i], NULL, f_adulto, (void*) &id_pessoas[i]);
		else pthread_create(&thr_pessoas[i], NULL, f_crianca, (void*) &id_pessoas[i]);
	}

	/* Espera 1 segundo para execução das threads */
	sleep(1);
	/* Todas as threads que não foram executadas morrem */
	return 0;
}