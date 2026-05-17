#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "TypesAndDefs.h"
#include "DataBase.h"
#include "Cosmology.h"
#include "MT_Random.h"

void CosmoClass::read_inputs(const std::string& filename)
{
    std::ifstream infile(filename);

    if (!infile.is_open()) {
        std::cerr << "Error opening file: "
                  << filename << std::endl;
        return;
    }

    std::string line;

    while (std::getline(infile, line))
    {
        // Remove comments beginning with //
        std::size_t comment_pos = line.find("//");
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }

        // Remove spaces/tabs
        line.erase(
            std::remove_if(line.begin(), line.end(), ::isspace),
            line.end()
        );

        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        // Find '='
        std::size_t eq_pos = line.find('=');

        if (eq_pos == std::string::npos) {
            continue;
        }

        std::string key   = line.substr(0, eq_pos);
        std::string value = line.substr(eq_pos + 1);

        // Assign values
        if (key == "hubble")      h = std::stod(value);
        else if (key == "Omega_m")     Omega_m = std::stod(value);
        else if (key == "Omega_bar")   Omega_bar = std::stod(value);
        else if (key == "Omega_nu")    Omega_nu = std::stod(value);
        else if (key == "Omega_r")     Omega_r = std::stod(value);
        else if (key == "n_s")         n_s = std::stod(value);
        else if (key == "w_de")        w_de = std::stod(value);
    }
}

/* Growth factor for flat wCDM cosmologies: */

void CosmoClass::GrowthFactor(real z, real *gf, real *g_dot){
   real x1, x2, dplus, ddot;
   const float redshift=200.0;
   real *ystart;

   x1 = 1.0/(1.0+100000.0);
   x2 = 1.0/(1.0+z);
   ystart = (real *)malloc(2*sizeof(real));
   ystart[0] = x1;
   ystart[1] = 0.0;

   odesolve(ystart, 2, x1, x2, 1.0e-6, 1.0e-6, &CosmoClass::growths, false);
      //printf("Dplus = %f;  Ddot = %f \n", ystart[0], ystart[1]);

   dplus = ystart[0];
   ddot  = ystart[1];
   x1 = 1.0/(1.0+100000.0);
   x2 = 1.0;
   ystart[0] = x1;
   ystart[1] = 0.0;

   odesolve(ystart, 2, x1, x2, 1.0e-6, 1.0e-6, &CosmoClass::growths, false);
      //printf("Dplus = %f;  Ddot = %f \n", ystart[0], ystart[1]);

   *gf    = dplus/ystart[0];
   *g_dot = ddot/ystart[0];
      //printf("\nGrowth factor = %f;  Derivative = %f \n", dplus/ystart[0], ddot/ystart[0]);
   free(ystart);

   return;
}

void CosmoClass::growths(real a, real y[], real dydx[]){
   real H;
   H = sqrt(Omega_r/(a*a*a*a) + Omega_m/(a*a*a) + (1.0-Omega_m-Omega_r)*pow(a, -3.0*(1.0+w_de)));
   dydx[0] = y[1]/(a*H);
   dydx[1] = -2.0*y[1]/a + 1.5*Omega_m*y[0]/(H*pow(a, 4.0));
   return;
}


#define MAXSTP 10000
#define TINY 1.0e-30
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
void CosmoClass::odesolve(real ystart[], int nvar, real x1, real x2, real eps, real h1,
                          void (CosmoClass::*derivs)(real, real [], real []), bool print_stat)
{
   int i, nstp, nok, nbad, feval;
   real x,hnext,hdid,h;
   real *yscal,*y,*dydx;
   const real hmin=0.0;

   feval = 0;
   yscal= (real *)malloc(nvar*sizeof(real));
   y= (real *)malloc(nvar*sizeof(real));
   dydx= (real *)malloc(nvar*sizeof(real));
   x=x1;
   h=SIGN(h1,x2-x1);
   nok = nbad = 0;
   for (i=0; i<nvar; ++i) {y[i]=ystart[i];}

   for (nstp=0; nstp<MAXSTP; ++nstp) {
      (this->*derivs)(x, y, dydx);
      ++feval;
      for (i=0; i<nvar; ++i)
      {yscal[i]=fabs(y[i])+fabs(dydx[i]*h)+TINY;}
      if ((x+h-x2)*(x+h-x1) > 0.0) h=x2-x;
      rkqs(y,dydx,nvar,&x,h,eps,yscal,&hdid,&hnext,&feval,derivs);
      if (hdid == h) ++nok; else ++nbad;
      if ((x-x2)*(x2-x1) >= 0.0) {
         for (i=0; i<nvar; ++i) {ystart[i]=y[i];}
         free(dydx);
         free(y);
         free(yscal);
         if (print_stat){
            printf("ODEsolve:\n");
            printf(" Evolved from x = %f to x = %f\n", x1, x2);
            printf(" successful steps: %d\n", nok);
            printf(" bad steps: %d\n", nbad);
            printf(" function evaluations: %d\n", feval);
         }
         return;
      }
      if (fabs(hnext) <= hmin) {
         printf("Step size too small in ODEsolve");
         exit(1);
      }
      h=hnext;
   }
   printf("Too many steps in ODEsolve");
   exit(1);
}
#undef MAXSTP
#undef TINY
#undef SIGN

