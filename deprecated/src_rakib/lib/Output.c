/*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *<LicenseText>
 *
 * CitcomS by Louis Moresi, Shijie Zhong, Lijie Han, Eh Tan,
 * Clint Conrad, Michael Gurnis, and Eun-seo Choi.
 * Copyright (C) 1994-2005, California Institute of Technology.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *</LicenseText>
 *
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
/* Routine to process the output of the finite element cycles
   and to turn them into a coherent suite  files  */


#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <mpi.h>
#include "element_definitions.h"
#include "global_defs.h"
#include "parsing.h"
#include "output.h"

void output_comp_nd(struct All_variables *, int);
void output_comp_el(struct All_variables *, int);
void output_coord(struct All_variables *);
void output_mat(struct All_variables *);
void output_velo(struct All_variables *, int);
void output_visc_prepare(struct All_variables *, float **);
void output_visc(struct All_variables *, int);
void output_surf_botm(struct All_variables *, int);
void output_geoid(struct All_variables *, int);
void output_stress(struct All_variables *, int);
void output_horiz_avg(struct All_variables *, int);
void output_tracer(struct All_variables *, int);
void output_tracer_dens(struct All_variables *, int);
void output_pressure(struct All_variables *, int);
void output_heating(struct All_variables *, int);
void output_heating_visc_nd(struct All_variables *, int); /* XXX DJB */
void output_heating_adi_nd(struct All_variables *, int); /* XXX DJB */
void output_heating_latent_nd(struct All_variables *, int); /* XXX DJB */
void output_temp_sph(struct All_variables *, int);
void output_comp_sph(struct All_variables *, int);
void output_sten_temp(struct All_variables *, int); /* XXX DJB */
void output_sten_velo(struct All_variables *, int); /* XXX DJB */
void output_divv(struct All_variables *, int); /* XXX DJB */

extern void parallel_process_termination();
extern void heat_flux(struct All_variables *);
extern void get_STD_topo(struct All_variables *, float**, float**,
                         float**, float**, int);
extern void get_CBF_topo(struct All_variables *, float**, float**);

/**********************************************************************/

void output_common_input(struct All_variables *E)
{
    int m = E->parallel.me;

    input_string("output_format", E->output.format, "ascii",m);
    input_string("output_optional", E->output.optional, "surf,botm,tracer",m);

    /* gzdir type of I/O */
    E->output.gzdir.vtk_io = 0;
    E->output.gzdir.rnr = 0;
    if(strcmp(E->output.format, "ascii-gz") == 0){
      /*
	 vtk_io = 1: write files for post-processing into VTK
	 vtk_io = 2: write serial legacy VTK file
	 vtk_io = 3: write paralle legacy VTK file

      */
      input_int("gzdir_vtkio",&(E->output.gzdir.vtk_io),"0",m);
      /* remove net rotation on output step? */
      input_boolean("gzdir_rnr",&(E->output.gzdir.rnr),"off",m);
      E->output.gzdir.vtk_base_init = 0;
      E->output.gzdir.vtk_base_save = 1; /* should we save the basis vectors? (memory!) */
      //fprintf(stderr,"gzdir: vtkio: %i save basis vectors: %i\n",
      //      E->output.gzdir.vtk_io,E->output.gzdir.vtk_base_save);
    }
}



