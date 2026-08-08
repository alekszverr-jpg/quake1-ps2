/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*
	vid_ps2.c -- PlayStation 2 video driver
	
	by Nicolas Plourde a.k.a nic067 <nicolasplourde@hotmail.com>
	
	See http://www.ps2dev.org for all your ps2 coding need.
*/

#include "quakedef.h"
#include "d_local.h"

#include "ps2_gs.h"

viddef_t	vid;				// global video state

static int verbose=0;
int ignorenext;

#ifndef PS2_INTERNAL_WIDTH
#define PS2_INTERNAL_WIDTH 640
#endif

#if PS2_INTERNAL_WIDTH != 320 && PS2_INTERNAL_WIDTH != 640
#error PS2_INTERNAL_WIDTH must be 320 or 640
#endif

#ifdef _NTSC
#define	BASEWIDTH	PS2_INTERNAL_WIDTH
#define	BASEHEIGHT  224
#define HORIZONTAL_MAGNIFICATION (2560 / BASEWIDTH)
#endif
#ifdef _VESA
#define	BASEWIDTH	PS2_INTERNAL_WIDTH
#define	BASEHEIGHT  480
#define HORIZONTAL_MAGNIFICATION (1280 / BASEWIDTH)
#endif
#ifdef _PAL //PAL
#define	BASEWIDTH	PS2_INTERNAL_WIDTH
#define	BASEHEIGHT  265 
#define HORIZONTAL_MAGNIFICATION (2560 / BASEWIDTH)
#endif

byte	*vid_buffer;
long	zbuffer;
void	*surfcache;
int		surfcachesize;

static long highhunkmark;
static long buffersize;

unsigned short	d_8to16table[256];

static uint32 tmp[BASEWIDTH*BASEHEIGHT] __attribute__((aligned(64)));

typedef struct
{
	char red;
	char green;
	char blue;
}RGB;

RGB myPalette[256];

#ifdef PS2_DIAGNOSTIC_METRICS
static float metrics_frame_ms;
static float metrics_ee_ms;
static float metrics_convert_ms;
static float metrics_gs_ms;
static float metrics_previous_frame_end;

static void VID_SmoothMetric(float *average, float sample)
{
	if (*average == 0.0f)
		*average = sample;
	else
		*average += (sample - *average) * 0.1f;
}

static void VID_DrawMetrics(void)
{
	char line[48];
	float fps;

	fps = metrics_frame_ms > 0.0f ? 1000.0f / metrics_frame_ms : 0.0f;

	Draw_Fill(0, 0, vid.width, 18, 0);
	snprintf(line, sizeof(line), "FPS %4.1f FRAME %4.1f %ix%i",
		fps, metrics_frame_ms, vid.width, vid.height);
	Draw_String(4, 1, line);
	snprintf(line, sizeof(line), "EE %4.1f CONV %4.1f GS %4.1f",
		metrics_ee_ms, metrics_convert_ms, metrics_gs_ms);
	Draw_String(4, 9, line);
}
#endif

void ResetFrameBuffer(void)
{
	//int mem;
	//int pwidth;

	if (d_pzbuffer)
	{
		D_FlushCaches ();
		Hunk_FreeToHighMark (highhunkmark);
		d_pzbuffer = NULL;
	}
	highhunkmark = Hunk_HighMark ();

// alloc an extra line in case we want to wrap, and allocate the z-buffer
	buffersize = vid.width * vid.height * sizeof (*d_pzbuffer);

	surfcachesize = D_SurfaceCacheForRes (vid.width, vid.height);

	buffersize += surfcachesize;

	d_pzbuffer = Hunk_HighAllocName (buffersize, "video");
	if (d_pzbuffer == NULL)
		Sys_Error ("Not enough memory for video mode\n");

	surfcache = (byte *) d_pzbuffer + vid.width * vid.height * sizeof (*d_pzbuffer);

	D_InitCaches(surfcache, surfcachesize);

	vid.buffer = vid_buffer;
	vid.conbuffer = vid.buffer;
}

void VID_SetPalette (unsigned char *palette)
{
	int i;
	for (i=0 ; i<256 ; i++)
	{
		myPalette[i].red = palette[i*3] * 257;
		myPalette[i].green = palette[i*3+1] * 257;
		myPalette[i].blue = palette[i*3+2] * 257;
	}
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette(palette);
}

