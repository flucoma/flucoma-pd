// Part of the Fluid Corpus Manipulation Project (http://www.flucoma.org/)
// Copyright University of Huddersfield.
// Licensed under the BSD-3 License.
// See license.md file in the project root for full license information.
// This project has received funding from the European Research Council (ERC)
// under the European Union’s Horizon 2020 research and innovation programme
// (grant agreement No 725899).

// Adapted from fluid.waveform and some of the sourcecode of ELSE https://github.com/porres/pd-else/

#define MIN(A,B) ((A)<(B) ? (A) : (B))
#define MAX(A,B) ((A)>(B) ? (A) : (B))
#define CLIP(A,B,C) (MAX(B,(MIN(A,C))))

#include "m_pd.h"
#include "g_canvas.h"

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

static t_class *fplot_class, *edit_proxy_class;
t_widgetbehavior fplot_widgetbehavior;

typedef struct _edit_proxy{
    t_object    p_obj;
    t_symbol   *p_sym;
    t_clock    *p_clock;
    struct      _fplot *p_cnv;
}t_edit_proxy;

typedef struct _fplot{
    t_object       x_obj;
    t_glist       *x_glist;
    t_edit_proxy  *x_proxy;
    t_symbol       *x_bindname;
    int            x_zoom;
    int            x_width;
    int            x_height;
    int            x_x;
    int            x_y;
    int            x_snd_set;
    int            x_rcv_set;
    int            x_edit;
    int            x_init;
    int            x_sel;
    int            x_outline;
    int            x_s_flag;
    int            x_r_flag;
    int            x_flag;
    int            x_latch;
    t_symbol      *x_receive;
    t_symbol      *x_rcv_raw;
    t_symbol      *x_send;
    t_symbol      *x_snd_raw;
    t_outlet      *x_outlet;
}t_fplot;

// ------------------------ draw inlet --------------------------------------------------------------------
static void fplot_draw_io_let(t_fplot *x){
    t_canvas *cv = glist_getcanvas(x->x_glist);
    int xpos = text_xpix(&x->x_obj, x->x_glist), ypos = text_ypix(&x->x_obj, x->x_glist);
    sys_vgui(".x%lx.c delete %lx_in\n", cv, x);
    if(x->x_edit && x->x_receive == &s_)
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags %lx_in\n",
                 cv, xpos, ypos, xpos+(IOWIDTH*x->x_zoom), ypos+(IHEIGHT*x->x_zoom), x);
    sys_vgui(".x%lx.c delete %lx_out\n", cv, x);
    if(x->x_edit && x->x_send == &s_)
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill black -tags %lx_out\n",
                 cv, xpos, ypos+x->x_height, xpos+(IOWIDTH*x->x_zoom), ypos+x->x_height-IHEIGHT*x->x_zoom, x);
}

static void fplot_get_snd_rcv(t_fplot* x){
    t_binbuf *bb = x->x_obj.te_binbuf;
    int n_args = binbuf_getnatom(bb), i = 0; // number of arguments
    char buf[128];
    if(!x->x_snd_set){ // no send set, search arguments/flags
        if(n_args > 0){ // we have arguments, let's search them
            if(x->x_flag){ // arguments are flags actually
                if(x->x_s_flag){ // we got a search flag, let's get it
                    for(i = 0;  i < n_args; i++){
                        atom_string(binbuf_getvec(bb) + i, buf, 80);
                        if(gensym(buf) == gensym("-send")){
                            i++;
                            atom_string(binbuf_getvec(bb) + i, buf, 80);
                            x->x_snd_raw = gensym(buf);
                            break;
                        }
                    }
                }
            }
            else{ // we got no flags, let's search for argument
                int arg_n = 4; // receive argument number
                if(n_args >= arg_n){ // we have it, get it
                    atom_string(binbuf_getvec(bb) + arg_n, buf, 80);
                    x->x_snd_raw = gensym(buf);
                }
            }
        }
    }
    if(x->x_snd_raw == &s_)
        x->x_snd_raw = gensym("empty");
    if(!x->x_rcv_set){ // no receive set, search arguments
        if(n_args > 0){ // we have arguments, let's search them
            if(x->x_flag){ // arguments are flags actually
                if(x->x_r_flag){ // we got a receive flag, let's get it
                    for(i = 0;  i < n_args; i++){
                        atom_string(binbuf_getvec(bb) + i, buf, 80);
                        if(gensym(buf) == gensym("-receive")){
                            i++;
                            atom_string(binbuf_getvec(bb) + i, buf, 80);
                            x->x_rcv_raw = gensym(buf);
                            break;
                        }
                    }
                }
            }
            else{ // we got no flags, let's search for argument
                int arg_n = 5; // receive argument number
                if(n_args >= arg_n){ // we have it, get it
                    atom_string(binbuf_getvec(bb) + arg_n, buf, 80);
                    x->x_rcv_raw = gensym(buf);
                }
            }
        }
    }
    if(x->x_rcv_raw == &s_)
        x->x_rcv_raw = gensym("empty");
}