void output(struct All_variables *E, int cycles)
{
   
  if (cycles == 0) {
    output_coord(E);
    output_domain(E);
    /*output_mat(E);*/

    if (E->output.coord_bin)
        output_coord_bin(E);

    
    /*-----------------------------------------------------------------------------
     * Initialize shell-output
     *-----------------------------------------------------------------------------*/
    if((E->output.so.initialized==0) && (E->output.output_num_shells>0))
    {
        int i=0,j=0;
        E->output.so.sd = (ShellData**)malloc(sizeof(ShellData*)*E->output.output_num_shells);
        for(i=0; i<E->output.output_num_shells; i++)
        {
            E->output.so.sd[i] = (ShellData*)malloc(sizeof(ShellData)*E->output.output_shell_num_buffered_timesteps);
        }

        for(i=0; i<E->output.output_num_shells; i++)
        {
            for(j=0; j<E->output.output_shell_num_buffered_timesteps; j++)
            {
                E->output.so.sd[i][j].theta          = (float*) malloc(sizeof(float)*E->lmesh.nsf);
                E->output.so.sd[i][j].phi            = (float*) malloc(sizeof(float)*E->lmesh.nsf);
                E->output.so.sd[i][j].r              = (float*) malloc(sizeof(float)*E->lmesh.nsf);
                E->output.so.sd[i][j].temperature    = (float*) malloc(sizeof(float)*E->lmesh.nsf);
                E->output.so.sd[i][j].vr             = (float*) malloc(sizeof(float)*E->lmesh.nsf);
            }
        }
        E->output.so.initialized = 1;
    }
    
  }


  output_velo(E, cycles);
  output_visc(E, cycles);

  output_surf_botm(E, cycles);

  /* optional output below */

  /* compute and output geoid (in spherical harmonics coeff) */
  if (E->output.geoid)		/* this needs to be called after the
				   surface and bottom topo has been
				   computed! */
      output_geoid(E, cycles);

  if (E->output.stress){
       output_stress(E, cycles);
  }
  if (E->output.pressure)
    output_pressure(E, cycles);

  if (E->output.horiz_avg)
      output_horiz_avg(E, cycles);

  if (E->output.seismic)
      output_seismic(E, cycles);

  if (E->output.comp_sph && E->composition.on) /* DJB */
      output_comp_sph(E, cycles);

  if (E->output.temp_sph) /* DJB */
      output_temp_sph(E, cycles);

  if(E->output.tracer && E->control.tracer)
      output_tracer(E, cycles);

  if(E->output.tracer_dens && E->control.tracer)
      output_tracer_dens(E, cycles);

  if (E->output.comp_nd && E->composition.on)
      output_comp_nd(E, cycles);

  if (E->output.comp_el && E->composition.on)
      output_comp_el(E, cycles);

  if(E->output.heating && E->control.disptn_number != 0)
      output_heating(E, cycles);

  if (E->output.heating_visc_nd && E->control.disptn_number != 0)
      output_heating_visc_nd(E, cycles); /* DJB */

  if (E->output.heating_adi_nd && E->control.disptn_number != 0)
      output_heating_adi_nd(E, cycles); /* DJB */

  if (E->output.heating_latent_nd && E->control.disptn_number != 0)
      output_heating_latent_nd(E, cycles); /* DJB */

  if (E->output.sten_temp)
      output_sten_temp(E, cycles); /* DJB */

  if (E->output.sten_velo)
      output_sten_velo(E, cycles); /* DJB */

  if (E->output.divv)
      output_divv(E, cycles); /* DJB */

  return;
}


FILE* output_open(char *filename, char *mode)
{
  FILE *fp1;

  /* if filename is empty, output to stderr. */
  if (*filename) {
    fp1 = fopen(filename,mode);
    if (!fp1) {
      fprintf(stderr,"Cannot open file '%s' for '%s'\n",
	      filename,mode);
      parallel_process_termination();
    }
  }
  else
    fp1 = stderr;

  return fp1;
}


void output_coord(struct All_variables *E)
{
  int i, j;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.coord.%d",E->control.data_file,E->parallel.me);
  fp1 = output_open(output_file, "w");

  for(j=1;j<=E->sphere.caps_per_proc;j++)     {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++)
      fprintf(fp1,"%.6e %.6e %.6e\n",E->sx[j][1][i],E->sx[j][2][i],E->sx[j][3][i]);
  }

  fclose(fp1);

  return;
}

void output_shell(struct All_variables *E, int cycles)
{
    int i=0, j=0, c=0;
    int proc = E->parallel.me;
    
    if(E->output.output_num_shells == 0) return;

    if(cycles!=0)
    {
        if(E->output.so.skip_count < E->output.output_shell_every)
        {
            E->output.so.skip_count++;
            return;
        }
        else
        {
            E->output.so.skip_count=0;
        }
    }

    for(i=0; i<E->output.output_num_shells; i++)
    {
        E->output.so.sd[i][E->output.so.save_count].ts = cycles;
    }

    for(c=0; c<E->output.output_num_shells; c++)
    {
        for(j=1;j<=E->sphere.caps_per_proc;j++)  
        {
            int count = 0;
            for(i=1;i<=E->lmesh.nno;i++)   
            {
                if(fabs(E->sphere.gr[E->output.output_shell_rids[c]+1] - E->sx[j][3][i]) < 0.0001)
                {
                    E->output.so.sd[c][E->output.so.save_count].theta[count]          = E->sx[j][1][i];
                    E->output.so.sd[c][E->output.so.save_count].phi[count]            = E->sx[j][2][i];
                    E->output.so.sd[c][E->output.so.save_count].r[count]              = E->sx[j][3][i];
                    E->output.so.sd[c][E->output.so.save_count].temperature[count]    = E->T[j][i];
                    E->output.so.sd[c][E->output.so.save_count].vr[count]             = E->sphere.cap[j].V[3][i];

                    count++;
                }
            }

            if((count>0) && (count!=E->lmesh.nsf))
            {
                fprintf(stderr, "Number of shell-nodes mismatch detected..! Aborting.. %d %d\n",
                        count, E->lmesh.nsf);
                parallel_process_termination();
            }        
        }
    }
    
    E->output.so.save_count++;

    if(E->output.so.save_count == E->output.output_shell_num_buffered_timesteps)
    {
        int k=0, m=0;
                
        /*-----------------------------------------------------------------------------
         * Save shells
         *-----------------------------------------------------------------------------*/
        for(k=0; k<E->output.so.save_count; k++)
        {
            char output_file[255];
            FILE *fp1;
            sprintf(output_file,"%s.shell.%d.%d", E->control.data_file,
                    proc, E->output.so.sd[0][k].ts);
            fp1 = output_open(output_file, "w");

            for(c=0; c<E->output.output_num_shells; c++)
            {
                for(m=0; m<E->lmesh.nsf; m++)
                {
                    fprintf(fp1,"%.6e %.6e %.6e %.6e %.6e\n",   E->output.so.sd[c][k].theta[m], 
                                                                E->output.so.sd[c][k].phi[m],
                                                                E->output.so.sd[c][k].r[m],
                                                                E->output.so.sd[c][k].temperature[m],
                                                                E->output.so.sd[c][k].vr[m]);
                }
            }

            fclose(fp1);            
        }

        E->output.so.save_count = 0;
    }

    return;
}

