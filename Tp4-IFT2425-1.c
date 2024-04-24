//------------------------------------------------------
// module  : Tp4-IFT2425-1.c
// author  : 
// date    : 
// version : 1.0
// language: C++
// note    :
//------------------------------------------------------
//  

//------------------------------------------------
// FICHIERS INCLUS -------------------------------
//------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <new>
#include <unistd.h>

/************************************************************************/
/* WINDOWS						          	*/
/************************************************************************/
#include <X11/Xutil.h>

Display   *display;
int	  screen_num;
int 	  depth;
Window	  root;
Visual*	  visual;
GC	  gc;

//------------------------------------------------
// DEFINITIONS -----------------------------------                       
//------------------------------------------------
#define CARRE(X) ((X)*(X))

#define OUTPUT_FILE "Tp4-Img-I.pgm"
#define VIEW_PGM    "xv" 
#define DEBUG 0

//-Cst-Modele
#define X_1 0.0
#define Y_1 1.0
#define X_2 -1.0/sqrt(2.0)
#define Y_2 -1.0/2.0
#define X_3 +1.0/2*sqrt(2.0)
#define Y_3 -1.0/2.0
#define C 0.25 
#define R 0.1
#define D 0.3

#define X_1_INI 0.2            
#define X_2_INI 0.0       
#define X_3_INI -1.6                  
#define X_4_INI 0.0 
 
//-Cst-Runge-Kutta
#define H            0.0001  
#define T_0          0.0                 
#define T_F         30.0 
#define NB_INTERV (T_F-T_0)/H
   
 //-Cst-Image                             
#define WIDTH  512
#define HEIGHT 512                  
#define MAX_X  4.0                
#define MAX_Y  4.0  
#define EVOL_GRAPH 3000
              
#define WHITE     255
#define GREYWHITE 230
#define GREY      200
#define GREYDARK  120
#define BLACK       0   

//------------------------------------------------
// GLOBAL CST ------------------------------------                       
//------------------------------------------------
float Xmin=0.0;
float Xmax=0.0;
float Ymin=0.0;
float Ymax=0.0; 

float xx_1=((WIDTH/MAX_X)*X_1)+(WIDTH/2);
float yy_1=(-(HEIGHT/MAX_Y)*Y_1)+(HEIGHT/2);
float xx_2=((WIDTH/MAX_X)*X_2)+(WIDTH/2);
float yy_2=(-(HEIGHT/MAX_Y)*Y_2)+(HEIGHT/2);
float xx_3=((WIDTH/MAX_X)*X_3)+(WIDTH/2);
float yy_3=(-(HEIGHT/MAX_Y)*Y_3)+(HEIGHT/2);

/************************************************************************/
/* OPEN_DISPLAY()							*/
/************************************************************************/
int open_display()
{
  if ((display=XOpenDisplay(NULL))==NULL)
   { printf("Connection impossible\n");
     return(-1); }

  else
   { screen_num=DefaultScreen(display);
     visual=DefaultVisual(display,screen_num);
     depth=DefaultDepth(display,screen_num);
     root=RootWindow(display,screen_num);
     return 0; }
}

/************************************************************************/
/* FABRIQUE_WINDOW()							*/
/* Cette fonction crée une fenetre X et l'affiche à l'écran.	        */
/************************************************************************/
Window fabrique_window(char *nom_fen,int x,int y,int width,int height,int zoom)
{
  Window                 win;
  XSizeHints      size_hints;
  XWMHints          wm_hints;
  XClassHint     class_hints;
  XTextProperty  windowName, iconName;

  char *name=nom_fen;

  if(zoom<0) { width/=-zoom; height/=-zoom; }
  if(zoom>0) { width*=zoom;  height*=zoom;  }

  win=XCreateSimpleWindow(display,root,x,y,width,height,1,0,255);

  size_hints.flags=PPosition|PSize|PMinSize;
  size_hints.min_width=width;
  size_hints.min_height=height;

  XStringListToTextProperty(&name,1,&windowName);
  XStringListToTextProperty(&name,1,&iconName);
  wm_hints.initial_state=NormalState;
  wm_hints.input=True;
  wm_hints.flags=StateHint|InputHint;
  class_hints.res_name=nom_fen;
  class_hints.res_class=nom_fen;

  XSetWMProperties(display,win,&windowName,&iconName,
                   NULL,0,&size_hints,&wm_hints,&class_hints);

  gc=XCreateGC(display,win,0,NULL);

  XSelectInput(display,win,ExposureMask|KeyPressMask|ButtonPressMask| 
               ButtonReleaseMask|ButtonMotionMask|PointerMotionHintMask| 
               StructureNotifyMask);

  XMapWindow(display,win);
  return(win);
}