// ------------------------ widgetbehaviour-------------------------------------------------------------------
static void fplot_motion(t_fplot *x, t_floatarg dx, t_floatarg dy){
    x->x_x += (int)(dx/x->x_zoom);
    x->x_y -= (int)(dy/x->x_zoom);
    x->x_x = CLIP(x->x_x,0,x->x_width);
    x->x_y = CLIP(x->x_y,0,x->x_height);
    t_atom at[2];
    SETFLOAT(at, x->x_x);
    SETFLOAT(at+1, x->x_y);
    outlet_anything(x->x_obj.ob_outlet, &s_list, 2, at);
    if(x->x_send != &s_ && x->x_send->s_thing)
        pd_list(x->x_send->s_thing, &s_list, 2, at);
}

static int fplot_click(t_fplot *x, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit){
    shift = alt = dbl = 0;
    t_atom at[2];
    int xpos = text_xpix(&x->x_obj, glist), ypos = text_ypix(&x->x_obj, glist);
    x->x_x = CLIP((xpix - xpos) / x->x_zoom,0,x->x_width);
    x->x_y = CLIP(x->x_height - (ypix - ypos) / x->x_zoom,0,x->x_height);
    if(doit){//if mousedown
        SETFLOAT(at, (float)x->x_x);
        SETFLOAT(at+1, (float)x->x_y);
        outlet_anything(x->x_obj.ob_outlet, &s_list, 2, at);
        if(x->x_send != &s_ && x->x_send->s_thing)
            pd_list(x->x_send->s_thing, &s_list, 2, at);
        // start streaming the coordinates
        glist_grab(x->x_glist, &x->x_obj.te_g, (t_glistmotionfn)fplot_motion, 0, xpix, ypix);
    }
    // else{// to send the coordinates on mouse up if we want to eventually
    //     SETFLOAT(at, (float)x->x_x);
    //     SETFLOAT(at+1, (float)x->x_y);
    //     outlet_anything(x->x_obj.ob_outlet, &s_list, 2, at);
    // }
    return(1);
}

static void fplot_mouserelease(t_fplot* x){
    // could do something here if needs be
    //    if(x->x_latch){
    //        outlet_float(x->x_outlet, -1);
    //        if(x->x_send != &s_ && x->x_send->s_thing)
    //            pd_float(x->x_send->s_thing, -1);
    //    }
}

static void fplot_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2){
    t_fplot* x = (t_fplot*)z;
    int xpos = *xp1 = text_xpix(&x->x_obj, glist), ypos = *yp1 = text_ypix(&x->x_obj, glist);
    *xp2 = xpos + x->x_width*x->x_zoom, *yp2 = ypos + x->x_height*x->x_zoom;
}

static void fplot_displace(t_gobj *z, t_glist *glist, int dx, int dy){
    t_fplot *obj = (t_fplot *)z;
    obj->x_obj.te_xpix += dx, obj->x_obj.te_ypix += dy;
    t_canvas *cv = glist_getcanvas(glist);
    sys_vgui(".x%lx.c move %lx_outline %d %d\n", cv, obj, dx*obj->x_zoom, dy*obj->x_zoom);
    sys_vgui(".x%lx.c move %lx_frame %d %d\n", cv, obj, dx*obj->x_zoom, dy*obj->x_zoom);
    if(obj->x_receive == &s_)
        sys_vgui(".x%lx.c move %lx_in %d %d\n", cv, obj, dx*obj->x_zoom, dy*obj->x_zoom);
    if(obj->x_send == &s_)
        sys_vgui(".x%lx.c move %lx_out %d %d\n", cv, obj, dx*obj->x_zoom, dy*obj->x_zoom);
    canvas_fixlinesfor(glist, (t_text*)obj);
}

static void fplot_select(t_gobj *z, t_glist *glist, int state){
    t_fplot *x = (t_fplot *)z;
    int xpos = text_xpix(&x->x_obj, glist);
    int ypos = text_ypix(&x->x_obj, glist);
    t_canvas *cv = glist_getcanvas(glist);
    x->x_sel = state;
    if(state){
        sys_vgui(".x%lx.c delete %lx_outline\n", cv, x);
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lx_outline -outline blue -width %d\n",
                 cv, xpos, ypos, xpos+x->x_width*x->x_zoom, ypos+x->x_height*x->x_zoom, x, x->x_zoom);
    }
    else{
        sys_vgui(".x%lx.c delete %lx_outline\n", cv, x);
        if(x->x_edit || x->x_outline)
            sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lx_outline -outline black -width %d\n",
                     cv, xpos, ypos, xpos+x->x_width*x->x_zoom, ypos+x->x_height*x->x_zoom, x, x->x_zoom);
    }
}