void output_domain(struct All_variables *E)
{
    /* This routine outputs the domain bounds in a single file. */
    /* The file will be useful for external program to understand */
    /* how the CitcomS mesh is domain decomposed. */

    /* Note: rank-0 writes the domain bounds of all processors */

    const int j = 1;
    const int tag = 0;
    const int receiver = 0;
    const int nox = E->lmesh.nox;
    const int noy = E->lmesh.noy;
    const int noz = E->lmesh.noz;
    const int corner_nodes[4] = {1,
                                 1 + noz*(nox-1),
                                 nox*noy*noz - (noz -1),
                                 1 + noz*nox*(noy-1)};
    /* Each line has so many columns:
     * The columns are min(r) and max(r),
     * then (theta, phi) of 4 bottom corners. */
#define ncolumns 10

    double buffer[ncolumns];

    buffer[0] = E->sx[j][3][1];
    buffer[1] = E->sx[j][3][noz];
    buffer[2] = E->sx[j][1][corner_nodes[0]];
    buffer[3] = E->sx[j][2][corner_nodes[0]];
    buffer[4] = E->sx[j][1][corner_nodes[1]];
    buffer[5] = E->sx[j][2][corner_nodes[1]];
    buffer[6] = E->sx[j][1][corner_nodes[2]];
    buffer[7] = E->sx[j][2][corner_nodes[2]];
    buffer[8] = E->sx[j][1][corner_nodes[3]];
    buffer[9] = E->sx[j][2][corner_nodes[3]];

    if(E->parallel.me == 0) {
        int i, rank;
        char output_file[255];
        FILE *fp1;
        int32_t header[4];
        MPI_Status status;

        sprintf(output_file,"%s.domain",E->control.data_file);
        fp1 = output_open(output_file, "wb");

        /* header */
        header[0] = E->parallel.nproc;
        header[1] = ncolumns;
        header[2] = 0x12345678;  /* guard */
        header[3] = sizeof(int32_t);
        fwrite(header, sizeof(int32_t), 4, fp1);

        /* bounds of self */
        fwrite(buffer, sizeof(double), ncolumns, fp1);

        /* bounds of other processors */
        for(rank=1; rank<E->parallel.nproc; rank++) {
            MPI_Recv(buffer, ncolumns, MPI_DOUBLE, rank, tag, E->parallel.world, &status);
            fwrite(buffer, sizeof(double), ncolumns, fp1);
        }

        fclose(fp1);
    }
    else {
        MPI_Send(buffer, ncolumns, MPI_DOUBLE, receiver, tag, E->parallel.world);
    }

#undef ncolumns

    return;
}


/* write coordinates in binary double */
void output_coord_bin(struct All_variables *E)
{
  int i, j;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.coord_bin.%d",E->control.data_file,E->parallel.me);
  fp1 = output_open(output_file, "wb");

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
      int32_t header[4];
      header[0] = E->lmesh.nox;
      header[1] = E->lmesh.noy;
      header[2] = E->lmesh.noz;
      header[3] = 0x12345678; /* guard */
      fwrite(header, sizeof(int32_t), 4, fp1);

      fwrite(&(E->x[j][1][1]), sizeof(double), E->lmesh.nno, fp1);
      fwrite(&(E->x[j][2][1]), sizeof(double), E->lmesh.nno, fp1);
      fwrite(&(E->x[j][3][1]), sizeof(double), E->lmesh.nno, fp1);
  }

  fclose(fp1);

  return;
}


void output_visc(struct All_variables *E, int cycles)
{
  int i, j;
  char output_file[255];
  FILE *fp1;
  int lev = E->mesh.levmax;

  sprintf(output_file,"%s.visc.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");


  for(j=1;j<=E->sphere.caps_per_proc;j++) {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++)
      fprintf(fp1,"%.4e\n",E->VI[lev][j][i]);
  }

  fclose(fp1);

  return;
}

