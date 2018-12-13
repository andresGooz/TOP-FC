#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <math.h>
#include <algorithm>

/**
para compilar:		g++ TOP.cpp -o output -Wall
para ejecutar: 		./output
**/
//hacer para que no se puedan realizar permutaciones que no sirvan como solución. por lo tanto hay que ir generando las permutaciones manualmente.
using namespace std;
bool maximo_encontrado;
float tmax;
int cant_rutas;
int cant_estaciones;
float *score_estaciones;
float *tiempos_traslados;
int score_solucion;
list<string> tabu_list;
list<list<int>> m_rutas;
string min_instance;
string max_instance;
string limit_instance;
int sum_puntajes;


/*
función addBinary
	input: string a, string b
	output: string c
	comentario: suma los strings de bits a y b, y retorna su suma.
ej:
input string a= "11";
input string b= "01";
output string c="100";
*/
string addBinary(string a, string b) 
{ 
    string result = "";  
    int s = 0;          
    int i = a.size() - 1, j = b.size() - 1; 
    while (i >= 0 || j >= 0 || s == 1) 
    { 
        s += ((i >= 0)? a[i] - '0': 0); 
        s += ((j >= 0)? b[j] - '0': 0); 
        result = char(s % 2 + '0') + result; 
        s /= 2;  
        i--; j--; 
    } 
    return result; 
}

/*
función next_GREAT_string
	input: string pop, list<string> rutas
	output: string push
	comentario: esta función está fuertemente acoplada al resto del algoritmo.
Esta función revisa la lista de rutas instanciadas localmente y revisa la lista tabu_list para filtrar el dominio de instanciaciones posibles.
La idea es no permitir ninguna instanciación de dominio que repita 1s (o vértices visitados) y prohibir la instanciación de dominios que ya está
comprobado que no son solución pues se encuentran en la lista tabú.
*/ 
string next_GREAT_string(string pop,list<string> rutas){
	list<string> rutas_aux=rutas;
	rutas_aux.pop_back();
	int unos[cant_estaciones-2];
	for (int i=0; i<cant_estaciones-2;i++){unos[i]=0;}
	for (auto ruta : rutas_aux)
		for (int i=0; (unsigned)i<ruta.size() ; i++)
			if (ruta[i]=='1')
				unos[i]+=1;
	string next_GREAT=pop;
	bool es_buena=false;
	do{
		es_buena=true;
		next_GREAT=addBinary(next_GREAT,"1");
		if (next_GREAT > max_instance)
			return limit_instance;
		for (int i=0; i<cant_estaciones-2;i++){
			if (next_GREAT[i]=='1' && unos[i]==1)
				es_buena=false;
			if (es_buena==false)
				break;
		}
		if (es_buena==false)
			continue;
		for (auto string_ruta : tabu_list){
			int n=count(string_ruta.begin(), string_ruta.end(), '1');
			for (int i=0; (unsigned)i < string_ruta.size() ; i++)
				if(string_ruta[i]=='1' && next_GREAT[i]=='1')
					n-=1;
			if (n==0){
				es_buena=false;
				break;
			}
		}
	}
	while(es_buena==false);
	return next_GREAT;	
}
/*
función verificar
	input: list<string> rutas
	output: void
	comentario: esta función permite checkear la calidad de un conjunto de rutas específico.
	Si la función encuentra un conjunto de vértices infactible (supera tmax), entonces la agrega a la tabu_list
	Si la función encuentra un conjunto de rutas factibles cuyo score es el mejor encontrado hasta ahora. guarda la solución y el orden de manera global.
	Si la función encuentra un conjunto de rutas factibles cuyo score es máximo. Entonces lo guarda e indica que el máximo ha sido encontrado.
*/ 
void verificar(list<string> rutas){
	float local_puntajes[cant_rutas];
	list<list<int>> local_sol;
	int n_ruta=-1;
	bool las_rutas_son_validas=true;
	for (auto ruta : rutas){
		bool ruta_valida=false;
		n_ruta+=1;
		list<int> list_estaciones;
		list_estaciones.push_back(0);
		for (int i=0; (unsigned)i < ruta.size() ; i++)
			if (ruta[i]=='1')
				list_estaciones.push_back(i+1);
		list_estaciones.push_back(ruta.size()+1);					
		do { 	
			float tiempo=0.0;
			float puntaje=0.0;
			int previo=0;
			for (auto l: list_estaciones){
				tiempo +=tiempos_traslados[previo*cant_estaciones+l]; 
				puntaje += score_estaciones[l];
				previo=l;
			}
			if (tiempo <= tmax){
				local_puntajes[n_ruta]=puntaje;
				local_sol.push_back(list_estaciones);
				ruta_valida=true;
				break;			
			}
		} while (next_permutation(next(list_estaciones.begin()), prev(list_estaciones.end())));
		if (ruta_valida == false){
			las_rutas_son_validas=false;
			tabu_list.push_back(ruta);
			break;		
		}
	}
	if (las_rutas_son_validas){
		int sol=0;
		for (int i=0; i < cant_rutas ; i++)
			sol+=local_puntajes[i];
		if (sol > score_solucion){
			score_solucion= sol;
			m_rutas= local_sol;
		}
		if (sol==sum_puntajes){
			maximo_encontrado=true;
			return;
		}
	}
}