/****************************************************************************/
/* CREE_XIMAGE()							    */
/* Crée une XImage à partir d'un tableau de float                          */
/* L'image peut subir un zoom.						    */
/****************************************************************************/
XImage* cree_Ximage(float** mat,int z,int length,int width)
{
  int lgth,wdth,lig,col,zoom_col,zoom_lig;
  float somme;
  unsigned char	 pix;
  unsigned char* dat;
  XImage* imageX;

  /*Zoom positiv*/
  /*------------*/
  if (z>0)
  {
   lgth=length*z;
   wdth=width*z;

   dat=(unsigned char*)malloc(lgth*(wdth*4)*sizeof(unsigned char));
   if (dat==NULL)
      { printf("Impossible d'allouer de la memoire.");
        exit(-1); }

  for(lig=0;lig<lgth;lig=lig+z) for(col=0;col<wdth;col=col+z)
   { 
    pix=(unsigned char)mat[lig/z][col/z];
    for(zoom_lig=0;zoom_lig<z;zoom_lig++) for(zoom_col=0;zoom_col<z;zoom_col++)
      { 
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+0)]=pix;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+1)]=pix;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+2)]=pix;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+3)]=pix; 
       }
    }
  } /*--------------------------------------------------------*/

  /*Zoom negatifv*/
  /*------------*/
  else
  {
   z=-z;
   lgth=(length/z);
   wdth=(width/z);

   dat=(unsigned char*)malloc(lgth*(wdth*4)*sizeof(unsigned char));
   if (dat==NULL)
      { printf("Impossible d'allouer de la memoire.");
        exit(-1); }

  for(lig=0;lig<(lgth*z);lig=lig+z) for(col=0;col<(wdth*z);col=col+z)
   {  
    somme=0.0;
    for(zoom_lig=0;zoom_lig<z;zoom_lig++) for(zoom_col=0;zoom_col<z;zoom_col++)
     somme+=mat[lig+zoom_lig][col+zoom_col];
           
     somme/=(z*z);    
     dat[((lig/z)*wdth*4)+((4*(col/z))+0)]=(unsigned char)somme;
     dat[((lig/z)*wdth*4)+((4*(col/z))+1)]=(unsigned char)somme;
     dat[((lig/z)*wdth*4)+((4*(col/z))+2)]=(unsigned char)somme;
     dat[((lig/z)*wdth*4)+((4*(col/z))+3)]=(unsigned char)somme; 
   }
  } /*--------------------------------------------------------*/

  imageX=XCreateImage(display,visual,depth,ZPixmap,0,(char*)dat,wdth,lgth,16,wdth*4);
  return (imageX);
}

//------------------------------------------------
// FUNCTIONS -------------------------------------                       
//------------------------------------------------
//-------------------------//
//-- Matrice de Double ----//
//-------------------------//
//---------------------------------------------------------
// Alloue de la memoire pour une matrice 1d de float
//----------------------------------------------------------
float* dmatrix_allocate_1d(int hsize)
 {
  float* matrix;
  matrix=new float[hsize]; return matrix; }

//----------------------------------------------------------
// Alloue de la memoire pour une matrice 2d de float
//----------------------------------------------------------
float** dmatrix_allocate_2d(int vsize,int hsize)
 {
  float** matrix;
  float *imptr;

  matrix=new float*[vsize];
  imptr=new float[(hsize)*(vsize)];
  for(int i=0;i<vsize;i++,imptr+=hsize) matrix[i]=imptr;
  return matrix;
 }

