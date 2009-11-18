/** \file
    Graphics-related functions

    Copyright (C) 2007 - 2008 Jeffrey P.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/
#include "pspcomic.h"

inline void* access_pixel(const SDL_Surface* surf, const Uint16 x, const Uint16 y) {
	return (void*)((char*)surf->pixels + x*surf->format->BytesPerPixel + surf->pitch * y);
}
inline void copy_pixel(const void *src, void *dst, Uint8 bpp) {
	#ifdef PSP
	while(bpp--) *((char*)dst++) = *((char*)src++);
	#else
	memcpy(dst,src,bpp);
	#endif
}
inline void blend(void *pixel, SDL_PixelFormat *fmt, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	Uint8 old_r, old_g, old_b, old_a;
	Uint8 new_r, new_g, new_b, new_a;
	switch(fmt->BytesPerPixel) {
		case 1:
			SDL_GetRGBA(*(Uint8*)pixel,fmt,&old_r,&old_g,&old_b,&old_a);
		break;
		case 2:
			SDL_GetRGBA(*(Uint16*)pixel,fmt,&old_r,&old_g,&old_b,&old_a);
		break;
		case 3: {
			Uint32 buffer;
			buffer = ((Uint8*)pixel)[0] | (((Uint8*)pixel)[1] << 8) | (((Uint8*)pixel)[2] << 16);
			SDL_GetRGBA(buffer,fmt,&old_r,&old_g,&old_b,&old_a);
		} break;
		case 4:
			SDL_GetRGBA(*(Uint32*)pixel,fmt,&old_r,&old_g,&old_b,&old_a);
		break;
	}
	new_r = (Uint8) ((((Uint32)old_r << 8) + ((Uint32)r * a)) / (256 + a));
	new_g = (Uint8) ((((Uint32)old_g << 8) + ((Uint32)g * a)) / (256 + a));
	new_b = (Uint8) ((((Uint32)old_b << 8) + ((Uint32)b * a)) / (256 + a));
	new_a = a + old_a;
	Uint32 new_color = SDL_MapRGBA(fmt,new_r,new_g,new_b,new_a);
	switch(fmt->BytesPerPixel) {
		case 1: *(Uint8*)pixel = (Uint8)new_color; break;
		case 2: *(Uint16*)pixel = (Uint16)new_color; break;
		case 3: {
			((Uint8*)pixel)[0] = new_color;
			((Uint8*)pixel)[1] = new_color >> 8;
			((Uint8*)pixel)[2] = new_color >> 16;
		} break;
		case 4: *(Uint32*)pixel = (Uint32)new_color; break;
	}
}

/*
typedef struct {
	float u, v;
	u32 color;
	float x, y, z;
} gu_vertex;
static gu_vertex __attribute__((aligned(16))) rot0[4] = 
{
	{ 0.0f, 1.0f, 0x80808080, 0.0f, 256.0f,0.0f },
	{ 1.0f, 0.0f, 0x80808080, 256.0f, 0.0f,0.0f },
};
*/