/*
función verificar
	input: parámetros globales del problema
	output: void
	comentario: Esta función guarda la lógica general del algoritmo en términos de orden y estrutura de código.
Se aprecian 3 tipos de movimientos principales en el arbol de backtrack usado para forward checking:
	INSTANCIAR: instancia con el valor de dominio mínimo disponible hasta llegar al final hacia abajo.
	ITERAR: itera sobre el nivel actual del arbol, filtrando el dominio para elegir la mejor proxima iteración. 
	BACKTRACK: realiza backtrack.
*/ 
void ALGORITMO()
{
	bool quedan_instancias_pendientes 	= true;
	bool back_track				= false;
	bool iterando				= false;
	bool instanciando			= true;
	int rutas_instanciadas			= 0;
	list<string> rutas;
	while (quedan_instancias_pendientes==true)
	{
		if (instanciando)
		{	//cout << "INS" << endl;
			rutas.push_back(min_instance);
			rutas_instanciadas+=1;
			if (rutas_instanciadas == cant_rutas){
				instanciando=false;
				iterando=true;
			}
		}
		if (iterando)
		{	//cout << "ITE" << endl;
			verificar(rutas);
			if (maximo_encontrado==true)
				return;
			string pop  = rutas.back();
			string push = next_GREAT_string(pop,rutas);
			if (push == limit_instance)
			{
				iterando=false;
				back_track=true;
			}
			else{
				rutas.pop_back();
				rutas.push_back(push);			
			}					
		}
		if (back_track)
		{	//cout << "BT" << endl;
			rutas.pop_back();
			rutas_instanciadas-=1;
			if (rutas_instanciadas==0)
				break;
			string pop=rutas.back();
			string push = next_GREAT_string(pop,rutas);
			if (push == limit_instance)
				continue;
			else{
				rutas.pop_back();
				rutas.push_back(push);
				back_track=false;
				instanciando=true;
			} 			
		}						
	}
};

int main(){
	ifstream file("input.txt");
	string string0;
	string string1;
	string string2;
	int contador=0;
	int cantidad_estaciones;				//**número de estaciones
	int cantidad_rutas;					//**número de rutas
	float tiempo_maximo;					//**tiempo máximo permitido para completar la ruta
	while ( file >> string0 >> string1){ 
		if (string0.compare("n") == 0){cantidad_estaciones=atoi(string1.c_str());if(contador==2){break;};contador+=1;continue;};
		if (string0.compare("m") == 0){cantidad_rutas=atoi(string1.c_str());if(contador==2){break;};contador+=1;continue;}; 
		if (string0.compare("tmax") == 0){tiempo_maximo=atof(string1.c_str());if(contador==2){break;};contador+=1;continue;};
	};
	float vertices[cantidad_estaciones][3];
	contador=0;
	while ( file >> string0 >> string1 >> string2){
		vertices[contador][0]=atof(string0.c_str());
		vertices[contador][1]=atof(string1.c_str());
		vertices[contador][2]=atof(string2.c_str());
		contador+=1;
	};
	float puntajes[cantidad_estaciones];			//**puntajes de cada estacion
	float tiempos[cantidad_estaciones][cantidad_estaciones];//**tiempo de traslado entre estaciones
	for (int i=0;i<cantidad_estaciones;i++){
		puntajes[i]=vertices[i][2];
		for(int j=0;j<cantidad_estaciones;j++){
			tiempos[i][j]=pow(pow((vertices[i][0]-vertices[j][0]),2)+pow((vertices[i][1]-vertices[j][1]),2),0.5);
		};	
	};
	/**
	comentario: hasta aquí hemos cargado los parámetros del problema: 
			-Número de estaciones: 					int cantidad_estaciones
			-Número de rutas: 					int cantidad_rutas
			-Tiempo máximo permitodo para completar cada ruta:	int tiempo_maximo
			-Puntajes de cada estación:				puntajes[cantidad_estaciones]
			-Tiempo de traslado entre estaciones			tiempos[cantidad_estaciones][cantidad_estaciones]
	**/
	//SETEANDO Y GLOBALIZANDO PARÁMETROS
	cant_estaciones=cantidad_estaciones;
	cant_rutas=cantidad_rutas;
	tmax=tiempo_maximo;
	score_estaciones = puntajes;
	tiempos_traslados=*tiempos;

	max_instance="";
	min_instance="";
	for (int i=0; i<cant_estaciones-2; i++){min_instance+="0";max_instance+="1";}
	limit_instance=addBinary(max_instance,"1");
	sum_puntajes=0;
	for (int i = 0; i < cantidad_estaciones; i++){sum_puntajes+=puntajes[i];}
	maximo_encontrado=false;
	score_solucion=-1;

	//EJECUTANDO ALGORITMO
	ALGORITMO();
	if (score_solucion==-1){
		cout << "NO HAY SOLUCION DE NINGUN TIPO" << endl;
		return 0;	
	}		
	//AQUÍ YA TENDRÍAMOS LA SOLUCION
	cout << score_solucion << endl;
	list<list<int>>::iterator itr;
	for (itr=m_rutas.begin(); itr != m_rutas.end(); itr++){
		float timer=0.0;
		list<int>tl=*itr;
		list<int>::iterator it;
		
		int inicio=0;
		for (it=next(tl.begin()); it != tl.end(); it++){
			timer+=tiempos[inicio][*it];
			inicio=*it;
		}
		cout << timer << " 1 ";
		for (it=next(tl.begin()); it != tl.end(); it++){
		   	cout<<*it+1 << " ";
		}
		cout<< endl ;
	}
};