//----------------------------------------------------------
// Libere la memoire de la matrice 1d de float
//----------------------------------------------------------
void free_dmatrix_1d(float* pmat)
{ delete[] pmat; }

//----------------------------------------------------------
// Libere la memoire de la matrice 2d de float
//----------------------------------------------------------
void free_dmatrix_2d(float** pmat)
{ delete[] (pmat[0]);
  delete[] pmat;}

//----------------------------------------------------------
// SaveImagePgm                       
//----------------------------------------------------------                
void SaveImagePgm(char* name,float** mat,int lgth,int wdth)
{
 int i,j;
 char buff[300];
 FILE* fic;

  //--extension--
  strcpy(buff,name);

  //--ouverture fichier--
  fic=fopen(buff,"wb");
    if (fic==NULL) 
        { printf("Probleme dans la sauvegarde de %s",buff); 
          exit(-1); }
  printf("\n Sauvegarde de %s au format pgm\n",buff);

  //--sauvegarde de l'entete--
  fprintf(fic,"P5");
  fprintf(fic,"\n# IMG Module");
  fprintf(fic,"\n%d %d",wdth,lgth);
  fprintf(fic,"\n255\n");

  //--enregistrement--
  for(i=0;i<lgth;i++) for(j=0;j<wdth;j++) 
	fprintf(fic,"%c",(char)mat[i][j]);
   
  //--fermeture fichier--
  fclose(fic); 
}

//------------------------------------------------------------------------
// plot_point
//
// Affiche entre x dans [-MAX_X/2  MAX_X/2]
//               y dans [-MAX_Y/2  MAX_Y/2]                
//------------------------------------------------------------------------
void plot_point(float** MatPts,float** MatPict,int NbPts)
{
 int x_co,y_co;
 int i,j,k;

 //Init
 for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++)  MatPict[i][j]=GREYWHITE;

 for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++) 
   { if ((fabs(i-yy_1)+fabs(j-xx_1))<10) MatPict[i][j]=GREYDARK;
     if ((fabs(i-yy_2)+fabs(j-xx_2))<10) MatPict[i][j]=GREYDARK;
     if ((fabs(i-yy_3)+fabs(j-xx_3))<10) MatPict[i][j]=GREYDARK; }

 //Loop
 for(k=0;k<NbPts;k++)
    { x_co=(int)((WIDTH/MAX_X)*MatPts[k][0]);
      y_co=-(int)((HEIGHT/MAX_Y)*MatPts[k][1]);
      y_co+=(HEIGHT/2);
      x_co+=(WIDTH/2);
      if (DEBUG) printf("[%d::%d]",x_co,y_co); 
      if ((x_co<WIDTH)&&(y_co<HEIGHT)&&(x_co>0)&&(y_co>0)) 
	 MatPict[y_co][x_co]=BLACK; 
    }
}


//------------------------------------------------------------------------
// Fill_Pict
//------------------------------------------------------------------------
void Fill_Pict(float** MatPts,float** MatPict,int PtsNumber,int NbPts)
{
 int i,j;
 int x_co,y_co;
 int k,k_Init,k_End;

 //Init
 for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++) 
   { if (MatPict[i][j]!=GREYWHITE) MatPict[i][j]=GREY;
     if ((fabs(i-yy_1)+fabs(j-xx_1))<10) MatPict[i][j]=GREYDARK;
     if ((fabs(i-yy_2)+fabs(j-xx_2))<10) MatPict[i][j]=GREYDARK;
     if ((fabs(i-yy_3)+fabs(j-xx_3))<10) MatPict[i][j]=GREYDARK; }

 //Loop
 k_Init=PtsNumber;
 k_End=(k_Init+EVOL_GRAPH)%NbPts;
 for(k=k_Init;k<k_End;k++)
    { k=(k%NbPts);
      x_co=(int)((WIDTH/MAX_X)*MatPts[k][0]);
      y_co=-(int)((HEIGHT/MAX_Y)*MatPts[k][1]);
      y_co+=(HEIGHT/2);
      x_co+=(WIDTH/2);
      if ((x_co<WIDTH)&&(y_co<HEIGHT)&&(x_co>0)&&(y_co>0)) 
         MatPict[y_co][x_co]=BLACK; }
}