static void fplot_delete(t_gobj *z, t_glist *glist){
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void fplot_draw(t_fplot* x, struct _glist *glist, int vis){
    t_canvas *cv = glist_getcanvas(glist);
    int xpos = text_xpix(&x->x_obj, glist), ypos = text_ypix(&x->x_obj, glist);
    int visible = (glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist));
    if(visible || (_Bool)vis){
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -width %d -outline %s -fill white -tags %lx_frame\n",
                 cv, xpos, ypos, xpos + x->x_width*x->x_zoom, ypos + x->x_height*x->x_zoom, x->x_zoom,
                 x->x_outline ? "black" : "white", x);
    }
    
    if(!x->x_init) x->x_init = 1;
    
    sys_vgui(".x%lx.c bind %lx_frame <ButtonRelease> {pdsend [concat %s _mouserelease \\;]}\n", cv, x, x->x_bindname->s_name);
    
    fplot_draw_io_let(x);
}

static void fplot_erase(t_fplot* x, struct _glist *glist){
    t_canvas *cv = glist_getcanvas(glist);
    sys_vgui(".x%lx.c delete %lx_frame\n", cv, x);
    sys_vgui(".x%lx.c delete %lx_outline\n", cv, x);
    sys_vgui(".x%lx.c delete %lx_in\n", cv, x);
    sys_vgui(".x%lx.c delete %lx_out\n", cv, x);
}

static void fplot_vis(t_gobj *z, t_glist *glist, int vis){
    t_fplot* x = (t_fplot*)z;
    vis ? fplot_draw(x, glist, 1) : fplot_erase(x, glist);
}

static void fplot_save(t_gobj *z, t_binbuf *b){
    t_fplot *x = (t_fplot *)z;
    fplot_get_snd_rcv(x);
    binbuf_addv(b, "ssiisiiissi", gensym("#X"), gensym("obj"), x->x_obj.te_xpix, x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)), x->x_width, x->x_height, x->x_outline, x->x_snd_raw,
                x->x_rcv_raw, x->x_latch);
    binbuf_addv(b, ";");
}

//------------------------------- METHODS --------------------------------------------
static void fplot_size_callback(t_fplot *x, t_float w, t_float h){ // callback
    post("callback");
    if ((x->x_width != w) || (x->x_height != h)){
        pd_error(x,"strange size");
        return;
    }
    if(glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist)){
        post("visible");
        t_canvas *cv = glist_getcanvas(x->x_glist);
        int xpos = text_xpix(&x->x_obj, x->x_glist), ypos = text_ypix(&x->x_obj, x->x_glist);
        post("----callback");
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -outline %s -fill black -tags %lx_frame\n",
                 cv, xpos, ypos, xpos+(IOWIDTH*x->x_zoom), ypos+(IHEIGHT*x->x_zoom),
                 x->x_outline ? "black" : "white", x);
        post("----called-back");
        canvas_fixlinesfor(x->x_glist, (t_text*)x);
        if(x->x_edit || x->x_outline){
            sys_vgui(".x%lx.c delete %lx_outline\n", cv, x);
            if(x->x_sel)
                sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lx_outline -outline blue -width %d\n",
                         cv, xpos, ypos, xpos+x->x_width*x->x_zoom, ypos+x->x_height*x->x_zoom, x, x->x_zoom);
            else
                sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lx_outline -outline black -width %d\n",
                         cv, xpos, ypos, xpos+x->x_width*x->x_zoom, ypos+x->x_height*x->x_zoom, x, x->x_zoom);
            fplot_draw_io_let(x);
        }
    }
    else
        fplot_erase(x, x->x_glist);
}