void	VID_Init (unsigned char *palette)
{
	ignorenext=0;
	vid.width = BASEWIDTH;
	vid.height = BASEHEIGHT;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.numpages = 2;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	verbose=COM_CheckParm("-verbose");
	
	vid_buffer = malloc(vid.width*vid.height);
	if (vid_buffer == NULL)
		Sys_Error ("Unable to allocate video buffer");
	
	ResetFrameBuffer();

	vid.rowbytes = vid.width;
	vid.buffer = vid_buffer;
	vid.direct = 0;
	vid.conbuffer = vid_buffer;
	vid.conrowbytes = vid.rowbytes;
	vid.conwidth = vid.width;
	vid.conheight = vid.height;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);

	#ifdef _NTSC
	/*
	 * Current PS2SDK NTSC timing uses an origin of (652, 26) for a
	 * 2560x224 display area.  The old port used y=36, which pushed the
	 * picture down by ten scanlines and clipped the status bar on CRTs.
	 */
	GS_MODE mode = {2,BASEWIDTH,224,0,32,HORIZONTAL_MAGNIFICATION,652,26};//NTSC-NI
	#endif
	#ifdef _VESA
	GS_MODE mode = {0x1C,BASEWIDTH,480,0,32,HORIZONTAL_MAGNIFICATION,356,18}; //VESA 75 Hz
    #endif
    #ifdef _PAL
	GS_MODE mode = {3,BASEWIDTH,480,0,32,HORIZONTAL_MAGNIFICATION,656,35}; //PAL
	#endif
	gs_init(mode);
	fill_rect(0, 0, BASEWIDTH, BASEHEIGHT);
}

void	VID_Shutdown (void)
{
	Con_Printf("VID_Shutdown\n");
}
	
void	VID_Update (vrect_t *rects)
{
	extern int scr_fullupdate;
	#ifdef PS2_DIAGNOSTIC_METRICS
	float convert_end;
	float frame_end;
	float update_start;
	float upload_start;
	#endif
	int hud_source_x;
	int hud_top;
	int source_offset;
	int x;
	int y;
	byte pixel;

	scr_fullupdate = 0;

	#ifdef PS2_DIAGNOSTIC_METRICS
	update_start = Sys_FloatTime();
	VID_DrawMetrics();
	#endif

	/*
	 * Quake's status bar artwork is a fixed 320 pixels wide.  Keep the
	 * world renderer at 640 pixels, but expand only the HUD while converting
	 * the indexed frame to the 32-bit GS upload buffer.
	 */
	hud_top = vid.height - sb_lines;
	hud_source_x = (cl.gametype == GAME_DEATHMATCH)
		? 0
		: (vid.width - 320) / 2;

	for (y = 0; y < vid.height; y++)
	{
		for (x = 0; x < vid.width; x++)
		{
			if (vid.width == 640 && sb_lines > 0 && y >= hud_top)
				source_offset = y * vid.rowbytes + hud_source_x + (x >> 1);
			else
				source_offset = y * vid.rowbytes + x;

			pixel = vid.buffer[source_offset];
			tmp[y * vid.width + x] =
				((uint8)(myPalette[pixel].red)   << 0)  |
				((uint8)(myPalette[pixel].green) << 8)  |
				((uint8)(myPalette[pixel].blue)  << 16) |
				((uint8)(255)                    << 24);
		}
	}

	#ifdef PS2_DIAGNOSTIC_METRICS
	convert_end = Sys_FloatTime();
	upload_start = convert_end;
	#endif
	put_image(0, 0, vid.width, vid.height, (uint32*)tmp);
	#ifdef PS2_DIAGNOSTIC_METRICS
	frame_end = Sys_FloatTime();
	if (metrics_previous_frame_end > 0.0f)
	{
		VID_SmoothMetric(&metrics_frame_ms,
			(frame_end - metrics_previous_frame_end) * 1000.0f);
		VID_SmoothMetric(&metrics_ee_ms,
			(update_start - metrics_previous_frame_end) * 1000.0f);
		VID_SmoothMetric(&metrics_convert_ms,
			(convert_end - update_start) * 1000.0f);
		VID_SmoothMetric(&metrics_gs_ms,
			(frame_end - upload_start) * 1000.0f);
	}
	metrics_previous_frame_end = frame_end;
	#endif
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
}


