/*******************************************************************
  *##############################################################*
  *#              Trabalho de Sistemas Operacionais             #*
  *#                                                            #*
  *#     Alunos: Guilherme Cardoso Silva      0022645           #*
  *#             Renata Caroline Cunha        0022429           #*
  *#                                                            #*
  *#  Para compilar utilize : gcc -Wall -pthread -o exe main.c  #*
  *#  Para executar utilize : ./exe <NumEstacionamento>         #*
  *#                                                            #*
  *##############################################################*
********************************************************************/
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define NUM_THREADS 7
#define NUM_GRUPOS  3
/* Define para verificações diretas em funcionários */
#define GIRAFALES 0
#define FLORINDA  1
#define XAVIER    2
#define JEAN      3
#define WALTER    4
#define PINKMAN   5
/* Define para estado atual do funcionário */
#define EmCasa    0
#define Esperando 1
#define Terminou  2
/* Declarações de Funções de Threads */
void *ThreadFuncionario(void *param);
void *ThreadDiretor();
/* Declarações de Funções do Monitor */
void estaciona(int funcionario);
void esperar(int funcionario);
void liberar(int funcionario);
void verificar(void);
/* Declarações de Funções Gerais */
int  GrupoPresente(int integrante1, int integrante2);
int  SouUnicoEsperar(int ind);
int  TodosTerminaram(void);
int  RetornaGrupo(int ind);
void PreencheDados(void);
void PassaVaga(void);

/* Declarações de Mutex e Variaveis de Condição */
pthread_mutex_t mutexGeral    = PTHREAD_MUTEX_INITIALIZER; /*Mutex da sessão critica de Estacionamento*/
pthread_mutex_t mutexEstado   = PTHREAD_MUTEX_INITIALIZER; /*Mutex da variavel global de Estado de cada funcionário*/
pthread_mutex_t mutexDeadlock = PTHREAD_MUTEX_INITIALIZER; /*Mutex da variavel global que indica deadlock*/
pthread_cond_t  condFuncionario[NUM_GRUPOS] = PTHREAD_COND_INITIALIZER; /*Vetor de Variaveis de condição para grupos*/
/* Declarações de variaveis globais */
char Nomes[NUM_THREADS-1][10] = {"Girafales","Florinda","Xavier","Jean","Walter","Pinkman"};/*Vetor de nomes*/
int NumEstacionamento, UltimoEstacionar, Deadlock, DeadlockFuncionario;
int InfoThreads[NUM_THREADS-1]; /*Vetor de Estado dos funcionários */

/* Implementação de Funções */
void esperar(int funcionario){
  printf("%s quer usar a vaga\n", Nomes[funcionario]);
  pthread_mutex_lock(&mutexEstado);     /*Trava a mutex verifica estado dos Funcionarios*/
  InfoThreads[funcionario] = Esperando; /*Altera o estado do funcionario para esperando */
  if(!SouUnicoEsperar(funcionario)){
    pthread_cond_wait(&condFuncionario[RetornaGrupo(funcionario)], &mutexEstado);
  }
  pthread_mutex_unlock(&mutexEstado);   /*Destrava a Mutex verifica estado dos Funcionarios*/
}

void estaciona(int funcionario){
  pthread_mutex_lock(&mutexDeadlock);   /*Trava a Mutex Deadlock*/
  if(DeadlockFuncionario){
    /*Caso o funcionario tenha sido liberado pelo diretor significa que existiu um deadlock anteriormente*/
    printf("Diretor detectou um deadlock, liberando %s\n", Nomes[funcionario]);
    DeadlockFuncionario = 0;  /*Variavel que controla se funcionario foi liberado por um Deadlock*/
  }
  pthread_mutex_unlock(&mutexDeadlock); /*Destrava a Mutex Deadlock*/
  printf("%s estaciona para trabalhar\n", Nomes[funcionario]);
  sleep(1);
}

void liberar(int funcionario){
  printf("%s vai pra casa estudar\n", Nomes[funcionario]);
  pthread_mutex_lock(&mutexEstado);   /*Trava a mutex estado dos Funcionarios*/
    InfoThreads[funcionario] = EmCasa;/*Altera o estado do funcionario*/
  pthread_mutex_unlock(&mutexEstado); /*Destrava a Mutex estado dos Funcionarios*/
  UltimoEstacionar = funcionario;     /*Variavel armazena quem foi o ultimo a estacionar para que ao passar a
                                        vaga ele verifique possiveis inanissoes*/
  PassaVaga();
}