//------------------------------------------------
// FONCTIONS TPs----------------------------------                      
//------------------------------------------------
      

// Euler

void Euler (float t, float x, float y, float u, float v, float coordAimants[3][2], float *result, float **MatPts) {
  float sumX;
  float sumY;
  //float xy[2];
  for (int i = 0; i<(int)(NB_INTERV); i++) {
    sumX = 0;
    sumY = 0;
    x += H*u;
    y += H*v;
    for (int j=0;j<3;j++){
      sumX += (coordAimants[j][0]-x)/pow(sqrt(pow((coordAimants[j][0]-x),2) + pow((coordAimants[j][1]-y),2) + D*D),3);
    }
    for (int j=0;j<3;j++){
      sumY += (coordAimants[j][1]-y)/pow(sqrt(pow((coordAimants[j][0]-x),2) + pow((coordAimants[j][1]-y),2) + D*D),3);
    }
    u += H*(sumX - R*u -C*x);
    v += H*(sumY -R*v -C*y);

    
    MatPts[i][0]=(x); 
    MatPts[i][1]=(y); 
    

  }

  result[0] = x;
  result[1] = y;
  result[2] = u;
  result[3] = v;

}

//x
float f1(float u) {
  return u;
}

//y
float f2(float v) {
  return v;
}

//u
float f3(float x, float y, float u, float coordAimants[3][2]) {
  float sumX;
  for (int j=0;j<3;j++){
      sumX += (coordAimants[j][0]-x)/pow(sqrt(pow((coordAimants[j][0]-x),2) + pow((coordAimants[j][1]-y),2) + D*D),3);
    }
    //printf("sumX=%f \t", sumX);
    return (sumX - R*u -C*x);
}

//v
float f4(float x, float y, float v, float coordAimants[3][2]) {
  float sumY;
  for (int j=0;j<3;j++){
    sumY += (coordAimants[j][1]-y)/pow(sqrt(pow((coordAimants[j][0]-x),2) + pow((coordAimants[j][1]-y),2) + D*D),3);
  }
  //printf("sumY=%f \t", sumY);
  return (sumY-R*v-C*y);
}