/* XXX DJB */
void output_divv(struct All_variables *E, int cycles) /* DJB */
{
  void p_to_nodes();
  int i, j, m;
  float *divv_nd[NCS];
  char output_file[255];
  FILE *fp1;

  for(m=1;m<=E->sphere.caps_per_proc;m++)
    divv_nd[m] = (float *) malloc ((E->lmesh.nno+1)*sizeof(float));

  /* for(m=1;m<=E->sphere.caps_per_proc;m++) {
    for(i=1;i<=E->lmesh.nno;i++) {
      divv_nd[m][i] = 0.0;
    }
  } */

  p_to_nodes(E,E->divv,divv_nd,E->mesh.levmax);

  sprintf(output_file,"%s.divv.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  fprintf(fp1,"%d %d %.5e\n",cycles,E->lmesh.nno,E->monitor.elapsed_time);

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++)
      fprintf(fp1,"%.6e\n",divv_nd[j][i]);
  }

  fclose(fp1);

  for(m=1;m<=E->sphere.caps_per_proc;m++)
    free(divv_nd[m]);

  return;
}


/* XXX DJB */
void output_sten_temp(struct All_variables *E, int cycles)
{
  int i, j;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.sten_temp.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  fprintf(fp1,"%d %d %.5e\n",cycles,E->lmesh.nno,E->monitor.elapsed_time);

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++) {
        fprintf(fp1,"%.6e\n",E->sphere.cap[j].slab_sten[i]);
    }
  }

  fclose(fp1);

  return;
}

/* XXX DJB */
void output_sten_velo(struct All_variables *E, int cycles)
{
  int i, j;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.sten_velo.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  fprintf(fp1,"%d %d %.5e\n",cycles,E->lmesh.nno,E->monitor.elapsed_time);

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++) {
        fprintf(fp1,"%d\n",E->sphere.cap[j].slab_sten2[i]);
    }
  }

  fclose(fp1);

  return;
}


void output_velo(struct All_variables *E, int cycles)
{
  int i, j;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.velo.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  fprintf(fp1,"%d %d %.5e\n",cycles,E->lmesh.nno,E->monitor.elapsed_time);

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++) {
        fprintf(fp1,"%.6e %.6e %.6e %.6e\n",E->sphere.cap[j].V[1][i],E->sphere.cap[j].V[2][i],E->sphere.cap[j].V[3][i],E->T[j][i]);
    }
  }

  fclose(fp1);

  return;
}


void output_surf_botm(struct All_variables *E, int cycles)
{
  int i, j, s;
  char output_file[255];
  FILE* fp2;
  float *topo;

  if((E->output.write_q_files == 0) || (cycles == 0) ||
     (cycles % E->output.write_q_files)!=0)
      heat_flux(E);
  /* else, the heat flux will have been computed already */

  if(E->control.use_cbf_topo){
    get_CBF_topo(E,E->slice.tpg,E->slice.tpgb);

  }else{
    get_STD_topo(E,E->slice.tpg,E->slice.tpgb,E->slice.divg,E->slice.vort,cycles);
  }

  if (E->output.surf && (E->parallel.me_loc[3]==E->parallel.nprocz-1)) {
    sprintf(output_file,"%s.surf.%d.%d", E->control.data_file,
            E->parallel.me, cycles);
    fp2 = output_open(output_file, "w");

    for(j=1;j<=E->sphere.caps_per_proc;j++)  {
        /* choose either STD topo or pseudo-free-surf topo */
        if(E->control.pseudo_free_surf)
            topo = E->slice.freesurf[j];
        else
            topo = E->slice.tpg[j];

        fprintf(fp2,"%3d %7d\n",j,E->lmesh.nsf);
        for(i=1;i<=E->lmesh.nsf;i++)   {
            s = i*E->lmesh.noz;
            fprintf(fp2,"%.4e %.4e %.4e %.4e\n",
		    topo[i],E->slice.shflux[j][i],E->sphere.cap[j].V[1][s],E->sphere.cap[j].V[2][s]);
        }
    }
    fclose(fp2);
  }


  if (E->output.botm && (E->parallel.me_loc[3]==0)) {
    sprintf(output_file,"%s.botm.%d.%d", E->control.data_file,
            E->parallel.me, cycles);
    fp2 = output_open(output_file, "w");

    for(j=1;j<=E->sphere.caps_per_proc;j++)  {
      fprintf(fp2,"%3d %7d\n",j,E->lmesh.nsf);
      for(i=1;i<=E->lmesh.nsf;i++)  {
        s = (i-1)*E->lmesh.noz + 1;
        fprintf(fp2,"%.4e %.4e %.4e %.4e\n",
		E->slice.tpgb[j][i],E->slice.bhflux[j][i],E->sphere.cap[j].V[1][s],E->sphere.cap[j].V[2][s]);
      }
    }
    fclose(fp2);
  }

  return;
}