//void fplot_audiobuffer(t_fplot* x, t_symbol* name){
//    t_garray* inputarray;
//    t_word* words = NULL;
//    int length = 0;
//    int nbchans = 0;
//    int nbframes = 0;
//    char nameString[MAXPDSTRING];
//    int multichannel = 0;
//    int ioutw = x->x_width;
//    int iouth = x->x_height;
//    // float minVal, maxVal; //no normalisation for audio buffers for now
//
//    // parsing the 'multiarray'
//    // check there is an array at the end of that name as is (mono)
//    inputarray = (t_garray*)pd_findbyclass(name, garray_class);
//    if (inputarray){
//        nbchans = 1;
//        garray_getfloatwords(inputarray, &length, &words);
//        nbframes = length;
//    }
//    else {
//        snprintf(nameString, MAXPDSTRING, "%s-%d", name->s_name, nbchans);
//        while ((inputarray = (t_garray*)pd_findbyclass(gensym(nameString), garray_class))) {
//            nbchans ++;
//            garray_getfloatwords(inputarray, &length, &words);
//            nbframes = MAX(nbframes, length);
//            snprintf(nameString, MAXPDSTRING, "%s-%d", name->s_name, nbchans);
//        }
//        multichannel = 1;
//    }
//    //if nothing, bails
//    if (!nbchans) {
//        pd_error(x, "[fluid.waveform]: no valid buffer");
//        return;
//    }
//
//    //set min and max to first value
//    // minVal = maxVal = words[0].w_float;
//
//    // post("%d chans %d frames",nbchans,nbframes);
//
//    // if we have something
//
//    // make a local copy of the input
//    // allocate the memory
//    float** localcopy = (float**) malloc((unsigned long)nbchans * sizeof(float*));
//    for(int i=0;i<nbchans;i++) {
//        localcopy[i]=(float*)malloc((unsigned long)nbframes * sizeof(float));
//    }
//
//    // iterated and copy
//    //iterate through channels
//    for (int j = 0;j<nbchans;j++) {
//        // if mono
//        if (!j && !multichannel){
//            inputarray = (t_garray*)pd_findbyclass(name, garray_class);
//        }
//        else { //if fluid format
//            snprintf(nameString, MAXPDSTRING, "%s-%d", name->s_name, j);
//            inputarray = (t_garray*)pd_findbyclass(gensym(nameString), garray_class);
//        }
//        // retrieve sample array and find the min and the max
//        garray_getfloatwords(inputarray, &length, &words);
//        for (int i = 0;i < length;i++){
//            localcopy[j][i] = words[i].w_float;
//            // if (minVal > words[i].w_float) minVal = words[i].w_float;
//            // else if (maxVal < words[i].w_float) maxVal = words[i].w_float;
//        }
//    }
//    // post("%f %f", minVal, maxVal);
//    x->x_nbframes = nbframes;
//
//    // ratios
//    float ratiow = (float)nbframes / ioutw;
//    float ratioh = (float)iouth / nbchans / 2;
//    // post("%d %f %d %f", ioutw, ratiow, iouth, ratioh);
//    if(x->x_def_img)
//        x->x_def_img = 0;
//
//    // for (int ch = 0; ch < nbchans; ch++){
//    //   for (int fr = 0; fr < ioutw; fr++){
//    //     post("in ch %d fr %d = ", ch, fr);
//    //     int min = (int)((localcopy[ch][(int)(fr*ratiow)] * ratioh) + (ratioh * ((2 * ch) + 1)));
//    //     int max = min;
//    //     for (int localidx = (int)(fr*ratiow); localidx < (int)((fr+1)*ratiow); localidx++) {
//    //       int val = (int)((localcopy[ch][(int)(fr*ratiow)] * ratioh) + (ratioh * ((2 * ch) + 1)));
//    //       if (val < min) {
//    //         min = val;
//    //       } else if (val > max) {
//    //         max = val;
//    //       }
//    //     }
//    //     post("%d %d\n", min, max);
//    // //     post("%d %d %d %f %d\n",ch, fr, (int)(fr * ratiow), localcopy[ch][(int)(fr*ratiow)] * ratioh, (int)((localcopy[ch][(int)(fr*ratiow)] * ratioh) + (ratioh * ((2 * ch) + 1))));
//    //   }
//    // }
//    // make an array for the list of peaks in TCLTK
//    sys_vgui("if { [info exists %lx_audiopeaks] == 1 } {array unset %lx_audiopeaks}\n", x->x_fullname, x->x_fullname); // delete previous version
//    sys_vgui("set %lx_audiopeaks {", x->x_fullname); //header to the channel line
//    for (int ch = 0; ch < nbchans; ch++) {
//        for (int fr = 0; fr < ioutw; fr++) {
//            int min = (int)(localcopy[ch][(int)(fr*ratiow)] * ratioh);
//            int max = min;
//            for (int localidx = (int)(fr*ratiow); localidx < (int)((fr+1)*ratiow); localidx++) {
//                int val = (int)(localcopy[ch][localidx] * ratioh);
//                if (val < min) {
//                    min = val;
//                } else if (val > max) {
//                    max = val;
//                }
//            }
//            int centre = (int)(ratioh * ((2 * ch) + 1));
//            sys_vgui("%d %d %d %d ", fr, centre - MAX(0,max) , fr, centre - MIN(min, 0));
//        }
//    }
//    sys_vgui("}\n"); //footer to the channel line
//    fplot_erase(x, x->x_glist);
//    fplot_draw(x, x->x_glist, 1);
//
//    for(int i=0;i<nbchans;i++) {
//        free(localcopy[i]);
//    }
//    free(localcopy);
//}

