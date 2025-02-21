/*
	Modelo: Problema do Transporte (PT)
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

//Conjuntos do Problema:
int m, n; // m origens e n destinos
vector<int> S; // Oferta
vector<int> D; // Demanda

typedef struct aresta {
	int id_s, id_d, c; // id de origem, de destino e o custo unitário de transporte de cada aresta
};

vector<vector<aresta>> arestas; // Custo de cada aresta (i, j)

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
	for( i = 0; i < m; i++ ){
		x.add(IloNumVarArray(env));
		for( j = 0; j < n; j++ ){
			x[i].add(IloIntVar(env, 0, INT_MAX));
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
	for( i = 0; i < m; i++ ){
        for( j = 0; j < n; j++) {
            sum += (arestas[i][j].c * x[i][j]);
        }
	}

	//Modelo de Minimizacao
	model.add(IloMinimize(env, sum)); //Minimizacao

	//RESTRICOES ---------------------------------------------	
	 
	// 2 - Atender demanda:
	for( j = 0; j < n; j++ ){
        sum.clear();
        for( i = 0; i < m; i++ ){
            sum += x[i][j];
        }

		model.add(sum == D[j]); 
		numberRes++;
    }
	
    // 3 - Respeitar oferta:
    for( i = 0; i < m; i++ ){
        sum.clear();
        for( j = 0; j < n; j++ ){
            sum += x[i][j];
        }

		model.add(sum <= S[i]);
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
		for( i = 0; i < m; i++ ){
            for( j = 0; j < n; j++) {
                value = IloRound(cplex.getIntValue(x[i][j]));
			    printf("x[%d][%d]: %.0lf\n", i, j, value);
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
	cin >> m >> n; // número de origens e destinos
    S.resize(m);
    D.resize(n);
	arestas.resize(m, vector<aresta>(n));

    // Leitura da oferta de cada origem:
	for(i=0; i<m; i++){
		cin >> S[i];
	}

    // Leitura da demanda de cada destino:
    for(i=0; i<n; i++) {
        cin >> D[i];
    }

    // Leitura do custo de transporte de cada aresta:
    for(i=0; i<m; i++) {
        for(j=0; j<n; j++) {
            cin >> arestas[i][j].id_s >> arestas[i][j].id_d >> arestas[i][j].c;
			arestas[i][j].id_s = i+1;
			arestas[i][j].id_d = i+1;
        }
    }
    
	// Imprimindo os dados lidos no arquivo in.txt:
	printf("Verificacao da leitura dos dados:\n");
	printf("Num. de origens: %d\n", m);
	printf("Num. de destinos: %d\n", n);
	printf("Valor da oferta de cada origem:\n");
    for(i=0; i<m; i++) {
        printf("%d\n", S[i]);
    }
	printf("\n");

    printf("Valor da demanda de cada destino:\n");
    for(i=0; i<n; i++) {
        printf("%d\n", D[i]);
    }
	printf("\n");

    printf("ID Oferta - ID Demanda - Custo:\n");
    for(i=0; i<m; i++) {
        for(j=0; j<n; j++) {
            printf("%d %d %d\n", arestas[i][j].id_s, arestas[i][j].id_d, arestas[i][j].c);
        }
    }
	printf("\n");

	cplex();

    return 0;
}