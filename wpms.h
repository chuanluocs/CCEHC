#ifndef _PMS_H_
#define _PMS_H_

#include "basis_wpms.h"

inline void unsat(int clause)
{
    if(org_clause_weight[clause]==top_clause_weight) 
    {
        index_in_hardunsat_stack[clause] = hardunsat_stack_fill_pointer;
        push(clause,hardunsat_stack);
        
        hard_unsat_nb++;
    }
    else {
        index_in_softunsat_stack[clause] = softunsat_stack_fill_pointer;
        push(clause,softunsat_stack);
        
        soft_unsat_weight += org_clause_weight[clause];
    }   
}


inline void sat(int clause)
{
	int index,last_unsat_clause;

	//since the clause is satisfied, its position can be reused to store the last_unsat_clause
    if (org_clause_weight[clause]==top_clause_weight) 
    {
        last_unsat_clause = pop(hardunsat_stack);

        index = index_in_hardunsat_stack[clause];
        hardunsat_stack[index] = last_unsat_clause;
        index_in_hardunsat_stack[last_unsat_clause] = index;
        
        hard_unsat_nb--;
    }
    else {
        last_unsat_clause = pop(softunsat_stack);

        index = index_in_softunsat_stack[clause];
        softunsat_stack[index] = last_unsat_clause;
        index_in_softunsat_stack[last_unsat_clause] = index;
        
        soft_unsat_weight -= org_clause_weight[clause];
    }
}



//initiation of the algorithm
void init()
{
	int 		v,c;
	int			i,j;

    hard_unsat_nb=0;
    soft_unsat_weight=0;
    
    //Initialize edge weights
	for (c = 0; c<num_clauses; c++)
    {
		if (org_clause_weight[c]==top_clause_weight) 
            clause_weight[c] = 1;
        else 
            clause_weight[c] = org_clause_weight[c];
    }

	//init solution
	for (v = 1; v <= num_vars; v++) {

		if(rand()%2==1) cur_soln[v] = 1;
		else cur_soln[v] = 0;

		time_stamp[v] = 0;
		//unsat_app_count[v] = 0;
		
		//hcscc
		hard_cscc[v] = 1;
		
		conf_change[v] = 1;
	}
	
	//init stacks
	hardunsat_stack_fill_pointer = 0;
    softunsat_stack_fill_pointer = 0;
    
    large_weight_clauses_count = 0;
    
	//unsatvar_stack_fill_pointer = 0;
    
	/* figure out sat_count, and init unsat_stack */
	for (c=0; c<num_clauses; ++c) 
	{
		sat_count[c] = 0;
		
		for(j=0; j<clause_lit_count[c]; ++j)
		{
			if (cur_soln[clause_lit[c][j].var_num] == clause_lit[c][j].sense)
			{
				sat_count[c]++;
				sat_var[c] = clause_lit[c][j].var_num;	
			}
		}

		if (sat_count[c] == 0) 
        {
            unsat(c);
        }
	}

	/*figure out score*/
	for (v=1; v<=num_vars; v++) 
	{
		hscore[v]=sscore[v]=0;
		for(i=0; i<var_lit_count[v]; ++i)
		{
			c = var_lit[v][i].clause_num;
            if (org_clause_weight[c]==top_clause_weight)
            {
                if (sat_count[c]==0) hscore[v]+=clause_weight[c];
                else if (sat_count[c]==1 && var_lit[v][i].sense==cur_soln[v]) hscore[v]-=clause_weight[c];
            }
            else
            {
                if (sat_count[c]==0) sscore[v]+=clause_weight[c];
                else if (sat_count[c]==1 && var_lit[v][i].sense==cur_soln[v]) sscore[v]-=clause_weight[c];
            }
		}
	}
		
	//init goodvars stack
	goodvar_stack_fill_pointer = 0;
	for (v=1; v<=num_vars; v++) 
	{
		if(hscore[v]>0 || (hscore[v]==0 && sscore[v]>0) )
		{
			already_in_goodvar_stack[v] = 1;
			push(v,goodvar_stack);
			
		}
		else
			already_in_goodvar_stack[v] = 0;
	}
	
	//setting for the virtual var 0
	//time_stamp[0]=0;

}