static void fplot_send(t_fplot *x, t_symbol *s){
    if(s != gensym("")){
        t_symbol *snd = (s == gensym("empty")) ? &s_ : canvas_realizedollar(x->x_glist, s);
        if(snd != x->x_send){
            x->x_snd_raw = s;
            x->x_send = snd;
            x->x_snd_set = 1;
            if(x->x_edit && glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist)){
                if(x->x_send == &s_)
                    fplot_draw_io_let(x);
                else
                    sys_vgui(".x%lx.c delete %lx_out\n", glist_getcanvas(x->x_glist), x);
            }
        }
    }
}

static void fplot_receive(t_fplot *x, t_symbol *s){
    if(s != gensym("")){
        t_symbol *rcv = s == gensym("empty") ? &s_ : canvas_realizedollar(x->x_glist, s);
        if(rcv != x->x_receive){
            if(x->x_receive != &s_)
                pd_unbind(&x->x_obj.ob_pd, x->x_receive);
            x->x_rcv_set = 1;
            x->x_rcv_raw = s;
            x->x_receive = rcv;
            if(x->x_receive == &s_){
                if(x->x_edit && glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist))
                    fplot_draw_io_let(x);
            }
            else{
                pd_bind(&x->x_obj.ob_pd, x->x_receive);
                if(x->x_edit && glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist))
                    sys_vgui(".x%lx.c delete %lx_in\n", glist_getcanvas(x->x_glist), x);
            }
        }
    }
}

static void fplot_outline(t_fplot *x, t_float f){
    int outline = (int)(f != 0);
    if(x->x_outline != outline){
        x->x_outline = outline;
        if(glist_isvisible(x->x_glist) && gobj_shouldvis((t_gobj *)x, x->x_glist)){
            t_canvas *cv = glist_getcanvas(x->x_glist);
            if(x->x_outline){
                int xpos = text_xpix(&x->x_obj, x->x_glist), ypos = text_ypix(&x->x_obj, x->x_glist);
                if(x->x_sel) sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lx_outline -outline blue -width %d\n",
                                      cv, xpos, ypos, xpos+x->x_width*x->x_zoom, ypos+x->x_height*x->x_zoom, x, x->x_zoom);
                else sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lx_outline -outline black -width %d\n",
                              cv, xpos, ypos, xpos+x->x_width*x->x_zoom, ypos+x->x_height*x->x_zoom, x, x->x_zoom);
            }
            else if(!x->x_edit)
                sys_vgui(".x%lx.c delete %lx_outline\n", cv, x);
            
        }
    }
}

static void fplot_latch(t_fplot *x, t_float f){
    x->x_latch = (int)(f != 0);
}

static void edit_proxy_any(t_edit_proxy *p, t_symbol *s, int ac, t_atom *av){
    int edit = ac = 0;
    if(p->p_cnv){
        if(s == gensym("editmode"))
            edit = (int)(av->a_w.w_float);
        else if(s == gensym("obj") || s == gensym("msg") || s == gensym("floatatom")
                || s == gensym("symbolatom") || s == gensym("text") || s == gensym("bng")
                || s == gensym("toggle") || s == gensym("numbox") || s == gensym("vslider")
                || s == gensym("hslider") || s == gensym("vradio") || s == gensym("hradio")
                || s == gensym("vumeter") || s == gensym("mycnv") || s == gensym("selectall")){
            edit = 1;
        }
        else
            return;
        if(p->p_cnv->x_edit != edit){
            p->p_cnv->x_edit = edit;
            t_canvas *cv = glist_getcanvas(p->p_cnv->x_glist);
            if(edit){
                int x = text_xpix(&p->p_cnv->x_obj, p->p_cnv->x_glist);
                int y = text_ypix(&p->p_cnv->x_obj, p->p_cnv->x_glist);
                int w = p->p_cnv->x_width, h = p->p_cnv->x_height;
                if(!p->p_cnv->x_outline)
                    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lx_outline -outline black -width %d\n",
                             cv, x, y, x+w, y+h, p->p_cnv, p->p_cnv->x_zoom);
                fplot_draw_io_let(p->p_cnv);
            }
            else{
                if(!p->p_cnv->x_outline)
                    sys_vgui(".x%lx.c delete %lx_outline\n", cv, p->p_cnv);
                sys_vgui(".x%lx.c delete %lx_in\n", cv, p->p_cnv);
                sys_vgui(".x%lx.c delete %lx_out\n", cv, p->p_cnv);
            }
        }
    }
}

static void fplot_zoom(t_fplot *x, t_floatarg zoom){
    x->x_zoom = (int)zoom;
    //draw? if so waveform too
    fplot_draw_io_let(x);
}

//------------------- Properties --------------------------------------------------------
void fplot_properties(t_gobj *z, t_glist *gl){
    gl = NULL;
    t_fplot *x = (t_fplot *)z;
    fplot_get_snd_rcv(x);
    char buffer[512];
    sprintf(buffer, "fplot_properties %%s %d %d %d %d {%s} {%s} \n",
            x->x_width,
            x->x_height,
            x->x_outline,
            x->x_latch,
            x->x_snd_raw->s_name,
            x->x_rcv_raw->s_name);
    gfxstub_new(&x->x_obj.ob_pd, x, buffer);
}

