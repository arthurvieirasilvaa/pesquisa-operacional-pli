/*
	Modelo: Problema de Fluxo de Custo Mínimo (PFCM)
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
    int c, u; // custo de transporte unitário e fluxo máximo por aresta
};

struct oferta{
	int id, s; // id e quantidade de mercadoria s produzida em cada nó 
};

struct demanda {
	int id, d; // id e quantidade de mercadoria d consumida em cada nó
};

//Conjuntos do Problema:
int N, M, S, D, T; // Número de vértices, arestas, nós de oferta, demanda e transbordo
vector<vector<aresta>> arestas; // Conjunto de arestas
vector<oferta> ofertas; // Conjunto de nós de oferta
vector<demanda> demandas; // Conjunto de nós de demanda
vector<int> transbordos; // Conjunto de nós de transbordo


void cplex(){
    //CPLEX
	IloEnv env; //Define o ambiente do CPLEX

	//Variaveis --------------------------------------------- 
	int i, j, k; //Auxiliares
	int numberVar = 0; //Total de Variaveis
	int numberRes = 0; //Total de Restricoes
	
	//---------- MODELAGEM ---------------

	
	//Definicao - Variaveis de Decisao 2 dimensoes (x_ij) não binárias (discretas)
	IloArray<IloNumVarArray> x(env);
	for( i = 0; i < N; i++ ){
		x.add(IloNumVarArray(env));
		for( j = 0; j < N; j++ ){
			// Verifica se a aresta (i, j) existe:
			if(arestas[i][j].c != -1) {
				// Verifca se a aresta (i, j) possui fluxo máximo diferente de infinito:
				if(arestas[i][j].u != -1) {
					x[i].add(IloIntVar(env, 0, arestas[i][j].u));
				}
				
				else {
					x[i].add(IloIntVar(env, 0, INT_MAX));
				}
				
				numberVar++;
			}

			else {
				x[i].add(IloIntVar(env, 0, 0));
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

	// FUNCAO OBJETIVO ---------------------------------------------
	sum.clear();
	for( i = 0; i < N; i++ ){
		for( j = 0; j < N; j++) {
			// Verifica se a aresta (i, j) existe e se i é diferente de j:
			if(arestas[i][j].c != -1 && i != j) {
				sum += (arestas[i][j].c * x[i][j]);
			}
		}
	}

	//Modelo de Minimizacao
	model.add(IloMinimize(env, sum)); //Minimizacao

	//RESTRICOES ---------------------------------------------	
	

	// 2 - Restrição Oferta:
	for(i=0; i<S; i++){ // For que representa o (Para Todo).		
		sum.clear(); //Somatório 1
		for( j = 0; j < N; j++ ){
			// Verifica se a aresta (S[i], j) existe e se S[i] é diferente de j:
			if(arestas[ofertas[i].id][j].c != -1 && ofertas[i].id != j) {
				sum += x[ofertas[i].id][j];
			}
		}
	
		sum2.clear(); //Somatório 2
		for( k = 0; k < N; k++ ){
			// Verifica se a aresta (k, S[i]) existe e se S[i] é diferente de k:
			if(arestas[k][ofertas[i].id].c != -1 && ofertas[i].id != k) {
				sum2 += x[k][ofertas[i].id];
			}
		}

		model.add(sum - sum2 <= ofertas[i].s);
		numberRes++;
	}//Fim do for que representa o (Para Todo).


	// 3 - Restrição Demanda:
	for(i=0; i<D; i++){ // For que representa o (Para Todo).	
		sum.clear(); //Somatório 1
		for( j = 0; j < N; j++ ){
			// Verifica se a aresta (D[i], j) existe e se D[i] é diferente de j:
			if(arestas[demandas[i].id][j].c != -1 && demandas[i].id != j) {
				sum += x[demandas[i].id][j];
			}
		}
	
		sum2.clear(); //Somatório 2
		for( k = 0; k < N; k++ ){
			// Verifica se a aresta (k, D[i]) existe e se D[i] é diferente de k:
			if(arestas[k][demandas[i].id].c != -1 && demandas[i].id != k) {
				sum2 += x[k][demandas[i].id];
			}
		}

		model.add(sum - sum2 <= -demandas[i].d); 
		numberRes++;
	}//Fim do for que representa o (Para Todo).


	// 4 - Conservação de Fluxo:
	for(i=0; i<T; i++) {		
		sum.clear(); //Somatório 1
		for( j = 0; j < N; j++ ){
			// Verifica se a aresta (T[i], j) existe e se T[i] é diferente de j:
			if(arestas[transbordos[i]][j].c != -1 && transbordos[i] != j) {
				sum += x[transbordos[i]][j];
			}
		}
	
		sum2.clear(); //Somatório 2
		for( k = 0; k < N; k++ ){
			// Verifica se a aresta (T[i], j) existe e se T[i] é diferente de k:
			if(arestas[k][transbordos[i]].c != -1 && transbordos[i] != k) {
				sum2 += x[k][transbordos[i]];
			}
		}

		model.add(sum - sum2 == 0); 
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
				if(arestas[i][j].c != -1 && i != j) {
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
	int i, j;
	int id_i, id_j, c, u;
	cin >> N >> M;
	cin >> S >> D >> T;
	arestas.resize(N, vector<aresta>(N));
	ofertas.resize(S);
	demandas.resize(D);
	transbordos.resize(T);

	// Inicializa os custos e fluxos máximos de cada aresta com -1:
	for(i=0; i<N; i++) {
		for(j=0; j<N; j++) {
			arestas[i][j].c = -1;
			arestas[i][j].u = -1;
		}
	}

	// Leitura dos nós de oferta:
	for(i=0; i<S; i++){
		cin >> ofertas[i].id >> ofertas[i].s;
	}

	// Leitura dos nós de demanda:
	for(i=0; i<D; i++){
		cin >> demandas[i].id >> demandas[i].d;
	}

	// Leitura dos nós de transbordo:
	for(i=0; i<T; i++){
		cin >> transbordos[i];
	}

	// Leitura das arestas do arquivo de entrada:
	for(i=0; i<M; i++) {
		cin >> id_i >> id_j >> c >> u;
		arestas[id_i][id_j].c = c;
		arestas[id_i][id_j].u = u;
	}
	
	// Imprimindo os dados lidos no arquivo in.txt:
	printf("Verificacao da leitura dos dados:\n");
	printf("Num. Vértices: %d\n", N);
	printf("Num. Arestas: %d\n", M);
	printf("Num. nós de oferta, demanda e transbordo: %d %d %d\n", S, D, T);
	printf("Nós de Oferta - id: oferta\n");
	for(i=0; i<S; i++) {
		printf("%d %d\n", ofertas[i].id, ofertas[i].s);
	}
	printf("\n");

	printf("Nós de Demanda - id: demanda\n");
	for(i=0; i<D; i++) {
		printf("%d %d\n", demandas[i].id, demandas[i].d);
	}
	printf("\n");

	printf("Nós de Transbordo - id\n");
	for(i=0; i<T; i++) {
		printf("%d\n", transbordos[i]);
	}
	printf("\n");
	
	printf("Arestas - id_i - id_j: custo fluxo máximo\n");
	for(i=0; i<N; i++) {
		for(j=0; j<N; j++) {
			if(arestas[i][j].c != -1) {
				printf("%d - %d: %d %d\n", i, j, arestas[i][j].c, arestas[i][j].u);
			}
		}
	}
	printf("\n");

	cplex();

    return 0;
}