void output_geoid(struct All_variables *E, int cycles)
{
    void compute_geoid();
    int ll, mm, p;
    char output_file[255];
    FILE *fp1;

    compute_geoid(E);

    if (E->parallel.me == (E->parallel.nprocz-1))  {
        sprintf(output_file, "%s.geoid.%d.%d", E->control.data_file,
                E->parallel.me, cycles);
        fp1 = output_open(output_file, "w");

        /* write headers */
        fprintf(fp1, "%d %d %.5e\n", cycles, E->output.llmax,
                E->monitor.elapsed_time);

        /* write sph harm coeff of geoid and topos */
        for (ll=0; ll<=E->output.llmax; ll++)
            for(mm=0; mm<=ll; mm++)  {
                p = E->sphere.hindex[ll][mm];
                fprintf(fp1,"%d %d %.4e %.4e %.4e %.4e %.4e %.4e\n",
                        ll, mm,
                        E->sphere.harm_geoid[0][p],
                        E->sphere.harm_geoid[1][p],
                        E->sphere.harm_geoid_from_tpgt[0][p],
                        E->sphere.harm_geoid_from_tpgt[1][p],
                        E->sphere.harm_geoid_from_bncy[0][p],
                        E->sphere.harm_geoid_from_bncy[1][p]);

                       
            }

        fclose(fp1);
    }
}



void output_stress(struct All_variables *E, int cycles)
{
  int m, node;
  char output_file[255];
  FILE *fp1;
 /* for stress computation */
  void allocate_STD_mem();
  void compute_nodal_stress();
  void free_STD_mem();
  float *SXX[NCS],*SYY[NCS],*SXY[NCS],*SXZ[NCS],*SZY[NCS],*SZZ[NCS];
  float *divv[NCS],*vorv[NCS];
  /*  */
  if(E->control.use_cbf_topo)	{/* for CBF topo, stress will not have been computed */
    allocate_STD_mem(E, SXX, SYY, SZZ, SXY, SXZ, SZY, divv, vorv);
    compute_nodal_stress(E, SXX, SYY, SZZ, SXY, SXZ, SZY, divv, vorv);
    free_STD_mem(E, SXX, SYY, SZZ, SXY, SXZ, SZY, divv, vorv);
  }
  sprintf(output_file,"%s.stress.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  fprintf(fp1,"%d %d %.5e\n",cycles,E->lmesh.nno,E->monitor.elapsed_time);

  for(m=1;m<=E->sphere.caps_per_proc;m++) {
    fprintf(fp1,"%3d %7d\n",m,E->lmesh.nno);
    /* those are sorted like stt spp srr stp str srp  */
    for (node=1;node<=E->lmesh.nno;node++)
      fprintf(fp1, "%.4e %.4e %.4e %.4e %.4e %.4e\n",
              E->gstress[m][(node-1)*6+1],
              E->gstress[m][(node-1)*6+2],
              E->gstress[m][(node-1)*6+3],
              E->gstress[m][(node-1)*6+4],
              E->gstress[m][(node-1)*6+5],
              E->gstress[m][(node-1)*6+6]);
  }
  fclose(fp1);
}


void output_horiz_avg(struct All_variables *E, int cycles)
{
  /* horizontal average output of temperature, composition and rms velocity*/
  void compute_horiz_avg();

  int j;
  char output_file[255];
  FILE *fp1;

  /* compute horizontal average here.... */
  compute_horiz_avg(E);

  /* only the first nprocz processors need to output */

  if (E->parallel.me<E->parallel.nprocz)  {
    sprintf(output_file,"%s.horiz_avg.%d.%d", E->control.data_file,
            E->parallel.me, cycles);
    fp1=fopen(output_file,"w");
    for(j=1;j<=E->lmesh.noz;j++)  {
        fprintf(fp1,"%.4e %.4e %.4e %.4e",E->sx[1][3][j],
		E->Have.T[j],E->Have.V[1][j],E->Have.V[2][j]);

        if (E->composition.on) {
            int n;
            for(n=0; n<E->composition.ncomp; n++)
                fprintf(fp1," %.4e", E->Have.C[n][j]);
        }
        fprintf(fp1,"\n");
    }
    fclose(fp1);
  }

  return;
}



void output_seismic(struct All_variables *E, int cycles)
{
    void get_prem(double, double*, double*, double*);
    void compute_seismic_model(const struct All_variables*, double*, double*, double*);

    char output_file[255];
    FILE* fp;
    int i;

    double *rho, *vp, *vs;
    const int len = E->lmesh.nno;

    rho = malloc(len * sizeof(double));
    vp = malloc(len * sizeof(double));
    vs = malloc(len * sizeof(double));
    if(rho==NULL || vp==NULL || vs==NULL) {
        fprintf(stderr, "Error while allocating memory\n");
        abort();
    }

    /* isotropic seismic velocity only */
    /* XXX: update for anisotropy in the future */
    compute_seismic_model(E, rho, vp, vs);

    sprintf(output_file,"%s.seismic.%d.%d", E->control.data_file, E->parallel.me, cycles);
    fp = output_open(output_file, "wb");

    fwrite(rho, sizeof(double), E->lmesh.nno, fp);
    fwrite(vp, sizeof(double), E->lmesh.nno, fp);
    fwrite(vs, sizeof(double), E->lmesh.nno, fp);

    fclose(fp);

#if 0
    /** debug **/
    sprintf(output_file,"%s.dv.%d.%d", E->control.data_file, E->parallel.me, cycles);
    fp = output_open(output_file, "w");
    fprintf(fp, "%d %d %.5e\n", cycles, E->lmesh.nno, E->monitor.elapsed_time);
    for(i=0; i<E->lmesh.nno; i++) {
        double vpr, vsr, rhor;
        int nz = (i % E->lmesh.noz) + 1;
        get_prem(E->sx[1][3][nz], &vpr, &vsr, &rhor);

        fprintf(fp, "%.4e %.4e %.4e\n",
                rho[i]/rhor - 1.0,
                vp[i]/vpr - 1.0,
                vs[i]/vsr - 1.0);

    }
    fclose(fp);
#endif

    free(rho);
    free(vp);
    free(vs);
    return;
}


void output_mat(struct All_variables *E)
{
  int m, el;
  char output_file[255];
  FILE* fp;

  sprintf(output_file,"%s.mat.%d", E->control.data_file,E->parallel.me);
  fp = output_open(output_file, "w");

  for (m=1;m<=E->sphere.caps_per_proc;m++)
    for(el=1;el<=E->lmesh.nel;el++)
      fprintf(fp,"%d %d %f\n", el,E->mat[m][el],E->VIP[m][el]);

  fclose(fp);

  return;
}



void output_pressure(struct All_variables *E, int cycles)
{
  int i, j;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.pressure.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  fprintf(fp1,"%d %d %.5e\n",cycles,E->lmesh.nno,E->monitor.elapsed_time);

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++)
      fprintf(fp1,"%.6e\n",E->NP[j][i]);
  }

  fclose(fp1);

  return;
}