static void fplot_ok(t_fplot *x, t_symbol *s, int ac, t_atom *av){
    // post("in ok");
    s = NULL;
    x->x_width = MAX(10, (int)(atom_getfloatarg(0, ac, av)));
    x->x_height = MAX(10, (int)(atom_getfloatarg(1, ac, av)));
    fplot_outline(x, atom_getfloatarg(2, ac, av));
    fplot_latch(x, atom_getfloatarg(3, ac, av));
    fplot_send(x, atom_getsymbolarg(4, ac, av));
    fplot_receive(x, atom_getsymbolarg(5, ac, av));
    canvas_dirty(x->x_glist, 1);
    fplot_erase(x, x->x_glist);
    fplot_draw(x, x->x_glist, 1);
}

//-------------------------------------------------------------------------------------
static void edit_proxy_free(t_edit_proxy *p){
    pd_unbind(&p->p_obj.ob_pd, p->p_sym);
    clock_free(p->p_clock);
    pd_free(&p->p_obj.ob_pd);
}

static t_edit_proxy * edit_proxy_new(t_fplot *x, t_symbol *s){
    t_edit_proxy *p = (t_edit_proxy*)pd_new(edit_proxy_class);
    p->p_cnv = x;
    pd_bind(&p->p_obj.ob_pd, p->p_sym = s);
    p->p_clock = clock_new(p, (t_method)edit_proxy_free);
    return(p);
}

static void fplot_free(t_fplot *x){ // delete if variable is unset and image is unused
    if(x->x_receive != &s_)
        pd_unbind(&x->x_obj.ob_pd, x->x_receive);
    pd_unbind(&x->x_obj.ob_pd, x->x_bindname);
    x->x_proxy->p_cnv = NULL;
    clock_delay(x->x_proxy->p_clock, 0);
    gfxstub_deleteforkey(x);
}

static void *fplot_new(t_symbol *s, int ac, t_atom *av){
    s = NULL;
    t_fplot *x = (t_fplot *)pd_new(fplot_class);
    t_canvas *cv = canvas_getcurrent();
    x->x_glist = (t_glist*)cv;
    x->x_zoom = x->x_glist->gl_zoom;
    char buf[MAXPDSTRING];
    snprintf(buf, MAXPDSTRING-1, ".x%lx", (unsigned long)cv);
    buf[MAXPDSTRING-1] = 0;
    x->x_proxy = edit_proxy_new(x, gensym(buf));
    sprintf(buf, "#%lx", (long)x);
    pd_bind(&x->x_obj.ob_pd, x->x_bindname = gensym(buf));
    x->x_edit = cv->gl_edit;
    x->x_send = x->x_snd_raw = x->x_receive = x->x_rcv_raw = &s_;
    x->x_rcv_set = x->x_snd_set = x->x_init = x->x_latch = 0;
    x->x_outline =  1;
    x->x_width = x->x_height = 10;
    
    if(ac && av->a_type == A_FLOAT){ // 1ST width
        x->x_width = MAX((int)av->a_w.w_float, 10);
        ac--; av++;
        if(ac && av->a_type == A_FLOAT){ // 2nd height
            x->x_height = MAX((int)av->a_w.w_float, 10);
            ac--; av++;
            if(ac && av->a_type == A_FLOAT){ // 3rd outline
                x->x_outline = (int)(av->a_w.w_float != 0);
                ac--; av++;
                if(ac && av->a_type == A_SYMBOL){ // 4th Send
                    if(av->a_w.w_symbol == gensym("empty")){ // ignore
                        ac--, av++;
                    }
                    else {
                        x->x_send = av->a_w.w_symbol;
                        ac--, av++;
                    }
                    if(ac && av->a_type == A_SYMBOL){ // 5th Receive
                        if(av->a_w.w_symbol == gensym("empty")){ // ignore
                            ac--, av++;
                        }
                        else {
                            x->x_receive = av->a_w.w_symbol;
                            ac--, av++;
                        }
                        
                        if(ac && av->a_type == A_FLOAT){ // 6TH latch
                            x->x_latch = (int)(av->a_w.w_float != 0);
                            ac--; av++;
                        }
                    }
                }
            }
        }
    }
    t_symbol *cursym = NULL, *sym = NULL;
    while(ac > 0){
        if(av->a_type == A_SYMBOL){
            cursym = atom_getsymbolarg(0, ac, av);
            if(cursym == gensym("-outline")){ // no arg
                x->x_flag = x->x_outline = 1;
                ac--, av++;
            }
            else if(cursym == gensym("-latch")){ // no arg
                x->x_flag = x->x_latch = 1;
                ac--, av++;
            }
            else if(cursym == gensym("-send")){
                if(ac >= 2 && (av+1)->a_type == A_SYMBOL){
                    sym = atom_getsymbolarg(1, ac, av);
                    x->x_flag = x->x_s_flag = 1;
                    if(sym != gensym("empty"))
                        x->x_send = sym;
                    ac-=2, av+=2;
                }
                else goto errstate;
            }
            else if(cursym == gensym("-receive")){
                if(ac >= 2 && (av+1)->a_type == A_SYMBOL){
                    sym = atom_getsymbolarg(1, ac, av);
                    x->x_flag = x->x_r_flag = 1;
                    if(sym != gensym("empty"))
                        x->x_receive = sym;
                    ac-=2, av+=2;
                }
                else goto errstate;
            }
            else goto errstate;
        }
        else goto errstate;
    };
    if(x->x_receive != &s_)
        pd_bind(&x->x_obj.ob_pd, x->x_receive);
    x->x_outlet = outlet_new(&x->x_obj, &s_bang);
    return(x);
errstate:
    pd_error(x, "[fluid.plotter]: improper args");
    return NULL;
}

