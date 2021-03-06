
/*
 *
 * Copyright (C) 2012 Pierre Guillot, Universite Paris 8
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Library General Public License as published 
 * by the Free Software Foundation; either version 2 of the License.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public 
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License 
 * along with this library; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#define CICM_MACOSX

#include "../revComponents/cicmFir.h"

extern "C" 
{
	#include "ext.h"
	#include "ext_obex.h"
	#include "z_dsp.h"
	#include "buffer.h"
	#include "ext_globalsymbol.h"
}


typedef struct _Fir
{
	t_pxobject		f_ob;	
	FirFilter		*f_Fir;
	t_symbol		*f_name;
	t_buffer		*f_buffer;
	long			f_channel;
	long			f_size;
}
t_Fir ;

void *Fir_new(t_symbol *s, long argc, t_atom *argv);
void Fir_free(t_Fir *x);
void Fir_assist(t_Fir *x, void *b, long m, long a, char *s);

t_max_err buffer_set(t_Fir *x, t_object *attr, long argc, t_atom *argv);
t_max_err channel_set(t_Fir *x, t_object *attr, long argc, t_atom *argv);
t_max_err Fir_notify(t_Fir *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void buffer_setup(t_Fir *x);

void Fir_dsp(t_Fir *x, t_signal **sp, short *count);
t_int *Fir_perform(t_int *w);

void Fir_dsp64(t_Fir *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void Fir_perform64(t_Fir *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

void *Fir_class;


int main(void)
{	
	t_class *c;
	
	c = class_new("rev.fir~", (method)Fir_new, (method)Fir_free, (long)sizeof(t_Fir ), 0L, A_GIMME, 0);
	
	class_addmethod(c, (method)Fir_dsp,			"dsp",			A_CANT, 0);
	class_addmethod(c, (method)Fir_dsp64,		"dsp64",		A_CANT, 0);
	class_addmethod(c, (method)Fir_assist,		"assist",		A_CANT, 0);
	class_addmethod(c, (method)Fir_notify,		"notify",		A_CANT, 0);

	CLASS_ATTR_SYM				(c, "buffer", 0, t_Fir, f_name);
	CLASS_ATTR_CATEGORY			(c, "buffer", 0, "Behavior");
	CLASS_ATTR_LABEL			(c, "buffer", 0, "buffer~ Object Name");
	CLASS_ATTR_ORDER			(c, "buffer", 0, "1");
	CLASS_ATTR_ACCESSORS		(c, "buffer", NULL, buffer_set);
	CLASS_ATTR_DEFAULT			(c, "buffer", 0, "");
	CLASS_ATTR_SAVE				(c, "buffer", 1);

	CLASS_ATTR_LONG				(c, "channel", 0, t_Fir, f_channel);
	CLASS_ATTR_CATEGORY			(c, "channel", 0, "Behavior");
	CLASS_ATTR_LABEL			(c, "channel", 0, "Channels");
	CLASS_ATTR_ORDER			(c, "channel", 0, "2");
	CLASS_ATTR_ACCESSORS		(c, "channel", NULL, channel_set);
	CLASS_ATTR_DEFAULT			(c, "channel", 0, "0");
	CLASS_ATTR_SAVE				(c, "channel", 1);
	
	class_dspinit(c);				
	class_register(CLASS_BOX, c);	
	Fir_class = c;
	
	post("rev.fir~ by Pierre Guillot");
	post("Copyright (C) 2012, CICM / Universite Paris 8");
	
	return 0;
}

void *Fir_new(t_symbol *s, long argc, t_atom *argv)
{
	t_Fir *x = NULL;
	
	if (x = (t_Fir *)object_alloc((t_class*)Fir_class)) 
	{	
		x->f_size = 128;
		if(atom_gettype(argv) == A_LONG)
			x->f_size = atom_getlong(argv);
		if(x->f_size <= 0)
			x->f_size = 2;
		
		x->f_Fir = new FirFilter(x->f_size);
		
		dsp_setup((t_pxobject *)x, 1);	
		outlet_new(x, "signal");
		x->f_buffer = NULL;
		

		attr_args_process(x, argc, argv);
	}
	return (x);
}

void Fir_dsp(t_Fir *x, t_signal **sp, short *count)
{
	dsp_add(Fir_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

t_int *Fir_perform(t_int *w)
{
	t_Fir*	x				= (t_Fir *)(w[1]);
	float**	ins				= (float **)(w+2);
	float**	outs			= (float **)(w+3);
	long	sampleframes	= (long)(w[4]);
	
	for(int i = 0; i < sampleframes; i++)
		outs[0][i] = x->f_Fir->process(ins[0][i]);
	
	return (w + 5);
}

void Fir_dsp64(t_Fir *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), x, Fir_perform64, 0, NULL);	
}

void Fir_perform64(t_Fir *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	for(int i = 0; i < sampleframes; i++)
		outs[0][i] = x->f_Fir->process(ins[0][i]);
}

void Fir_assist(t_Fir *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) 
	{
		sprintf(s,"(Signal) Input signal");
	} 
	else 
	{
		sprintf(s,"(Signal) Filtered signal"); 			
	}
}

void Fir_free(t_Fir *x)
{
	dsp_free((t_pxobject *)x);
	delete x->f_Fir;
}

t_max_err Fir_notify(t_Fir *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	if(msg == gensym("globalsymbol_binding"))
	{
		x->f_buffer = (t_buffer*)x->f_name->s_thing;
	}
	else if (msg == gensym("globalsymbol_unbinding"))
	{
		x->f_buffer = NULL;
	}
	else if (msg == gensym("buffer_modified"))
	{
		buffer_setup(x);
	}
	return object_notify(x, s, data);
}

t_max_err buffer_set(t_Fir *x, t_object *attr, long argc, t_atom *argv)
{
	t_buffer *b;
	t_symbol *name;
	t_atom channel[1];
	if (atom_gettype(argv) == A_SYM)
	{
		name = atom_getsym(argv);
		if(x->f_name != name)
		{
			if ((b = (t_buffer *)(name->s_thing)) && ob_sym(b) == gensym("buffer~")) 
			{
				if(x->f_name && x->f_buffer != NULL)
				{
					globalsymbol_dereference((t_object*)x, x->f_name->s_name, "buffer~");
				}
				x->f_name = name;
				x->f_buffer = (t_buffer*)globalsymbol_reference((t_object*)x, x->f_name->s_name, "buffer~");
				atom_setlong(channel, x->f_channel);
				channel_set(x, attr, 1, channel);
			}
			else
			{
				x->f_buffer = NULL;
				x->f_name = gensym("");
			}
		}
	}
	return 0;
}

t_max_err channel_set(t_Fir *x, t_object *attr, long argc, t_atom *argv)
{
	if(atom_gettype(argv) == A_LONG)
	{
		if(x->f_buffer == NULL)
		{
			if(atom_getlong(argv) > 0)
				x->f_channel = atom_getlong(argv);
		}
		else
		{
			if(atom_getlong(argv) > 0 && x->f_channel <= x->f_buffer->b_nchans)
				x->f_channel = atom_getlong(argv);
			else if(atom_getlong(argv) <= 0)
				x->f_channel = 1;
			else
				x->f_channel = x->f_buffer->b_nchans;
			buffer_setup(x);
		}
	}
	return 0;
}


void buffer_setup(t_Fir *x)
{
	int i;
	double* datas;
	if(x->f_buffer != NULL)
	{
		ATOMIC_INCREMENT(&x->f_buffer->b_inuse);
		if (!x->f_buffer->b_valid) 
		{
			ATOMIC_DECREMENT(&x->f_buffer->b_inuse);
		}
		else
		{
			datas = new double[x->f_size];
			for(i = 0; i < x->f_size; i++)
			{
				if(i < x->f_buffer->b_frames)
					datas[i] = x->f_buffer->b_samples[i * x->f_buffer->b_nchans + (x->f_channel - 1)];
				else
					datas[i] = 0.;
			}
			ATOMIC_DECREMENT(&x->f_buffer->b_inuse);
			x->f_Fir->setImpulseResponse(datas);
			free(datas);
		}
	}
}