SDL_Surface* rotozoom(SDL_Surface *old, SDL_Rect *clip, Uint16 newW, Uint16 newH, char rotation) {
	#ifndef USE_GFX
	/* Doesn't work
	#ifdef PSP
	if(*access_int_global(access_resize) == resize_hardware) {
		SDL_Rect temp;
		if(clip == NULL) {
			temp.x = 0;
			temp.y = 0;
			temp.w = old->w;
			temp.h = old->h;   
			clip = &temp;
		}
		float x_ratio;
		float y_ratio;
		if(newW == 0) {
			if(newH == 0) {
				if((rotation % 4) == 0) return old;
				x_ratio = 1.0f;
				y_ratio = 1.0f;
			} else {
				x_ratio = y_ratio = ((float)newH)/old->h;
				newW = old->w * x_ratio;
			}
		} else if(newH == 0) {
			x_ratio = y_ratio = ((float)newW)/old->w;
			newH = old->h * y_ratio;
		} else if((newW == old->w) && (newH == old->h) && ((rotation % 4) == 0))
			return old;
		else {
			x_ratio = ((float)newW)/old->w;
			y_ratio = ((float)newH)/old->h;
		}
		device scr_acc = access_device(access_screen);
		SDL_Surface *ret = SDL_CreateRGBSurface(
			SDL_SWSURFACE, newW, newH,
			24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		if(!ret) return NULL;
		SDL_Surface *hw_buf = SDL_CreateRGBSurface(
			SDL_HWSURFACE, 256, 256,
			32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		SDL_Surface *hw_draw_buf = SDL_CreateRGBSurface(
			SDL_HWSURFACE, 256, 256,
			32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		if(!hw_buf || !hw_draw_buf) {
			if(hw_buf) SDL_FreeSurface(hw_buf);
			else SDL_FreeSurface(hw_draw_buf);
			SDL_FreeSurface(ret);
			return NULL;
		}
		void *display_list = memalign(16,262144);
		if(!display_list) {
			//TODO
		}
		void *draw_buf = (void*)((int)scr_acc.screen->pixels-0x4000000);
		sceKernelDcacheWritebackInvalidateAll();
		sceGuStart(GU_DIRECT,display_list);
		sceGuEnable(GU_TEXTURE_2D);
		//sceGuDrawBuffer(GU_PSM_8888,(void*)((int)hw_buf->pixels-0x4000000),256);
		sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadIdentity();
		sceGumOrtho(0.0f,256.0f,256.0f,0.0f,-1.0f,1.0f);
		sceGumMatrixMode(GU_VIEW);
		sceGumLoadIdentity();
		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);	
		sceGuTexFunc(GU_TFX_DECAL, GU_TCC_RGB);
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);
		sceGuTexWrap(GU_CLAMP,GU_CLAMP);
		sceGuTexScale(x_ratio, y_ratio);
		sceGuTexOffset(0.0f, 0.0f);
		sceGuTexImage(0,256,256,512,hw_buf->pixels);
		sceGuFinish();
		sceGuSync(0,0);
		int i;
		for(i = 0; i < clip->h; i+=256) {
			int j;
			for(j = 0; j < clip->w; j+=256) {
				SDL_Rect region = { .x = clip->x + j, .y = clip->y + i, .w = 256, .h = 256 };
				SDL_BlitSurface(old,NULL,hw_buf,NULL);
				SDL_LockSurface(hw_buf);
				SDL_LockSurface(scr_acc.screen);
				sceKernelDcacheWritebackInvalidateAll();
				sceGuStart(GU_DIRECT,display_list);
				sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
				sceGumDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,NULL,rot0);
				sceGuFinish();
				sceGuSync(0,0);
				region.x = j*x_ratio;
				region.y = i*y_ratio;
				region.w *= x_ratio;
				region.h *= y_ratio;
				SDL_UnlockSurface(hw_buf);
				SDL_UnlockSurface(scr_acc.screen);
				SDL_BlitSurface(scr_acc.screen,NULL,ret,NULL);
				flip_screen();
			}
		}
		//sceGuDrawBuffer(GU_PSM_8888,draw_buf,512);
		SDL_FreeSurface(hw_buf);
		SDL_FreeSurface(hw_draw_buf);
		free(display_list);
		return ret;
	}
	#endif
	*/
	SDL_Rect src_rect;
	if(clip == NULL) {
		src_rect.x = 0;
		src_rect.y = 0;
		src_rect.w = old->w;
		src_rect.h = old->h;
	} else src_rect = *clip;
	if((newW == 0) && (newH != 0)) newW = (Uint16)(((float)newH/src_rect.h)*old->w);
	else if((newH == 0) && (newW != 0)) newH = (Uint16)(((float)newW/src_rect.w)*old->h);
	else if((newW == 0) && (newH == 0)) {
		newW = old->w;
		newH = old->h;
	}
	if((clip == NULL) && (newW == old->w) && (newH == old->h) && ((rotation % 4) == 0)) return old;
	int current_resize_method = *access_int_global(access_resize);
	scale_up_cpu();
	if((newW/old->w >= 2) && (newH/old->h >= 2)) current_resize_method = resize_nn;
	SDL_Surface *new_surf = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		(rotation % 2)?newH:newW, (rotation % 2)?newW:newH,
		((current_resize_method == resize_nn) || (old->format->BytesPerPixel >= 3)) ? old->format->BitsPerPixel :
		((old->format->BytesPerPixel == 1) && (old->flags & SDL_SRCCOLORKEY) && (current_resize_method == resample))? 32 : 24,
		(old->format->BytesPerPixel == 1)?0x00FF0000:old->format->Rmask,
		(old->format->BytesPerPixel == 1)?0x0000FF00:old->format->Gmask,
		(old->format->BytesPerPixel == 1)?0x000000FF:old->format->Bmask,
		(((old->format->BytesPerPixel == 1) && (old->flags & SDL_SRCCOLORKEY) && (current_resize_method == resample)) ||
		(old->format->BytesPerPixel == 4))?0xFF000000:0 //Not sure why it has to be done this way, but it does...
	);
	if(!new_surf) {
		scale_down_cpu();
		sdl_error();
		return NULL;
	}
	if((new_surf->format->BytesPerPixel == 1) &&
		  !(copy_palette(old, new_surf) == 1)) {
		SDL_FreeSurface(new_surf);
		scale_down_cpu();
		sdl_error();
		return NULL;
	}
	/* if(new_surf->format->BytesPerPixel == 4)
		SDL_SetAlpha(new_surf,SDL_SRCALPHA,SDL_ALPHA_TRANSPARENT); */
	SDL_LockSurface(old);
	SDL_LockSurface(new_surf);
	Uint32 i, j;
	Uint8 *new_pixel, *row_start;
	Uint32 new_pitch = new_surf->pitch;
	Uint8 bpp = new_surf->format->BytesPerPixel;
	Uint32 new_to_next_row = new_pitch-bpp*(newW - 1);
	Uint32 new_to_prev_column = new_pitch*newH - bpp;
	int oldW = old->w;
	int oldH = old->h;
	Uint32 old_pitch = new_surf->pitch;
	Uint32 old_to_next_row = old_pitch-old->format->BytesPerPixel*(old->w - 1);
	Uint32 old_to_next_column = -old_pitch*oldH + old->format->BytesPerPixel;
	Uint32 old_bpp = old->format->BytesPerPixel;
	if(rotation % 2) {
		newW = new_surf->w;
		newH = new_surf->h;
	}
	char amount = rotation % 4;
	if((newW == oldW) && (newH == oldH)) {
		//Old dims == new dims. Just rotate.
		Uint8 *old_pixel = old->pixels;
		switch(amount) {
			case 1:
			new_pixel = access_pixel(new_surf,i,newH);
			for(j = newH; j--;) {
				for(i = newW; i--;) {
					copy_pixel(old_pixel,new_pixel,bpp);
					old_pixel = (Uint8*)old_pixel - old_pitch;
					new_pixel = (Uint8*)new_pixel - bpp;
				}
				old_pixel = (Uint8*)old_pixel + old_bpp;
				new_pixel = (Uint8*)new_pixel - new_to_next_row;
			} break;

			case 2:
			new_pixel = access_pixel(new_surf,newW-1,newH-1);
			for(i = newW; i--;) {
				for(j = newH; j--;) {
					copy_pixel(old_pixel,new_pixel,bpp);
					old_pixel = (Uint8*)old_pixel + old_pitch;
					new_pixel = (Uint8*)new_pixel - new_pitch;
				}
				new_pixel = (Uint8*)new_pixel + new_to_prev_column;
				old_pixel = (Uint8*)old_pixel + old_to_next_column;
			} break;

			case 3:
			new_pixel = new_surf->pixels;
			for(i = newW; i--;) {
				for(j = newH; j--;) {
					copy_pixel(old_pixel,new_pixel,bpp);
					old_pixel = (Uint8*)old_pixel + bpp;
					new_pixel = (Uint8*)new_pixel - new_pitch;
				}
				old_pixel = (Uint8*)old_pixel + old_to_next_row;
				new_pixel = (Uint8*)new_pixel + new_to_next_row;
			} break;
		}
	} else {
		switch(current_resize_method) {
			case resize_nn: {
				Uint32 locationX, locationY;
				Uint8 *old_pixel;
				new_pixel = new_surf->pixels;
				for(i = newH; i--;) {
					row_start = new_pixel;
					for(j = 0; j < newW; ++j,new_pixel+=bpp) {
						switch(amount) {
							case 0:
							locationX = (j<<16)/newW*(src_rect.w-1) + (src_rect.x<<16);
							locationY = (src_rect.h<<16) - ((i<<16)/newH*(src_rect.h-1) + (src_rect.y<<16)) - 1;
							break;
							case 1:
							locationX = (src_rect.w<<16) - ((i<<16)/newH*(src_rect.w-1) + (src_rect.x<<16)) - 1;
							locationY = (src_rect.h<<16) - ((j<<16)/newW*(src_rect.h-1) + (src_rect.y<<16)) - 1;
							break;
							case 2:
							locationX = (src_rect.w<<16) - ((j<<16)/newW*(src_rect.w-1) + (src_rect.x<<16)) - 1;
							locationY = ((i<<16)/newH*(src_rect.h-1) + (src_rect.y<<16));
							break;
							case 3:
							locationX = (i<<16)/newH*(src_rect.w-1) + (src_rect.x<<16);
							locationY = (j<<16)/newW*(src_rect.h-1) + (src_rect.y<<16);
							break;
						}
						old_pixel = access_pixel(old,(Uint16)(locationX>>16),(Uint16)(locationY>>16));
						copy_pixel(old_pixel,new_pixel,bpp);
					}
					new_pixel = row_start + new_pitch;
				}
			} break;
			case resample: {
				Uint32 locationX, locationY;
				Uint16 cornerX0, cornerX1, cornerY0, cornerY1;
				Uint8 *old_pixel0, *old_pixel1;
				SDL_Surface *buffer = NULL;
				if(old->format->BytesPerPixel < 3) {
					buffer = old;
					old = SDL_CreateRGBSurface(
						SDL_SWSURFACE,
						buffer->w, buffer->h, 32,
						0x00FF0000, 0x0000FF00,
						0x000000FF, 0xFF000000
					);
					SDL_SetAlpha(old,SDL_SRCALPHA,SDL_ALPHA_TRANSPARENT);
					SDL_UnlockSurface(buffer);
					SDL_BlitSurface(buffer,NULL,old,NULL);
					SDL_UpdateRect(old,0,0,0,0);
					SDL_LockSurface(old);
				}
				Uint8 k = new_surf->format->BytesPerPixel;
				Uint16 val;
				new_pixel = access_pixel(new_surf,newW-1,newH-1);
				for(i = newH; i--;) {
					row_start = new_pixel;
					for(j = newW; j--; new_pixel -= k*2-1) {
						switch(amount) {
							case 0:
							locationX = (j<<16)/newW*(src_rect.w-1) + (src_rect.x<<16);
							locationY = (i<<16)/newH*(src_rect.h-1) + (src_rect.y<<16);
							break;
							case 1:
							locationX = (i<<16)/newH*(src_rect.w-1) + (src_rect.x<<16);
							locationY = (src_rect.h<<16) - ((j<<16)/newW*(src_rect.h-1) + (src_rect.y<<16)) - 1;
							break;
							case 2:
							locationX = (src_rect.w<<16) - ((j<<16)/newW*(src_rect.w-1) + (src_rect.x<<16)) - 1;
							locationY = (src_rect.h<<16) - ((i<<16)/newH*(src_rect.h-1) + (src_rect.y<<16)) - 1;
							break;
							case 3:
							locationX = (src_rect.w<<16) - ((i<<16)/newH*(src_rect.w-1) + (src_rect.x<<16)) - 1;
							locationY = (j<<16)/newW*(src_rect.h-1) + (src_rect.y<<16);
							break;
						}
						cornerX0 = (Uint16)(locationX>>16);
						cornerY0 = (Uint16)(locationY>>16);
						cornerX1 = cornerX0;
						cornerY1 = cornerY0;

						if(amount > 1) {
							if(locationX-(cornerX0<<16)) --cornerX1;
						}
						else {
							if(cornerX0+1<oldW) ++cornerX1;
						}

						if((amount < 3) && (amount > 0)) {
							if(locationY-(cornerY0<<16)) --cornerY1;
						}
						else {
							if(cornerY0+1<oldH) ++cornerY1;
						}

						old_pixel0 = access_pixel(old,cornerX0,cornerY0);
						old_pixel1 = access_pixel(old,cornerX1,cornerY1);
						//Basically, this averages two pixels' values and puts it
						//into the new image.
						val = ((Uint16)(*(old_pixel0))+
							(Uint16)(*(old_pixel1)))>>1;
						*((Uint8*)new_pixel) = (Uint8)val;
						//Do it up to four times, and also pre-increment the
						//variables so that the pointer is right immediately
						val = ((Uint16)(*(++old_pixel0))+
							(Uint16)(*(++old_pixel1)))>>1;
						*((Uint8*)++new_pixel) = (Uint8)val;
						val = ((Uint16)(*(++old_pixel0))+
							(Uint16)(*(++old_pixel1)))>>1;
						*((Uint8*)++new_pixel) = (Uint8)val;
						if(k == 4) {
							val = ((Uint16)(*(++old_pixel0))+
								(Uint16)(*(++old_pixel1)))>>1;
							*((Uint8*)++new_pixel) = (Uint8)val;
						}
					}
					new_pixel = row_start - new_pitch;
				}
				if(buffer) {
					SDL_FreeSurface(old);
					old = NULL;
				}
			} break;
		}
	}
	if(old) SDL_UnlockSurface(old);
	SDL_UnlockSurface(new_surf);
	scale_down_cpu();
	return new_surf;
	#else /* USE_GFX */
	int current_resize_method = *access_int_global(access_resize);
	SDL_Rect temp;
	if(clip == NULL) {
		temp.x = 0;
		temp.y = 0;
		temp.w = old->w;
		temp.h = old->h;
		clip = &temp;
	}
	double x_zoom, y_zoom;
	if((newW == 0) && (newH != 0)) x_zoom = y_zoom= newH/(double)clip->h;
	else if((newH == 0) && (newW != 0)) x_zoom = y_zoom= newW/(double)clip->w;
	else if((newW == 0) && (newH == 0)) return x_zoom = y_zoom = 1;
	else {
		x_zoom= newW/(double)clip->w;
		y_zoom= newH/(double)clip->h;
	}
	if((clip == &temp) && (x_zoom == 1) && (y_zoom == 1) && (rotation == 0))
		return old;
	SDL_Surface *buffer;
	if(clip == &temp) buffer = old;
	else {
		buffer = SDL_CreateRGBSurface(
			SDL_SWSURFACE,
			clip->w, clip->h,
			((current_resize_method == resize_nn) || (old->format->BytesPerPixel >= 3)) ? old->format->BitsPerPixel :
			((old->format->BytesPerPixel == 1) && (old->flags & SDL_SRCCOLORKEY) && (current_resize_method == resample))? 32 : 24,
			(old->format->BytesPerPixel == 1)?0x00FF0000:old->format->Rmask,
			(old->format->BytesPerPixel == 1)?0x0000FF00:old->format->Gmask,
			(old->format->BytesPerPixel == 1)?0x000000FF:old->format->Bmask,
			(((old->format->BytesPerPixel == 1) && (old->flags & SDL_SRCCOLORKEY) && (current_resize_method == resample)) ||
			(old->format->BytesPerPixel == 4))?0xFF000000:0 //Not sure why it has to be done this way, but it does...
		);
		if(!buffer) {
			sdl_error();
			return NULL;
		}
		if((buffer->format->BytesPerPixel == 1) &&
			  !(copy_palette(old, buffer) == 1)) {
			SDL_FreeSurface(buffer);
			sdl_error();
			return NULL;
		}
	}
	SDL_Rect all = { 0, 0, buffer->w, buffer->h };
	if(SDL_BlitSurface(old,clip,buffer,&all) != 0) {
		sdl_error();
		return NULL;
	}
	SDL_UpdateRect(buffer,0,0,0,0);
	return rotozoomSurfaceXY(buffer,(rotation%4)*(-90.0),x_zoom,y_zoom,(current_resize_method != resize_nn));
	#endif
}
int copy_palette(SDL_Surface *old, SDL_Surface *new_surf) {
	if((old->format->BytesPerPixel != 1) || (new_surf->format->BytesPerPixel != 1))
		return -1;
	SDL_Palette *pal = old->format->palette;
	SDL_Color *colors = malloc(sizeof(SDL_Color)*pal->ncolors);
	if(!colors) {
		error_message(get_message(mesg_out_of_memory));
		return -2;
	}
	memcpy(colors,pal->colors,pal->ncolors*sizeof(SDL_Color));
	if(old->flags & SDL_SRCCOLORKEY)
		SDL_SetColorKey(new_surf, SDL_SRCCOLORKEY|SDL_RLEACCEL, old->format->colorkey);
	return SDL_SetPalette(new_surf, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, pal->ncolors);
}
void blend_rect(SDL_Surface *surf, SDL_Rect rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	if((rect.w == 0) || (rect.h == 0)) return;
	SDL_LockSurface(surf);
	Uint16 i;
	void *pixel;
	scale_up_cpu();
	pixel = access_pixel(surf,rect.x,rect.y);
	for(i = 0; i < rect.w; i++) {
		blend(pixel,surf->format,r,g,b,a);
		pixel += surf->format->BytesPerPixel;
	}
	if(rect.h > 1) {
		pixel = access_pixel(surf,rect.x,rect.y+1);
		for(i = 1; i < rect.h; i++) {
			blend(pixel,surf->format,r,g,b,a);
			pixel += surf->pitch;
		}
		if(rect.w > 1) {
			pixel = access_pixel(surf,rect.x+rect.w-1,rect.y+1);
			for(i = 1; i < rect.h; i++) {
				blend(pixel,surf->format,r,g,b,a);
				pixel += surf->pitch;
			}
			pixel = access_pixel(surf,rect.x+1,rect.y+rect.h-1);
			for(i = 1; i < rect.w-1; i++) {
				blend(pixel,surf->format,r,g,b,a);
				pixel += surf->format->BytesPerPixel;
			}
		}
	}
	SDL_UnlockSurface(surf);
	scale_down_cpu();
}
