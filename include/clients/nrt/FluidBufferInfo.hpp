/*
Part of the Fluid Corpus Manipulation Project (http://www.flucoma.org/)
Copyright 2017-2019 University of Huddersfield.
Licensed under the BSD-3 License.
See license.md file in the project root for full license information.
This project has received funding from the European Research Council (ERC)
under the European Union’s Horizon 2020 research and innovation programme
(grant agreement No 725899).
*/

#pragma once

#include "PDBufferAdaptor.hpp"
#include <memory> 

namespace fluid {
namespace client {
static t_class* fluidbufinfo_class;

using Buffer = std::unique_ptr<PDBufferAdaptor>;

typedef struct _fluidbufinfo
{
  t_object x_obj;
  Buffer   source;
} t_fluidbufinfo;

void fluidbufinfo_bang(t_fluidbufinfo* x)
{
  if (x->source)
  {
    auto buf = PDBufferAdaptor::ReadAccess(x->source.get());

    if (!buf.exists())
    {
      pd_error(x, "No array %s", x->source->name()->s_name);
      return;
    }

    t_atom data[2];
    SETFLOAT(&data[0], buf.numFrames());
    SETFLOAT(&data[1], buf.numChans());

    outlet_list(x->x_obj.ob_outlet, nullptr, 2, data);
  }
}

void fluidbufinfo_source(t_fluidbufinfo* x, t_symbol* src)
{
  if (src != gensym(""))
  {
    x->source.reset(new PDBufferAdaptor(src, sys_getsr()));
  }
}

void fluidbufinfo_buffer(t_fluidbufinfo* x, t_symbol* src)
{
  fluidbufinfo_source(x, src);
  fluidbufinfo_bang(x);
}

void* fluidbufinfo_new(t_symbol*, int ac, t_atom* argv)
{
  t_fluidbufinfo* x = (t_fluidbufinfo*) pd_new(fluidbufinfo_class);
  
  outlet_new((t_object*)x, gensym("list"));
  
  if (ac == 2)
  {
    t_symbol* flag = atom_getsymbol(argv);
    if(!strcmp(flag->s_name,"-source"))
    {
      t_symbol* source = atom_getsymbol(argv + 1);
      fluidbufinfo_source(x, source);
    }
  }
  else
  {
    if(ac) pd_error(x,"Unexpected number of arguments. Got %d, expect 0 or 2 (-source <array>)",ac);
  }
  
  return x;
}

void fluidbufinfo_setup(void)
{
  fluidbufinfo_class = class_new(gensym("fluid.bufinfo"),  (t_newmethod)fluidbufinfo_new, 0, sizeof(t_fluidbufinfo), CLASS_DEFAULT, A_GIMME, 0);

  class_addmethod(fluidbufinfo_class, (t_method) fluidbufinfo_bang,gensym("bang"),A_NULL);
  class_addmethod(fluidbufinfo_class, (t_method) fluidbufinfo_source,
                  gensym("source"), A_DEFSYM, 0);
  class_addmethod(fluidbufinfo_class, (t_method) fluidbufinfo_buffer,
                  gensym("buffer"), A_DEFSYM, 0);
}
}
}