void output_tracer(struct All_variables *E, int cycles)
{
  int i, j, n, ncolumns;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.tracer.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  ncolumns = 3 + E->trace.number_of_extra_quantities;

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
      fprintf(fp1,"%d %d %d %.5e\n", cycles, E->trace.ntracers[j],
              ncolumns, E->monitor.elapsed_time);

      for(n=1;n<=E->trace.ntracers[j];n++) {
          /* write basic quantities (coordinate) */
          fprintf(fp1,"%.12e %.12e %.12e",
                  E->trace.basicq[j][0][n],
                  E->trace.basicq[j][1][n],
                  E->trace.basicq[j][2][n]);

          /* write extra quantities */
          for (i=0; i<E->trace.number_of_extra_quantities; i++) {
              fprintf(fp1," %.12e", E->trace.extraq[j][i][n]);
          }
          fprintf(fp1, "\n");
      }

  }

  fclose(fp1);
  return;
}


void output_tracer_dens(struct All_variables *E, int cycles)
{
  int i, j;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.tracer_dens.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  fprintf(fp1,"%d %d %.5e\n",cycles,E->lmesh.nno,E->monitor.elapsed_time);

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++) {
        fprintf(fp1,"%.6e\n",E->trace.dens_node[j][i]);
    }
  }

  fclose(fp1);

  return;
}


void output_comp_nd(struct All_variables *E, int cycles)
{
    int i, j, k;
    char output_file[255];
    FILE *fp1;

    sprintf(output_file,"%s.comp_nd.%d.%d", E->control.data_file,
            E->parallel.me, cycles);
    fp1 = output_open(output_file, "w");

    for(j=1;j<=E->sphere.caps_per_proc;j++) {
        fprintf(fp1,"%3d %7d %.5e %d\n",
                j, E->lmesh.nel,
                E->monitor.elapsed_time, E->composition.ncomp);
        for(i=0;i<E->composition.ncomp;i++) {
            fprintf(fp1,"%.5e %.5e ",
                    E->composition.initial_bulk_composition[i],
                    E->composition.bulk_composition[i]);
        }
        fprintf(fp1,"\n");

        for(i=1;i<=E->lmesh.nno;i++) {
            for(k=0;k<E->composition.ncomp;k++) {
                fprintf(fp1,"%.6e ",E->composition.comp_node[j][k][i]);
            }
            fprintf(fp1,"\n");
        }

    }

    fclose(fp1);
    return;
}