//flip a var, and do the neccessary updating
void flip(int flipvar)
{
	int i,v,c;
	int index;
	lit* clause_c;

	int org_flipvar_hscore = hscore[flipvar];
    int org_flipvar_sscore = sscore[flipvar];
	cur_soln[flipvar] = 1 - cur_soln[flipvar];
	
	//update related clauses and neighbor vars
    //for(lit *q = var_lit[flipvar]; (c=q->clause_num)>=0; q++)
	//{
    for(i=0; i<var_lit_count[flipvar]; ++i)
	{
		c = var_lit[flipvar][i].clause_num;
		clause_c = clause_lit[c];
        
        if (org_clause_weight[c]==top_clause_weight)
        {
            if(cur_soln[flipvar] == var_lit[flipvar][i].sense)
            {
                ++sat_count[c];
                
                if (sat_count[c] == 2) //sat_count from 1 to 2
                    hscore[sat_var[c]] += clause_weight[c];
                else if (sat_count[c] == 1) // sat_count from 0 to 1
                {
                    sat_var[c] = flipvar;//record the only true lit's var
                    
                    for(lit* p=clause_c; (v=p->var_num)!=0; p++) 
                    {
                    	hscore[v] -= clause_weight[c];
                    	
                    	//hscc
                    	hard_cscc[v] = 1;
                    }
                    
                    sat(c);
                }
            }
            else // cur_soln[flipvar] != cur_lit.sense
            {
                --sat_count[c];
                if (sat_count[c] == 1) //sat_count from 2 to 1
                {
                    for(lit* p=clause_c; (v=p->var_num)!=0; p++) 
                    {
                        if(p->sense == cur_soln[v] )
                        {
                            hscore[v] -= clause_weight[c];
                            sat_var[c] = v;
                            break;
                        }
                    }

                }
                else if (sat_count[c] == 0) //sat_count from 1 to 0
                {
                    for(lit* p=clause_c; (v=p->var_num)!=0; p++) 
                    {
                    	hscore[v] += clause_weight[c];
                    	
                    	//hscc
                    	hard_cscc[v] = 1;
                    }
                    
                    unsat(c); 
                    
                }//end else if
                
            }//end else
		
        }
        else
        {
            if(cur_soln[flipvar] == var_lit[flipvar][i].sense)
            {
                ++sat_count[c];
                
                if (sat_count[c] == 2) //sat_count from 1 to 2
                    sscore[sat_var[c]] += clause_weight[c];
                else if (sat_count[c] == 1) // sat_count from 0 to 1
                {
                    sat_var[c] = flipvar;//record the only true lit's var
                    
                    for(lit* p=clause_c; (v=p->var_num)!=0; p++) sscore[v] -= clause_weight[c];
                    
                    sat(c);
                }
            }
            else // cur_soln[flipvar] != cur_lit.sense
            {
                --sat_count[c];
                if (sat_count[c] == 1) //sat_count from 2 to 1
                {
                    for(lit* p=clause_c; (v=p->var_num)!=0; p++) 
                    {
                        if(p->sense == cur_soln[v] )
                        {
                            sscore[v] -= clause_weight[c];
                            sat_var[c] = v;
                            break;
                        }
                    }
                }
                else if (sat_count[c] == 0) //sat_count from 1 to 0
                {
                    for(lit* p=clause_c; (v=p->var_num)!=0; p++) sscore[v] += clause_weight[c];
                    unsat(c); 
                }//end else if
                
            }//end else
        }
	}

	//hcscc
	hard_cscc[flipvar] = 0;

	//update information of flipvar
	hscore[flipvar] = -org_flipvar_hscore;
    sscore[flipvar] = -org_flipvar_sscore;

	//remove the vars no longer goodvar in goodvar stack 
	for(index=goodvar_stack_fill_pointer-1; index>=0; index--)
	{
		v = goodvar_stack[index];
		if(hscore[v]<0 || (hscore[v]<=0 && sscore[v]<=0) )
		{
			goodvar_stack[index] = pop(goodvar_stack);
			already_in_goodvar_stack[v] = 0;
		}	
	}

	//add goodvar
	for(i=0; i<var_neighbor_count[flipvar]; ++i)
	{
		v = var_neighbor[flipvar][i];
		
		conf_change[v] = 1;
		
		if ( hscore[v]>0 || (hscore[v]==0 && sscore[v]>0) ) 
		{
            if(already_in_goodvar_stack[v] ==0)
            {
                push(v,goodvar_stack);
                already_in_goodvar_stack[v] = 1;
            }
		}
	}
	
	conf_change[flipvar] = 0;
}


#endif