void rungeKutta(float t, float x, float y, float u, float v, float coordAimants[3][2], float *result, float **MatPts) {
  float k11, k12, k13, k14, k21, k22, k23, k24, k31, k32, k33, k34, k41, k42, k43, k44, k51, k52, k53, k54, k61, k62, k63, k64;

  for (int i=0; i<NB_INTERV; i++){
    k11 = H*f1(u);
    k12 = H*f2(v);
    k13 = H*f3(x,y,u,coordAimants);
    k14 = H*f4(x,y,v,coordAimants);

    k21 = H*f1(u+k13/4.0);
    k22 = H*f2(v+k14/4.0);
    k23 = H*f3(x+k11/4.0, y+k12/4.0, u+k13/4.0,coordAimants);
    k24 = H*f4(x+k11/4.0, y+k12/4.0, v+k14/4.0,coordAimants);

    k31 = H*f1(u+3.0/32*k13+9.0/32*k23);
    k32 = H*f2(v+3.0/32*k14+9.0/32*k24);
    k33 = H*f3(x+3.0/32*k11+9.0/32*k21, y+3.0/32*k12+9.0/32*k22, u+3.0/32*k13+9.0/32*k23, coordAimants);
    k34 = H*f4(x+3.0/32*k11+9.0/32*k21, y+3.0/32*k12+9.0/32*k22, v+3.0/32*k14+9.0/32*k24, coordAimants);

    k41 = H*f1(u+(1932.0*k13+7200.0*k23+7296.0*k33)/2197);
    k42 = H*f2(v+(1932.0*k14+7200.0*k24+7296.0*k34)/2197);
    k43 = H*f3(x+(1932.0*k11+7200.0*k21+7296.0*k31)/2197, y+(1932.0*k12+7200.0*k22+7296.0*k32)/2197, u+(1932.0*k13+7200.0*k23+7296.0*k33)/2197, coordAimants);
    k44 = H*f4(x+(1932.0*k11+7200.0*k21+7296.0*k31)/2197, y+(1932.0*k12+7200.0*k22+7296.0*k32)/2197, v+(1932.0*k14+7200.0*k24+7296.0*k34)/2197, coordAimants);

    k51 = H*f1(u+439.0/216*k13-8.0*k23+3680.0/513*k33-845.0/4104*k43);
    k52 = H*f2(v+439.0/216*k14-8.0*k24+3680.0/513*k34-845.0/4104*k44);
    k53 = H*f3(x+439.0/216*k11-8.0*k21+3680.0/513*k31-845.0/4104*k41, y+439.0/216*k12-8.0*k22+3680.0/513*k32-845.0/4104*k42, u+439.0/216*k13-8.0*k23+3680.0/513*k33-845.0/4104*k43, coordAimants);
    k54 = H*f4(x+439.0/216*k11-8.0*k21+3680.0/513*k31-845.0/4104*k41, y+439.0/216*k12-8.0*k22+3680.0/513*k32-845.0/4104*k42, v+439.0/216*k14-8.0*k24+3680.0/513*k34-845.0/4104*k44, coordAimants);

    k61 = H*f1(u-8.0/27*k13+2.0*k23-3544.0/2565*k33+1859.0/4104*k43-11.0/40*k53);
    k62 = H*f2(v-8.0/27*k14+2.0*k24-3544.0/2565*k34+1859.0/4104*k44-11.0/40*k54);
    k63 = H*f3(x-8.0/27*k11+2.0*k21-3544.0/2565*k31+1859.0/4104*k41-11.0/40*k51, y-8.0/27*k12+2.0*k22-3544.0/2565*k32+1859.0/4104*k42-11.0/40*k52, u-8.0/27*k13+2.0*k23-3544.0/2565*k33+1859.0/4104*k43-11.0/40*k53, coordAimants);
    k64 = H*f4(x-8.0/27*k11+2.0*k21-3544.0/2565*k31+1859.0/4104*k41-11.0/40*k51, y-8.0/27*k12+2.0*k22-3544.0/2565*k32+1859.0/4104*k42-11.0/40*k52, v-8.0/27*k14+2.0*k24-3544.0/2565*k34+1859.0/4104*k44-11.0/40*k54, coordAimants);

    x += 16.0/135*k11+6656.0/12825*k31+28561.0/56430*k41-9.0/50*k51+2.0/55*k61;
    y += 16.0/135*k12+6656.0/12825*k32+28561.0/56430*k42-9.0/50*k52+2.0/55*k62;
    u += 16.0/135*k13+6656.0/12825*k33+28561.0/56430*k43-9.0/50*k53+2.0/55*k63;
    v += 16.0/135*k14+6656.0/12825*k34+28561.0/56430*k44-9.0/50*k54+2.0/55*k64;

    //printf("i=%d, x=%f, y=%f, u=%f, v=%f\n", i, x,y,u,v);

  }
  result[0] = x;
  result[1] = y;
  result[2] = u;
  result[3] = v;

}