#define SAFETY 0.9
#define PGROW -0.2
#define PSHRNK -0.25
#define ERRCON 1.89e-4
static real maxarg1,maxarg2, minarg1, minarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ? (maxarg1) : (maxarg2))
#define FMIN(a,b) (minarg1=(a),minarg2=(b),(minarg1) < (minarg2) ? (minarg1) : (minarg2))
void CosmoClass::rkqs(real y[], real dydx[], int n, real *x, real htry, real eps,
                      real yscal[], real *hdid, real *hnext, int *feval,
                      void (CosmoClass::*derivs)(real, real [], real []))
{
   int i;
   real errmax,h,htemp,xnew,*yerr,*ytemp;

   yerr= (real *)malloc(n*sizeof(real));
   ytemp= (real *)malloc(n*sizeof(real));
   h=htry;

   for (;;) {
      rkck(y,dydx,n,*x,h,ytemp,yerr,derivs);
      *feval += 5;
      errmax=0.0;
      for (i=0; i<n; ++i) {errmax=FMAX(errmax,fabs(yerr[i]/yscal[i]));}
      errmax /= eps;
      if (errmax <= 1.0) break;
      htemp=SAFETY*h*pow(errmax,PSHRNK);
      h=(h >= 0.0 ? FMAX(htemp,0.1*h) : FMIN(htemp,0.1*h));
      xnew=(*x)+h;
      if (xnew == *x) {
         printf("Stepsize underflow in ODEsolve rkqs");
         exit(1);
      }
   }
   if (errmax > ERRCON) *hnext=SAFETY*h*pow(errmax,PGROW);
   else *hnext=5.0*h;
   *x += (*hdid=h);
   for (i=0; i<n; ++i) {y[i]=ytemp[i];}
   free(ytemp);
   free(yerr);
}
#undef SAFETY
#undef PGROW
#undef PSHRNK
#undef ERRCON
#undef FMAX
#undef FMIN


/* Cash-Karp Runge-Kutta Step, based on the
work of Fehlberg, who ﬁgured out that six function evaluations could
be used to determine two Runge-Kutta steps, one fourth-order and one
ﬁfth-order. The diﬀerence between these estimates can be used as a
truncation error for adjusting the stepsize. */
void CosmoClass::rkck(real y[], real dydx[], int n, real x, real h, real yout[],
                      real yerr[], void (CosmoClass::*derivs)(real, real [], real []))
{
   int i;
   static real a2=0.2,a3=0.3,a4=0.6,a5=1.0,a6=0.875,b21=0.2,
   b31=3.0/40.0,b32=9.0/40.0,b41=0.3,b42 = -0.9,b43=1.2,
   b51 = -11.0/54.0, b52=2.5,b53 = -70.0/27.0,b54=35.0/27.0,
   b61=1631.0/55296.0,b62=175.0/512.0,b63=575.0/13824.0,
   b64=44275.0/110592.0,b65=253.0/4096.0,c1=37.0/378.0,
   c3=250.0/621.0,c4=125.0/594.0,c6=512.0/1771.0,
   dc5 = -277.00/14336.0;
   real dc1=c1-2825.0/27648.0,dc3=c3-18575.0/48384.0,
   dc4=c4-13525.0/55296.0,dc6=c6-0.25;
   real *ak2,*ak3,*ak4,*ak5,*ak6,*ytemp;

   ak2= (real *)malloc(n*sizeof(real));
   ak3= (real *)malloc(n*sizeof(real));
   ak4= (real *)malloc(n*sizeof(real));
   ak5= (real *)malloc(n*sizeof(real));
   ak6= (real *)malloc(n*sizeof(real));
   ytemp= (real *)malloc(n*sizeof(real));

   for (i=0; i<n; ++i)
      ytemp[i]=y[i]+b21*h*dydx[i];
   (this->*derivs)(x+a2*h,ytemp,ak2);
   for (i=0; i<n; ++i)
      ytemp[i]=y[i]+h*(b31*dydx[i]+b32*ak2[i]);
   (this->*derivs)(x+a3*h,ytemp,ak3);
   for (i=0; i<n; ++i)
      ytemp[i]=y[i]+h*(b41*dydx[i]+b42*ak2[i]+b43*ak3[i]);
   (this->*derivs)(x+a4*h,ytemp,ak4);
   for (i=0; i<n; ++i)
      ytemp[i]=y[i]+h*(b51*dydx[i]+b52*ak2[i]+b53*ak3[i]+b54*ak4[i]);
   (this->*derivs)(x+a5*h,ytemp,ak5);
   for (i=0; i<n; ++i)
      ytemp[i]=y[i]+h*(b61*dydx[i]+b62*ak2[i]+b63*ak3[i]+b64*ak4[i]+b65*ak5[i]);
   (this->*derivs)(x+a6*h,ytemp,ak6);
   for (i=0; i<n; ++i)
      yout[i]=y[i]+h*(c1*dydx[i]+c3*ak3[i]+c4*ak4[i]+c6*ak6[i]);
   for (i=0; i<n; ++i)
      yerr[i]=h*(dc1*dydx[i]+dc3*ak3[i]+dc4*ak4[i]+dc5*ak5[i]+dc6*ak6[i]);

   free(ytemp);
   free(ak6);
   free(ak5);
   free(ak4);
   free(ak3);
   free(ak2);
}

CosmoClass::~CosmoClass()
{
}
