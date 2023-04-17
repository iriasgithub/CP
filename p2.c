#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void MPI_BinomialColectiva (void * buf , int count , MPI_Datatype datatype ,
                int root , MPI_Comm comm);

void MPI_FlattreeColectiva (void * buff , void * recvbuff , int count ,
                    MPI_Datatype datatype , MPI_Op op , int root , MPI_Comm comm);

void inicializaCadena(char *cadena, int n);


int main(int argc, char *argv[]){
    
    int my_id, numprocs, n, count = 0, sum_total = 0;
    char *cadena, L;

    
    MPI_Init (&argc, &argv);                    //Inicializar el entorno MPI
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);  //Conseguir el numero de procesos
    MPI_Comm_rank (MPI_COMM_WORLD, &my_id);     //Conseguir el id propio

    if(my_id == 0){                             //El proceso 0 hace entrada
        if(argc != 3){
            printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n"
            "n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
            exit(1); 
        }
        n = atoi(argv[1]);
        L = *argv[2];
    }

    //El proceso 0 hace un send a todos, el resto de procesos recibe
    //MPI_Bcast(&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    //MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    //BINOMIAL
    MPI_BinomialColectiva (&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_BinomialColectiva (&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //Inicializamos la cadena -> todas las cadenas seran la misma EN VALOR
    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);
    
    //Hacemos las sumas por posiciones
    for(int i = my_id; i < n; i += numprocs)
        if(cadena[i] == L) count ++;

    //Enviamos los datos en count al 0, se suman y se guardan en el buff sum_total de 0
    //MPI_Reduce(&count, &sum_total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    //FLAT TREE REDUCE (mismos parametros)
    MPI_FlattreeColectiva (&count , &sum_total , 1, MPI_INT ,MPI_SUM, 0 , MPI_COMM_WORLD);

    if(my_id == 0)
        printf("El numero de apariciones de la letra %c es %d\n", L, sum_total);
    
    free(cadena);
    MPI_Finalize();         //Esperamos a que todos los procesos finalicen para cerrar el entorno
    exit(0);

}

void MPI_BinomialColectiva (void * buf , int count , MPI_Datatype datatype ,
                int root , MPI_Comm comm){
                    
    int numprocs, my_id; 

    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);   
    MPI_Comm_rank (MPI_COMM_WORLD, &my_id); 

    int iterations = ceil(log2(numprocs));          //El número de iteracciones del bucle será igual a la altura del árbol.

    for (int i = 1; i <= iterations; i++){
        int potencia = (int) pow(2,i-1);
        if(my_id < potencia && my_id + potencia < numprocs){                                                                //Los procesos con ID MENOR A POTENCIA 
                                                                                                                            //y con ALGUIEN A QUIEN MANDAR (que no se pase del numero de procesos) 
            printf("Soy el proceso %d y estoy mandando al proceso %d || PASO: %d\n", my_id, my_id + potencia, i);           //Mandan a su id más potencia
            MPI_Send(buf, count, datatype, my_id + potencia, 0, comm);
        }
        else if(i == (int)(log2(my_id)) + 1){                                                                               //El proceso recibe si le toca recibir en este paso
                                                                                                                            //Concretamente un proceso recibe en el paso log2(id) + 1
            printf("Soy el proceso %d y estoy recibiendo del proceso %d || PASO: %d\n", my_id, my_id - potencia, i);
            MPI_Recv (buf, count, datatype, my_id - potencia, 0, comm, MPI_STATUS_IGNORE);
        } 
    }
}

void MPI_FlattreeColectiva (void * buff , void * recvbuff , int count ,
                    MPI_Datatype datatype , MPI_Op op, int root , MPI_Comm comm){
    
    int numprocs, my_id; 

    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);   
    MPI_Comm_rank (MPI_COMM_WORLD, &my_id); 

    if( my_id == root){

        *(int *)recvbuff = *(int *)buff;                                                    //Guardamos el valor que hay en el buff del proceso root (O) en el buff de recepción
        for(int i = 1; i < numprocs; i++){                                                  //Espera por cada uno de los procesos restantes
            MPI_Recv(buff, count, datatype, MPI_ANY_SOURCE, 0, comm, MPI_STATUS_IGNORE);
            *(int *)recvbuff += *(int*)buff;                                                //Va sumando los valores que va recibiendo en buff al buff de recepción
        }
    }
    else{
        MPI_Send(buff, count, datatype, root, 0, comm);                                     //Si no es el proceso root, envia al root
    }   
}

void inicializaCadena(char *cadena, int n){
    int i;
    for(i=0; i<n/2; i++){
        cadena[i] = 'A';
    }
    for(i=n/2; i<3*n/4; i++){
        cadena[i] = 'C';
    }
    for(i=3*n/4; i<9*n/10; i++){
        cadena[i] = 'G';
    }
    for(i=9*n/10; i<n; i++){
        cadena[i] = 'T';
    }
}

    //realizar pruebas con n = 10, n = 13, n = 21 y np = 10, np = 13, np = 21 y en secuencial. Todos tienen que dar lo mismo que en secuencial.
    // 10A 5 / 10C 2 / 10G 2 / 10T 1
    // 13A 6 / 13C 3 / 13G 2 / 13T 2
    // 21A 10 / 21C 5 / 21G 3 / 21T 3