void verificar(void){
  int GrupoLiberado;
  pthread_mutex_lock(&mutexDeadlock); /*Trava a Mutex Deadlock*/
  if(Deadlock){    /*Se ao tentar passar a vaga ocorreu um Deadlock o diretor irá liberar alguem aleatoriamente*/
    GrupoLiberado = (rand()/(double)RAND_MAX)*3;
    pthread_cond_signal(&condFuncionario[GrupoLiberado]);
    Deadlock = 0;            /*Atribui valor falso para a variavel que indica Deadlock*/
    DeadlockFuncionario = 1; /*Variavel que informa a quem for estacionar que foi liberado pelo Diretor*/
  }
  pthread_mutex_unlock(&mutexDeadlock);/*Destrava a Mutex Deadlock*/
}

void PreencheDados(void){
  int i;
  /*Define que o estado inicial de todos os funcionarios é em casa*/
  for(i=0; i < (NUM_THREADS-1); i++){
    InfoThreads[i] = EmCasa;
  }
  DeadlockFuncionario = 0;
  Deadlock = 0;
}

int GrupoPresente(int integrante1, int integrante2){
  /*Verifica se algum integrante do grupo está esperando pela vaga */
  if(InfoThreads[integrante1] == Esperando || InfoThreads[integrante2] == Esperando){
    return(1);
  }
  return(0);
}

int RetornaGrupo(int ind){
  if(ind <= 1){   /*Se o ind for igual a 0 ou 1 retorna grupo 0*/
    return(0);
  }else{
    if(ind <= 3){ /*Se o ind for igual a 2 ou 3 retorna grupo 1*/
      return(1);
    }else{
      return(2);  /*Se o ind for igual a 4 ou 5 retorna grupo 2*/
    }
  }
}

int SouUnicoEsperar(int ind){
  int i;
  /*Verifica se o funcionario é o unico a esperar, caso seja ele poderá estacionar */
  for(i=0; i < (NUM_THREADS-1); i++){
    if(ind!=i){
      if(InfoThreads[i] == Esperando){
        return(0);
      }
    }
  }
  return(1);
}

void PassaVaga(void){
  int Grupo1 = GrupoPresente(GIRAFALES, FLORINDA);  /*Verifica se o Grupo1 está esperando*/
  int Grupo2 = GrupoPresente(XAVIER, JEAN);         /*Verifica se o Grupo2 está esperando*/
  int Grupo3 = GrupoPresente(WALTER, PINKMAN);      /*Verifica se o Grupo3 está esperando*/

  if(Grupo1 && Grupo2 && Grupo3){ //111 Deadlock o diretor decide
    /******* ******* ******* ******* Verifica Deadlock ******* ******* ******* *******/
    pthread_mutex_lock(&mutexDeadlock);   /*Trava a mutex estado dos Funcionarios*/
      Deadlock = 1; /*Atribui valor true para variavel de Deadlock, após isto esta Thread pode voltar a querer estacionar,
          porém ficara preso no wait por existir alguem esperando, apenas o diretor poderá liberar alguem para estacionar*/
    pthread_mutex_unlock(&mutexDeadlock); /*Destrava a Mutex estado dos Funcionarios*/
    /******* ******* ******* ******* ******* ******* ******* ******* ******* *******/
  }else{
    /******* Trata Inanição antes de escolher novo funcionario para estacionar *******/
      /*Caso o parceiro do ultimo a estacionar esteja presente e existem outros funcionarios esperando,
        retira a pontuação do grupo em concorrer a vaga*/
    if((GIRAFALES == UltimoEstacionar || FLORINDA == UltimoEstacionar)&&(Grupo2 || Grupo3)&&(Grupo1)){
      Grupo1--;
    }
    if((XAVIER == UltimoEstacionar || JEAN == UltimoEstacionar)&&(Grupo1 || Grupo3)&&(Grupo2)){
      Grupo2--;
    }
    if((WALTER == UltimoEstacionar || PINKMAN == UltimoEstacionar)&&(Grupo1 || Grupo2)&&(Grupo3)){
      Grupo3--;
    }
    /******* ******* ******* ******* ******* ******* ******* ******* ******* *******/

    /******* ******* Passa a Vaga caso alguem esteja esperando ******* *******/
    if((Grupo1 && !Grupo2 && !Grupo3) || (Grupo1 && Grupo2 && !Grupo3)){ //100 || 110 Grupo1 Vence
      pthread_cond_signal(&condFuncionario[0]); /*Libera o funcionario 1 ou 2 que aguarda na variavel do grupo 1*/
    }else{
      if((!Grupo1 && Grupo2 && !Grupo3) || (!Grupo1 && Grupo2 && Grupo3)){ //010 || 011 Grupo2 Vence
        pthread_cond_signal(&condFuncionario[1]); /*Libera o funcionario 3 ou 4 que aguarda na variavel do grupo 2*/
      }else{
        if((!Grupo1 && !Grupo2 && Grupo3) || (Grupo1 && !Grupo2 && Grupo3)){ //001 || 101 Grupo 3 Vence
          pthread_cond_signal(&condFuncionario[2]); /*Libera o funcionario 5 ou 6 que aguarda na variavel do grupo 3*/
        }
      }
    }
    /******* ******* ******* ******* ******* ******* ******* ******* ******* *******/
  }
}