void setup_fluid0x2eplotter(void){
    fplot_class = class_new(gensym("fluid.plotter"), (t_newmethod)fplot_new, (t_method)fplot_free, sizeof(t_fplot),0, A_GIMME,0);
    class_addmethod(fplot_class, (t_method)fplot_outline, gensym("outline"), A_DEFFLOAT, 0);
    class_addmethod(fplot_class, (t_method)fplot_latch, gensym("latch"), A_DEFFLOAT, 0);
    class_addmethod(fplot_class, (t_method)fplot_send, gensym("send"), A_DEFSYMBOL, 0);
    class_addmethod(fplot_class, (t_method)fplot_ok, gensym("ok"), A_GIMME, 0);
    class_addmethod(fplot_class, (t_method)fplot_receive, gensym("receive"), A_DEFSYMBOL, 0);
    class_addmethod(fplot_class, (t_method)fplot_zoom, gensym("zoom"), A_CANT, 0);
    class_addmethod(fplot_class, (t_method)fplot_size_callback, gensym("_fplotsize"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(fplot_class, (t_method)fplot_mouserelease, gensym("_mouserelease"), 0);
    edit_proxy_class = class_new(0, 0, 0, sizeof(t_edit_proxy), CLASS_NOINLET | CLASS_PD, 0);
    class_addanything(edit_proxy_class, edit_proxy_any);
    fplot_widgetbehavior.w_getrectfn  = fplot_getrect;
    fplot_widgetbehavior.w_displacefn = fplot_displace;
    fplot_widgetbehavior.w_selectfn   = fplot_select;
    fplot_widgetbehavior.w_deletefn   = fplot_delete;
    fplot_widgetbehavior.w_visfn      = fplot_vis;
    fplot_widgetbehavior.w_clickfn    = (t_clickfn)fplot_click;
    class_setwidget(fplot_class, &fplot_widgetbehavior);
    class_setsavefn(fplot_class, &fplot_save);
    class_setpropertiesfn(fplot_class, &fplot_properties);
    sys_vgui("if {[catch {pd}]} {\n");
    sys_vgui("    proc pd {args} {pdsend [join $args \" \"]}\n");
    sys_vgui("}\n");
    sys_vgui("proc fplot_ok {id} {\n");
    sys_vgui("    set vid [string trimleft $id .]\n");
    sys_vgui("    set var_width [concat var_width_$vid]\n");
    sys_vgui("    set var_height [concat var_height_$vid]\n");
    sys_vgui("    set var_outline [concat var_outline_$vid]\n");
    sys_vgui("    set var_latch [concat var_latch_$vid]\n");
    sys_vgui("    set var_snd [concat var_snd_$vid]\n");
    sys_vgui("    set var_rcv [concat var_rcv_$vid]\n");
    sys_vgui("\n");
    sys_vgui("    global $var_width\n");
    sys_vgui("    global $var_height\n");
    sys_vgui("    global $var_outline\n");
    sys_vgui("    global $var_latch\n");
    sys_vgui("    global $var_snd\n");
    sys_vgui("    global $var_rcv\n");
    sys_vgui("\n");
    sys_vgui("    set cmd [concat $id ok \\\n");
    sys_vgui("        [eval concat $$var_width] \\\n");
    sys_vgui("        [eval concat $$var_height] \\\n");
    sys_vgui("        [eval concat $$var_outline] \\\n");
    sys_vgui("        [eval concat $$var_latch] \\\n");
    sys_vgui("        [string map {\"$\" {\\$} \" \" {\\ } \";\" \"\" \",\" \"\" \"\\\\\" \"\" \"\\{\" \"\" \"\\}\" \"\"} [eval concat $$var_snd]] \\\n");
    sys_vgui("        [string map {\"$\" {\\$} \" \" {\\ } \";\" \"\" \",\" \"\" \"\\\\\" \"\" \"\\{\" \"\" \"\\}\" \"\"} [eval concat $$var_rcv]] \\;]\n");
    sys_vgui("    pd $cmd\n");
    sys_vgui("    fplot_cancel $id\n");
    sys_vgui("}\n");
    sys_vgui("proc fplot_cancel {id} {\n");
    sys_vgui("    set cmd [concat $id cancel \\;]\n");
    sys_vgui("    pd $cmd\n");
    sys_vgui("}\n");
    sys_vgui("proc fplot_properties {id width height outline latch snd rcv} {\n");
    sys_vgui("    set vid [string trimleft $id .]\n");
    sys_vgui("    set var_width [concat var_width_$vid]\n");
    sys_vgui("    set var_height [concat var_height_$vid]\n");
    sys_vgui("    set var_outline [concat var_outline_$vid]\n");
    sys_vgui("    set var_latch [concat var_latch_$vid]\n");
    sys_vgui("    set var_snd [concat var_snd_$vid]\n");
    sys_vgui("    set var_rcv [concat var_rcv_$vid]\n");
    sys_vgui("\n");
    sys_vgui("    global $var_width\n");
    sys_vgui("    global $var_height\n");
    sys_vgui("    global $var_outline\n");
    sys_vgui("    global $var_latch\n");
    sys_vgui("    global $var_snd\n");
    sys_vgui("    global $var_rcv\n");
    sys_vgui("\n");
    sys_vgui("    set $var_width $width\n");
    sys_vgui("    set $var_height $height\n");
    sys_vgui("    set $var_outline $outline\n");
    sys_vgui("    set $var_latch $latch\n");
    sys_vgui("    set $var_snd [string map {{\\ } \" \"} $snd]\n"); // remove escape from space
    sys_vgui("    set $var_rcv [string map {{\\ } \" \"} $rcv]\n"); // remove escape from space
    sys_vgui("\n");
    sys_vgui("    toplevel $id\n");
    sys_vgui("    wm title $id {[fluid.plotter] Properties}\n");
    sys_vgui("    wm protocol $id WM_DELETE_WINDOW [concat fplot_cancel $id]\n");
    sys_vgui("\n");
    sys_vgui("    frame $id.size\n");
    sys_vgui("    pack $id.size -side top\n");
    sys_vgui("    label $id.size.lw -text \"Width:\"\n");
    sys_vgui("    entry $id.size.w -textvariable $var_width -width 12\n");
    sys_vgui("    label $id.size.lh -text \"Height:\"\n");//dirty pad
    sys_vgui("    entry $id.size.h -textvariable $var_height -width 12\n");
    sys_vgui("    pack $id.size.lw $id.size.w $id.size.lh $id.size.h -side left\n");
    sys_vgui("\n");
    sys_vgui("    frame $id.tics\n");
    sys_vgui("    pack $id.tics -side top\n");
    sys_vgui("    label $id.tics.loutline -text \"Outline:\"\n");
    sys_vgui("    checkbutton $id.tics.outline -variable $var_outline \n");
    sys_vgui("    label $id.tics.llatch -text \"                Latch Mode:\"\n");//dirty pad
    sys_vgui("    checkbutton $id.tics.latch -variable $var_latch \n");
    sys_vgui("    pack $id.tics.loutline $id.tics.outline $id.tics.llatch $id.tics.latch -side left\n");
    sys_vgui("\n");
    sys_vgui("    frame $id.snd_rcv\n");
    sys_vgui("    pack $id.snd_rcv -side top\n");
    sys_vgui("    label $id.snd_rcv.lsnd -text \"Send symbol:\"\n");
    sys_vgui("    entry $id.snd_rcv.snd -textvariable $var_snd -width 12\n");
    sys_vgui("    label $id.snd_rcv.lrcv -text \"Receive symbol:\"\n");
    sys_vgui("    entry $id.snd_rcv.rcv -textvariable $var_rcv -width 12\n");
    sys_vgui("    pack $id.snd_rcv.lsnd $id.snd_rcv.snd $id.snd_rcv.lrcv $id.snd_rcv.rcv -side left\n");
    sys_vgui("\n");
    sys_vgui("    frame $id.buttonframe\n");
    sys_vgui("    pack $id.buttonframe -side bottom -fill x -pady 2m\n");
    sys_vgui("    button $id.buttonframe.cancel -text {Cancel} -command \"fplot_cancel $id\"\n");
    sys_vgui("    button $id.buttonframe.ok -text {OK} -command \"fplot_ok $id\"\n");
    sys_vgui("    pack $id.buttonframe.cancel -side left -expand 1\n");
    sys_vgui("    pack $id.buttonframe.ok -side left -expand 1\n");
    sys_vgui("}\n");
}