//----------------------------------------------------------
//----------------------------------------------------------
// MAIN  
//----------------------------------------------------------
//----------------------------------------------------------
int main (int argc, char **argv)
{
  int i,j,k;
  int flag_graph;
  int zoom;

  XEvent ev;
  Window win_ppicture;
  XImage *x_ppicture;
  char   nomfen_ppicture[100]; 
  char BufSystVisu[100];

  //>AllocMemory
  float** MatPict=dmatrix_allocate_2d(HEIGHT,WIDTH);
  float** MatPts=dmatrix_allocate_2d((int)(NB_INTERV),2);
  
  //>Init
  for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++) MatPict[i][j]=GREYWHITE;
  for(i=0;i<2;i++) for(j=0;j<(int)(NB_INTERV);j++) MatPts[i][j]=0.0;
  flag_graph=1;
  zoom=1;


  //---------------------------------------------------------------------
  //>Question 1 
  //---------------------------------------------------------------------  

  float coordAimants[3][2] = {{X_1,Y_1}, {X_2, Y_2}, {X_3, Y_3}};
  //float C = 0.25;
  //float R = 0.1;
  //float D = 0.3;

  float result[4];    //[x,y,u,v]
  float x = 3.0/32;

  
  Euler(0, 0.2, -1.6, 0, 0, coordAimants, result, MatPts);
  //rungeKutta(0, 1.2, -1.6, 0, 0, coordAimants, result, MatPts);

  printf("x=%f, y=%f, u=%f, v=%f\n, 3/32 = %f", result[0], result[1], result[2], result[3], x);




  //Il faut travailler ici ...et dans > // FONCTIONS TPs

  //Un exemple ou la matrice de points est remplie
  //par une courbe donné par l'équation d'en bas... et non pas par 
  //la solution de l'équation différentielle
 
  for(k=0;k<(int)(NB_INTERV);k++)
    { 
      //MatPts[k][0]=(k/(float)(NB_INTERV))*cos((k*0.0001)*3.14159); 
      //MatPts[k][1]=(k/(float)(NB_INTERV))*sin((k*0.001)*3.14159); 
      //>on peut essayer la ligne d'en bas aussi
      //MatPts[k][1]=(k/(float)(NB_INTERV))*sin((k*0.0001)*3.14159); 
     }


  //--Fin Question 1-----------------------------------------------------


  //>Affichage des Points dans MatPict
  plot_point(MatPts,MatPict,(int)(NB_INTERV));

  //>Save&Visu de MatPict
  SaveImagePgm((char*)OUTPUT_FILE,MatPict,HEIGHT,WIDTH);
  strcpy(BufSystVisu,VIEW_PGM);
  strcat(BufSystVisu," "); 	
  strcat(BufSystVisu,OUTPUT_FILE);
  strcat(BufSystVisu," &"); 
  system(BufSystVisu);

  //>Affiche Statistique
  printf("\n\n Stat:  Xmin=[%.2f] Xmax=[%.2f] Ymin=[%.2f] Ymax=[%.2f]\n",Xmin,Xmax,Ymin,Ymax);


 //--------------------------------------------------------------------------------
 //-------------- visu sous XWINDOW de l'évolution de MatPts ----------------------
 //--------------------------------------------------------------------------------
 if (flag_graph)
 {
 //>Uuverture Session Graphique
 if (open_display()<0) printf(" Impossible d'ouvrir une session graphique");
 sprintf(nomfen_ppicture,"Évolution du Graphe");
 win_ppicture=fabrique_window(nomfen_ppicture,10,10,HEIGHT,WIDTH,zoom);
 x_ppicture=cree_Ximage(MatPict,zoom,HEIGHT,WIDTH);

 printf("\n\n Pour quitter,appuyer sur la barre d'espace");
 fflush(stdout);

 //>Boucle Evolution
  for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++) MatPict[i][j]=GREYWHITE;
  for(k=0;;)
     {   
       k=((k+EVOL_GRAPH)%(int)(NB_INTERV));
       Fill_Pict(MatPts,MatPict,k,(int)(NB_INTERV));
       XDestroyImage(x_ppicture);
       x_ppicture=cree_Ximage(MatPict,zoom,HEIGHT,WIDTH);
       XPutImage(display,win_ppicture,gc,x_ppicture,0,0,0,0,x_ppicture->width,x_ppicture->height); 
       usleep(10000);  //si votre machine est lente mettre un nombre plus petit
     }
 } 
       
 //>Retour  
 printf("\n Fini... \n\n\n");
 return 0;
}



