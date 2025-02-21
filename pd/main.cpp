/*
	Modelo: Problema da Designação (PD)
	Nome: Arthur Vieira Silva
	Matrícula: 222050020
*/

#include <bits/stdc++.h>
#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN //MACRO - "using namespace" for ILOCPEX

//CPLEX Parameters
#define CPLEX_TIME_LIM 3600 //3600 segundos
//#define CPLEX_COMPRESSED_TREE_MEM_LIM 8128 //8GB
//#define CPLEX_WORK_MEM_LIM 4096 //4GB
//#define CPLEX_VARSEL_MODE 0
/*
* VarSel Modes:
* -1 Branch on variable with minimum infeasibility
* 0 Branch variable automatically selected
* 1 Branch on variable with maximum infeasibility
* 2 Branch based on pseudo costs
* 3 Strong branching
* 4 Branch based on pseudo reduced costs
*
* Default: 0
*/

struct aresta{
	// id da pessoa, da tarefa e custo de cada aresta:	
    int id_p, c;
    char id_t;
};

//Conjuntos do Problema
int N; //Quantidade de pessoas e tarefas
vector<vector<aresta>> arestas; // Conjunto das arestas

void cplex(){
    //CPLEX
	IloEnv env; //Define o ambiente do CPLEX

	//Variaveis --------------------------------------------- 
	int i, j, k; //Auxiliares
	int numberVar = 0; //Total de Variaveis
	int numberRes = 0; //Total de Restricoes


	//---------- MODELAGEM ---------------

	//Definicao - Variaveis de Decisao 2 dimensoes (x_ij) binarias
	IloArray<IloNumVarArray> x(env);
	for( i = 0; i < N; i++ ){
		x.add(IloNumVarArray(env));
		for( j = 0; j < N; j++ ){
			x[i].add(IloIntVar(env, 0, 1));
			numberVar++;
		}
	}

	//Definicao do ambiente modelo ------------------------------------------
	IloModel model ( env );
	
	//Definicao do ambiente expressoes, para os somatorios ---------------------------------
	//Nota: Os somatorios podem ser reaproveitados usando o .clear(),
	//com excecao de quando existe mais de um somatorio em uma mesma restricao.
	IloExpr sum(env); /// Expression for Sum
	IloExpr sum2(env); /// Expression for Sum2

	//FUNCAO OBJETIVO ---------------------------------------------
	sum.clear();
	for( i = 0; i < N; i++ ){
        for( j = 0; j < N; j++) {
            sum += (arestas[i][j].c * x[i][j]);
        }
	}

	//Modelo de Minimizacao
	model.add(IloMinimize(env, sum)); //Minimizacao

	//RESTRICOES ---------------------------------------------	
	 
	// 2 - Tarefa designada:
	for( j = 0; j < N; j++ ){
        sum.clear();
        for( i = 0; i < N; i++) {
            sum += x[i][j];
        }

        model.add(sum == 1);
		numberRes++;
	}
    
    // 3 - Pessoa designada:
	for( i = 0; i < N; i++ ){
        sum.clear();
        for( j = 0; j < N; j++) {
            sum += x[i][j];
        }

        model.add(sum == 1);
		numberRes++;
	}

	//------ EXECUCAO do MODELO ----------
	time_t timer, timer2;
	IloNum value, objValue;
	double runTime;
	string status;
	
	//Informacoes ---------------------------------------------	
	printf("--------Informacoes da Execucao:----------\n\n");
	printf("#Var: %d\n", numberVar);
	printf("#Restricoes: %d\n", numberRes);
	cout << "Memory usage after variable creation:  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;
	
	IloCplex cplex(model);
	cout << "Memory usage after cplex(Model):  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;

	//Setting CPLEX Parameters
	cplex.setParam(IloCplex::TiLim, CPLEX_TIME_LIM);
	//cplex.setParam(IloCplex::TreLim, CPLEX_COMPRESSED_TREE_MEM_LIM);
	//cplex.setParam(IloCplex::WorkMem, CPLEX_WORK_MEM_LIM);
	//cplex.setParam(IloCplex::VarSel, CPLEX_VARSEL_MODE);

	time(&timer);
	cplex.solve();//COMANDO DE EXECUCAO
	time(&timer2);
	
	//cout << "Solution Status: " << cplex.getStatus() << endl;
	//Results
	bool sol = true;
	/*
	Possible Status:
	- Unknown	 
	- Feasible	 
	- Optimal	 
	- Infeasible	 
	- Unbounded	 
	- InfeasibleOrUnbounded	 
	- Error
	*/
	switch(cplex.getStatus()){
		case IloAlgorithm::Optimal: 
			status = "Optimal";
			break;
		case IloAlgorithm::Feasible: 
			status = "Feasible";
			break;
		default: 
			status = "No Solution";
			sol = false;
	}

	cout << endl << endl;
	cout << "Status da FO: " << status << endl;

	if(sol){ 

		//Results
		//int Nbin, Nint, Ncols, Nrows, Nnodes, Nnodes64;
		objValue = cplex.getObjValue();
		runTime = difftime(timer2, timer);
		//Informacoes Adicionais
		//Nbin = cplex.getNbinVars();
		//Nint = cplex.getNintVars();
		//Ncols = cplex.getNcols();
		//Nrows = cplex.getNrows();
		//Nnodes = cplex.getNnodes();
		//Nnodes64 = cplex.getNnodes64();
		//float gap; gap = cplex.getMIPRelativeGap();
		
		cout << "Variaveis de decisao: " << endl;
		for( i = 0; i < N; i++ ){
            for( j = 0; j < N; j++) {
                value = IloRound(cplex.getValue(x[i][j]));
			    printf("x[%d][%c]: %.0lf\n", arestas[i][j].id_p, arestas[i][j].id_t, value);
            }
		}
		printf("\n");
		
		cout << "Funcao Objetivo Valor = " << objValue << endl;
		printf("..(%.6lf seconds).\n\n", runTime);

	}else{
		printf("No Solution!\n");
	}

	//Free Memory
	cplex.end();
	sum.end();
	sum2.end();

	cout << "Memory usage before end:  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;
	env.end();
}

int main(){

	//Leitura dos dados:
	//A partir de um arquivo (in.txt)
	int i, j;
	cin >> N;
	arestas.resize(N, vector<aresta>(N));

    // Leitura do custo de custo de designação de cada par pessoa-tarefa:
	for(i=0; i<N; i++){
        for(j=0; j<N; j++) {
            cin >> arestas[i][j].id_p >> arestas[i][j].id_t >> arestas[i][j].c;
		    arestas[i][j].id_p = i+1;
        }
	}

	// Imprimindo os dados lidos no arquivo in.txt:
	printf("Verificacao da leitura dos dados:\n");
	printf("Num. itens: %d\n", N);
	printf("Itens - id_p - id_t: custo\n");
    for(i=0; i<N; i++)
        for(j=0; j<N; j++) {
            printf("%d %c: %d\n", arestas[i][j].id_p, arestas[i][j].id_t, arestas[i][j].c);
        }
	printf("\n");

	cplex();

    return 0;
}