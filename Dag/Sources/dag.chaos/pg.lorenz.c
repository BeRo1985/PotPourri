#include "dag.chaos.h"
#include "pg.lorenz.h"

void lorenz_init(t_chaos *x, t_symbol *s, short argc, t_atom *argv)
{
	int i;

	// Initialisation des valeurs de bases //
	x->f_parameter[0] = 0.1f;
	x->f_parameter[1] = 0.1f;
	x->f_parameter[2] = 0.1f;
	x->f_parameter[3] = 10.0f;
	x->f_parameter[4] = 2.66f;
	x->f_parameter[5] = 28.f;
	x->f_parameter[6] = 0.01f;

	// Initialisation des valeurs par l utilisateur // 
	for(i = 0; i < 7; i++)
	{
		if(i < argc)
		{
			if( argv[i].a_type == A_LONG )
			{
				x->f_parameter[i] = (float)argv[i].a_w.w_long;
			}
			else if ( argv[i].a_type == A_FLOAT )
			{
				x->f_parameter[i] = argv[i].a_w.w_float;
			}
		}
		x->f_parameterInit[i] = x->f_parameter[i];
	}
}	

void lorenz_reset(t_chaos *x)
{
	int i;
	for(i = 0; i < 7; i++)
	{
		x->f_parameter[i] = x->f_parameterInit[i];
	}
}

void lorenz_bang(t_chaos *x)
{
	float posiX, posiY, posiZ;
	
	outlet_float(x->out, x->f_parameter[0]);
	outlet_float(x->out2, x->f_parameter[1]);
	outlet_float(x->out3, x->f_parameter[2]);
	
	posiX = x->f_parameter[0];
	posiY = x->f_parameter[1];
	posiZ = x->f_parameter[2];
	
	// x = a * (y-1 - x-1) * dt
	x->f_parameter[0] += ( x->f_parameter[3] * (posiY - posiX)) *  x->f_parameter[6];
	// y = ( x-1 * (r - z-1) - y-1 ) *dt
	x->f_parameter[1] += ((posiX * (x->f_parameter[5] - posiZ ) )- posiY) * x->f_parameter[6];
	// z = (x-1 * y-1 - b * z-1)* dt
	x->f_parameter[2] += ((posiX * posiY) -  (x->f_parameter[4] * posiZ) ) * x->f_parameter[6];

}

void lorenz_list(t_chaos *x, t_symbol *s, short ac, t_atom *av)
{
	int i;
	
	if(ac > 7)
	{
		ac = 7;
	}
	// Initialisation des parametres //
	for (i = 0; i < ac; i++, av++) 
	{
		if (atom_gettype(av)==A_LONG)
		{
			x->f_parameter[i] = (float)atom_getlong(av);
		}
		else if (atom_gettype(av)==A_FLOAT)
		{
			x->f_parameter[i] = (float)atom_getfloat(av);
		}
	}
	lorenz_bang(x);
}

void lorenz_set(t_chaos *x, t_symbol *s, short ac, t_atom *av)
{
	int i;
	
	if(ac > 7)
	{
		ac = 7;
	}
	// Initialisation des parametres //
	for (i = 0; i < ac; i++, av++) 
	{
		if (atom_gettype(av)==A_LONG)
		{
			x->f_parameter[i] = (float)atom_getlong(av);
		}
		else if (atom_gettype(av)==A_FLOAT)
		{
			x->f_parameter[i] = (float)atom_getfloat(av);
		}
		x->f_parameterInit[i] = x->f_parameter[i];
	}
	lorenz_bang(x);
}

void lorenz_print(t_chaos *x)
{
	t_atom list[8];
	int i;

	SETSYM(list, gensym("Lorenz"));
	for(i = 1; i < 8; i++)
	{
		SETFLOAT(list+i, x->f_parameterInit[i-1]);
	}
	outlet_list(x->out4 , 0L, 8, list);
}

void lorenz_record(t_chaos *x)
{
	int i;

	for(i = 0; i < 7; i++)
	{
		x->f_parameterInit[i] = x->f_parameter[i];
	}
}

void lorenz_free(t_chaos *x)
{
	object_free(x->f_proxy);
}

void lorenz_assist(t_chaos *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
	{
		sprintf(s,"Parameters x, y, z, a, b, c, d (list) or computation (bang)");
	}
	else 
	{
		switch (a) 
		{	
			case 0:
				sprintf(s,"Coordinates x (float)");
				break;
			case 1:
				sprintf(s,"Coordinates y (float)");
				break;
			case 2:
				sprintf(s,"Coordinates z (float)");
				break;
			case 3:
				sprintf(s,"Initial parameters (list)");
				break;
		}
		
	}
}