void output_comp_el(struct All_variables *E, int cycles)
{
    int i, j, k;
    char output_file[255];
    FILE *fp1;

    sprintf(output_file,"%s.comp_el.%d.%d", E->control.data_file,
            E->parallel.me, cycles);
    fp1 = output_open(output_file, "w");

    for(j=1;j<=E->sphere.caps_per_proc;j++) {
        fprintf(fp1,"%3d %7d %.5e %d\n",
                j, E->lmesh.nel,
                E->monitor.elapsed_time, E->composition.ncomp);
        for(i=0;i<E->composition.ncomp;i++) {
            fprintf(fp1,"%.5e %.5e ",
                    E->composition.initial_bulk_composition[i],
                    E->composition.bulk_composition[i]);
        }
        fprintf(fp1,"\n");

        for(i=1;i<=E->lmesh.nel;i++) {
            for(k=0;k<E->composition.ncomp;k++) {
                fprintf(fp1,"%.6e ",
			E->composition.comp_el[j][k][i]);
            }
            fprintf(fp1,"\n");
        }
    }

    fclose(fp1);
    return;
}


void output_heating(struct All_variables *E, int cycles)
{
    int j, e;
    char output_file[255];
    FILE *fp1;

    sprintf(output_file,"%s.heating.%d.%d", E->control.data_file,
            E->parallel.me, cycles);
    fp1 = output_open(output_file, "w");

    fprintf(fp1,"%.5e\n",E->monitor.elapsed_time);

    for(j=1;j<=E->sphere.caps_per_proc;j++) {
        fprintf(fp1,"%3d %7d\n", j, E->lmesh.nel);
        for(e=1; e<=E->lmesh.nel; e++)
            fprintf(fp1, "%.4e %.4e %.4e\n", E->heating_adi[j][e],
                    E->heating_visc[j][e], E->heating_latent[j][e]);
    }
    fclose(fp1);

    return;
}


void output_heating_adi_nd(struct All_variables *E, int cycles) /* DJB */
{
  void p_to_nodes();

  p_to_nodes(E,E->heating_adi,E->heating_adi_nd,E->mesh.levmax);

  int i, j;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.heating_adi_nd.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  fprintf(fp1,"%d %d %.5e\n",cycles,E->lmesh.nno,E->monitor.elapsed_time);

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++)
      fprintf(fp1,"%.4e\n",E->heating_adi_nd[j][i]);
  }

  fclose(fp1);

  return;
}


void output_heating_latent_nd(struct All_variables *E, int cycles) /* DJB */
{
  void p_to_nodes();

  p_to_nodes(E,E->heating_latent,E->heating_latent_nd,E->mesh.levmax);

  int i, j;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.heating_latent_nd.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  fprintf(fp1,"%d %d %.5e\n",cycles,E->lmesh.nno,E->monitor.elapsed_time);

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++)
      fprintf(fp1,"%.4e\n",E->heating_latent_nd[j][i]);
  }

  fclose(fp1);

  return;
}


void output_heating_visc_nd(struct All_variables *E, int cycles) /* DJB */
{
  void p_to_nodes();

  p_to_nodes(E,E->heating_visc,E->heating_visc_nd,E->mesh.levmax);

  int i, j;
  char output_file[255];
  FILE *fp1;

  sprintf(output_file,"%s.heating_visc_nd.%d.%d", E->control.data_file,
          E->parallel.me, cycles);
  fp1 = output_open(output_file, "w");

  fprintf(fp1,"%d %d %.5e\n",cycles,E->lmesh.nno,E->monitor.elapsed_time);

  for(j=1;j<=E->sphere.caps_per_proc;j++) {
    fprintf(fp1,"%3d %7d\n",j,E->lmesh.nno);
    for(i=1;i<=E->lmesh.nno;i++)
      fprintf(fp1,"%.4e\n",E->heating_visc_nd[j][i]);
  }

  fclose(fp1);

  return;
}