int TodosTerminaram(void){
  int i;

  for(i=0; i < (NUM_THREADS-1); i++){
    if(InfoThreads[i] != Terminou){
      return(0);
    }
  }
  return(1);
}

void *ThreadDiretor(){
  int verif = 0;

  while(!verif){
    sleep(5);
    verificar();
    pthread_mutex_lock(&mutexEstado);   /*Trava a mutex verifica estado dos Funcionarios*/
      verif = TodosTerminaram();
    pthread_mutex_unlock(&mutexEstado); /*Destrava a Mutex verifica estado dos Funcionarios*/
  }
  pthread_exit(0);
}

void *ThreadFuncionario(void *param){
  int ind = *((int *) param), FunEstacionou = NumEstacionamento, tempoEspera;

  tempoEspera = (rand()/(double)RAND_MAX)*2;
  sleep(tempoEspera);             /*Randon de inicialização para que alguns cheguem com 1 segundo gerando chegada aleatoria*/

  while(FunEstacionou != 0){      /*Enquanto a quantidade de vezes que o funcionario falta para estacionar nao é 0 continua*/
    esperar(ind);                 /*Funcionario espera pela vaga*/
    pthread_mutex_lock(&mutexGeral);  /*Trava a mutex*/
      estaciona(ind);                 /*funcionário estaciona na vaga*/
      liberar(ind);                   /*funcionário libera a vaga*/
    pthread_mutex_unlock(&mutexGeral);/*Destrava a Mutex*/
    FunEstacionou--;
    if(FunEstacionou != 0){       /*Se ainda não terminou espera 3 a 5 seg para retormar*/
      tempoEspera = (rand()/(double)RAND_MAX)*3 + 3;
      sleep(tempoEspera);
    }
  }
  pthread_mutex_lock(&mutexEstado);   /*Trava a mutex estado dos funcionários*/
    InfoThreads[ind] = Terminou;
  pthread_mutex_unlock(&mutexEstado); /*Destrava a Mutex estado dos funcionários*/
  pthread_exit(0);
}

int main(int argc, char *argv[]){
  if(argc == 2){
    int thread_args[NUM_THREADS], i;
    pthread_t threads[NUM_THREADS];

    NumEstacionamento = atoi(argv[1]);
    srand(time(NULL));
    PreencheDados();

    /*Cria os Threads funcionários*/
    for(i=0; i < (NUM_THREADS-1); i++){
      thread_args[i] = i;
      pthread_create(&threads[i], NULL, ThreadFuncionario, (void *) &thread_args[i]);
    }
    /*Cria o Threads Diretor*/
    pthread_create(&threads[NUM_THREADS-1], NULL, ThreadDiretor, NULL);

    /*Espera até que todas as Threads sejam finalizadas*/
    for(i=0; i < NUM_THREADS; i++){
      pthread_join(threads[i], NULL);
    }
    /*Destroi variaveis de condição e a mutex*/
    for(i=0; i < NUM_GRUPOS; i++){
      pthread_cond_destroy(&condFuncionario[i]);
    }
    pthread_mutex_destroy(&mutexGeral);
    return(0);
  }
  printf("Erro de Entrada ./<executavel> <NumEstacionamento>\n");
  return(-1);
}
