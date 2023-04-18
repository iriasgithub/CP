#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

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

int main(int argc, char *argv[])
{
  int my_id, numprocs, n, count=0, sum_total = 0;
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
      


        //El proceso 0 hace un send a todos, el resto de procesos recibe
      for (int i = 1; i < numprocs; i++){
        MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Send(&L, 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
      }
      
  }
  else{
    MPI_Recv (&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv (&L, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  //Inicializamos la cadena -> todas las cadenas seran la misma EN VALOR
  cadena = (char *) malloc(n*sizeof(char));
  inicializaCadena(cadena, n);

  //Hacemos las sumas por posiciones
  for(int i = my_id; i < n; i += numprocs){
    if(cadena[i] == L){
      count++;
    }
  }

  if(my_id == 0){    
      sum_total = count;                         
      for (int i = 1; i < numprocs; i++){
        MPI_Recv (&count, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        sum_total += count;
      }
      printf("El numero de apariciones de la letra %c es %d\n", L, sum_total);
  }
  else{
    MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }

  free(cadena);
  MPI_Finalize();
  exit(0);
}
