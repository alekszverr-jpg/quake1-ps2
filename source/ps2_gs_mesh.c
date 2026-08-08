/*
 * Copyright (C) 2026 Quake for PlayStation 2 contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * BSP polygon extraction for the experimental PS2 GS renderer.
 *
 * This follows the same edge-walking scheme as GLQuake's
 * BuildSurfaceDisplayList.  Quake BSP faces are convex, so the backend can
 * turn every polygon into a triangle fan without an additional tessellator.
 */

#include <stddef.h>

#include "quakedef.h"
#include "ps2_gs_mesh.h"

static ps2_gs_mesh_stats_t mesh_stats;

void PS2_GS_ResetMeshStats(void)
{
	memset(&mesh_stats, 0, sizeof(mesh_stats));
}

const ps2_gs_mesh_stats_t *PS2_GS_GetMeshStats(void)
{
	return &mesh_stats;
}

void PS2_GS_BuildSurfaceMesh(model_t *model, msurface_t *surface)
{
	ps2_gs_poly_t *poly;
	int alloc_size;
	int edge_index;
	int i;

	if (!model || !surface || surface->numedges < 3)
		Sys_Error("PS2_GS_BuildSurfaceMesh: invalid BSP face");

	alloc_size = offsetof(ps2_gs_poly_t, verts) +
		surface->numedges * sizeof(ps2_gs_vertex_t);
	poly = Hunk_AllocName(alloc_size, "gs_poly");
	poly->numverts = surface->numedges;
	poly->numtriangles = surface->numedges - 2;

	for (i = 0; i < surface->numedges; ++i)
	{
		mvertex_t *source;
		ps2_gs_vertex_t *dest;
		float s;
		float t;

		edge_index = model->surfedges[surface->firstedge + i];
		if (edge_index >= 0)
			source = &model->vertexes[model->edges[edge_index].v[0]];
		else
			source = &model->vertexes[model->edges[-edge_index].v[1]];

		dest = &poly->verts[i];
		VectorCopy(source->position, dest->position);

		s = DotProduct(source->position, surface->texinfo->vecs[0]) +
			surface->texinfo->vecs[0][3];
		t = DotProduct(source->position, surface->texinfo->vecs[1]) +
			surface->texinfo->vecs[1][3];

		/* Base texture coordinates match GLQuake and naturally repeat. */
		dest->texcoord[0] = s / surface->texinfo->texture->width;
		dest->texcoord[1] = t / surface->texinfo->texture->height;

		/*
		 * Keep lightmap coordinates local to the surface for now.  The atlas
		 * allocator will add its block offset when lightmaps are uploaded.
		 */
		dest->lightmapcoord[0] =
			(s - surface->texturemins[0] + 8.0f) /
			(surface->extents[0] + 16.0f);
		dest->lightmapcoord[1] =
			(t - surface->texturemins[1] + 8.0f) /
			(surface->extents[1] + 16.0f);
	}

	surface->ps2_gs_poly = poly;
	mesh_stats.surfaces++;
	mesh_stats.vertices += poly->numverts;
	mesh_stats.triangles += poly->numtriangles;
	mesh_stats.bytes += alloc_size;
}
