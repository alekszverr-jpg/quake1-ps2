/*
 * Copyright (C) 2026 Quake for PlayStation 2 contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Experimental Quake BSP -> PlayStation 2 GS mesh conversion.
 *
 * The software renderer remains the presentation path while this data is
 * validated.  Later v0.5.0 steps will submit these polygons to the GS/VU1
 * backend without changing the on-disk BSP format.
 */

#ifndef PS2_GS_MESH_H
#define PS2_GS_MESH_H

typedef struct model_s model_t;
typedef struct msurface_s msurface_t;

typedef struct ps2_gs_vertex_s
{
	float position[3];
	float texcoord[2];
	float lightmapcoord[2];
} ps2_gs_vertex_t;

typedef struct ps2_gs_poly_s
{
	int numverts;
	int numtriangles;
	ps2_gs_vertex_t verts[1];
} ps2_gs_poly_t;

typedef struct ps2_gs_mesh_stats_s
{
	int surfaces;
	int vertices;
	int triangles;
	int bytes;
} ps2_gs_mesh_stats_t;

void PS2_GS_BuildSurfaceMesh(model_t *model, msurface_t *surface);
void PS2_GS_ResetMeshStats(void);
const ps2_gs_mesh_stats_t *PS2_GS_GetMeshStats(void);

#endif
