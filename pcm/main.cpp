/*
	Modelo: Problema do Caminho Mínimo (PCM)
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

// Conjuntos do Problema:
int N, M; // Quantidade de vértices e arestas
vector<vector<int>> arestas; // Conjunto das arestas
int s, d; // origem e destino

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
    	for( j = 0; j < N; j++) {
			x[i].add(IloIntVar(env, 0, 1));
			// Verifica se a aresta (i, j) existe e se i é diferente de j:
        	if(arestas[i][j] != -1 && i != j) {
            	numberVar++;
			}
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
			// Verifica se a aresta (i, j) existe e se i é diferente de j:
			if(arestas[i][j] != -1 && i != j) {
				sum += (arestas[i][j] * x[i][j]);
			}
		}
	}

	//Modelo de Minimizacao
	model.add(IloMinimize(env, sum)); //Minimizacao

	//RESTRICOES ---------------------------------------------	
	 

	// 2 - Restrição do PCM baseada no PFCM:
	for(i=0; i<N; i++){ // For que representa o (Para Todo).
		sum.clear(); //Somatório 1
		for( j = 0; j < N; j++ ){
			// Verifica se a aresta (i, j) existe e se i é diferente de j:
			if(arestas[i][j] != -1 && i != j) {
				sum += x[i][j];
			}
		}

		sum2.clear(); //Somatório 2
		for(k = 0; k < N; k++ ){
			// Verifica se a aresta (k, i) existe e se i é diferente de k:
			if(arestas[k][i] != -1 && k != i) {
				sum2 += x[k][i];
			}
		}

		// Se i é a origem:
		if(i == s) {
			model.add(sum - sum2 == 1);
			numberRes++;
		}

		// Se i é o destino:
		else if(i == d) {
			model.add(sum - sum2 == -1);
			numberRes++;
		}

		// Caso contrário:
		else {
			model.add(sum - sum2 == 0);
			numberRes++;
		}

	}//Fim do for que representa o (Para Todo).


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
				// Verifica se a aresta (i, j) existe e se i é diferente de j:
				if(arestas[i][j] != -1 && i != j) {
					value = IloRound(cplex.getIntValue(x[i][j]));
					printf("x[%d][%d]: %.0lf\n", i, j, value);
				}
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
	int i, j, id_1, id_2, c;
	cin >> N >> M;
	arestas.resize(N, vector<int>(N));

    cin >> s >> d;

	// Inicializa os custos de cada aresta com -1:
	for(i=0; i<N; i++) {
		for(j=0; j<N; j++) {
			arestas[i][j] = -1;
		}
	}

	// Leitura das arestas do arquivo de entrada:
	for(i=0; i<M; i++){
        cin >> id_1 >> id_2 >> c;
		arestas[id_1][id_2] = c;
		arestas[id_2][id_1] = c;
	}

	// Imprimindo os dados lidos no arquivo in.txt:
	printf("Verificacao da leitura dos dados:\n");
	printf("Num. Vértices: %d\n", N);
	printf("Num. Arestas: %d\n", M);
    printf("Origem: %d\n", s);
    printf("Destino: %d\n", d);
	printf("Itens - id_1 - id_2: peso\n");
    for(i=0; i<N; i++) {
		for(j=0; j<N; j++) {
			// Verifica se a aresta (i, j) existe:
			if(arestas[i][j] != -1) {
				printf("%d - %d: %d\n", i, j, arestas[i][j]);
			}
		}
	}
	printf("\n");

	cplex();

    return 0;
}