/* used template from debug_sphere_expansion */
void output_temp_sph(struct All_variables *E, int cycles)
{
    /* expand temperature field (which should be a sph. harm. load) */
    int m, i, j, k, p, node;
    int ll, mm;
    float *TT[NCS], *sph_harm[2];
    char output_file[255];
    FILE *fp1;

    for(m=1;m<=E->sphere.caps_per_proc;m++)
        TT[m] = (float *) malloc ((E->lmesh.nsf+1)*sizeof(float));

    /* sin coeff */
    sph_harm[0] = (float*)malloc(E->sphere.hindice*sizeof(float));
    /* cos coeff */
    sph_harm[1] = (float*)malloc(E->sphere.hindice*sizeof(float));

    if(E->parallel.me < E->parallel.nprocz) {
        sprintf(output_file,"%s.temp_sph.%d.%d", E->control.data_file,
                E->parallel.me, cycles);
        fp1=fopen(output_file,"w");
    }

    for(k=1;k<=E->lmesh.noz;k++)  {
        for(m=1;m<=E->sphere.caps_per_proc;m++)
            for(i=1;i<=E->lmesh.noy;i++)
                for(j=1;j<=E->lmesh.nox;j++)  {
                    node= k + (j-1)*E->lmesh.noz + (i-1)*E->lmesh.nox*E->lmesh.noz;
                    p = j + (i-1)*E->lmesh.nox;
                    TT[m][p] = E->T[m][node];
                }

        /* expand TT into spherical harmonics */
        sphere_expansion(E, TT, sph_harm[0], sph_harm[1]);

        /* only the first nprocz CPU needs output */
        if(E->parallel.me < E->parallel.nprocz) {
            for (ll=0;ll<=E->output.llmax;ll++)
                for (mm=0; mm<=ll; mm++)   {
                    p = E->sphere.hindex[ll][mm];
                    fprintf(fp1, "T expanded layer=%d ll=%d mm=%d -- %12g %12g\n",
                            k+E->lmesh.nzs-1, ll, mm,
                            sph_harm[0][p], sph_harm[1][p]);
                }
        }
    }

    if(E->parallel.me < E->parallel.nprocz) {
        fclose(fp1);
    }

    return;
}


/* used template from debug_sphere_expansion */
void output_comp_sph(struct All_variables *E, int cycles)
{
    /* expand composition field (which should be a sph. harm. load) */
    int m, i, j, k, p, t, node;
    int ll, mm;
    float *CC[NCS], *sph_harm[2];
    char output_file[255];
    FILE *fp1;

    for(m=1;m<=E->sphere.caps_per_proc;m++)
        CC[m] = (float *) malloc ((E->lmesh.nsf+1)*sizeof(float));

    /* sin coeff */
    sph_harm[0] = (float*)malloc(E->sphere.hindice*sizeof(float));
    /* cos coeff */
    sph_harm[1] = (float*)malloc(E->sphere.hindice*sizeof(float));

    if(E->parallel.me < E->parallel.nprocz) {
        sprintf(output_file,"%s.comp_sph.%d.%d", E->control.data_file,
                E->parallel.me, cycles);
        fp1=fopen(output_file,"w");
    }

    for(k=1;k<=E->lmesh.noz;k++)  {
        for(m=1;m<=E->sphere.caps_per_proc;m++)
            for(i=1;i<=E->lmesh.noy;i++)
                for(j=1;j<=E->lmesh.nox;j++)  {
                    node= k + (j-1)*E->lmesh.noz + (i-1)*E->lmesh.nox*E->lmesh.noz;
                    p = j + (i-1)*E->lmesh.nox;
                    for(t=0;t<E->composition.ncomp;t++) {
                        CC[m][p] = E->composition.comp_node[m][t][node];
                    }
                }

        /* expand TT into spherical harmonics */
        sphere_expansion(E, CC, sph_harm[0], sph_harm[1]);

        /* only the first nprocz CPU needs output */
        if(E->parallel.me < E->parallel.nprocz) {
            for (ll=0;ll<=E->output.llmax;ll++)
                for (mm=0; mm<=ll; mm++)   {
                    p = E->sphere.hindex[ll][mm];
                    fprintf(fp1, "C expanded layer=%d ll=%d mm=%d -- %12g %12g\n",
                            k+E->lmesh.nzs-1, ll, mm,
                            sph_harm[0][p], sph_harm[1][p]);
                }
        }
    }

    if(E->parallel.me < E->parallel.nprocz) {
        fclose(fp1);
    }

    return;
}


void output_time(struct All_variables *E, int cycles)
{
  double CPU_time0();

  double current_time = CPU_time0();

  if (E->parallel.me == 0) {
    fprintf(E->fptime,"%d %.4e %.4e %.4e %.4e\n",
            cycles,
            E->monitor.elapsed_time,
            E->advection.timestep,
            current_time - E->monitor.cpu_time_at_start,
            current_time - E->monitor.cpu_time_at_last_cycle);

    fflush(E->fptime);
  }

  output_shell(E, cycles);

  E->monitor.cpu_time_at_last_cycle = current_time;

/*Exit running if the current age is more than -1 Ma*/
  if ((E->control.ExitAtPresent == 1)&&(E->monitor.elapsed_time*E->data.scalet - E->control.start_age >=1)){
//  if (E->monitor.elapsed_time*E->data.scalet - E->control.start_age >=1){
	fprintf(E->fp,"The age now is : %f\tThe start age is: %f\n",E->monitor.elapsed_time*E->data.scalet,E->control.start_age);
	citcom_finalize(E, 0);
  }
  return;
}
