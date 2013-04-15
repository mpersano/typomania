/* Copyright (C) 2010 Mauro Persano
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* dumpglyphs.c -- part of vulcan
 *
 * This program is copyright (C) 2006 Mauro Persano, and is free
 * software which is freely distributable under the terms of the
 * GNU public license, included as the file COPYING in this
 * distribution.  It is NOT public domain software, and any
 * redistribution not permitted by the GNU General Public License is
 * expressly forbidden without prior written permission from
 * the author.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h> /* for basename(3) */
#include <png.h>
#include <ft2build.h>
#include <alloca.h>
#include FT_FREETYPE_H
#include <assert.h>

struct glyph {
	int code;
	int width;
	int height;
	int left;
	int top;
	int advance_x;
	int advance_y;
	int texture_x;
	int texture_y;
	int transposed;
	unsigned *bitmap;
};

struct node {
	int x, y;
	int width, height;
	int used;
	struct node *left, *right;
};

struct glyph_range {
	struct glyph_range *next;
	int start_code;
	int end_code;
};

static char ttf_filename[PATH_MAX + 1];
static char font_basename[NAME_MAX];

static int font_size;

static struct glyph_range *glyph_range_list;
static int num_glyphs;

static struct glyph *glyphs;

static int texture_width;
static int texture_height;

static char texture_filename[PATH_MAX + 1];

static unsigned *texture;

static char fontdef_filename[PATH_MAX + 1];

static int can_transpose;
static int can_pack;
static int use_gradient;
static int drop_shadows;
static int drop_shadow_dist;
static int glyph_border_size = 4;

static unsigned gradient_from, gradient_to;
static unsigned bg_color, fg_color;

enum {
	NUM_LOW_PASS_FILTER_PASSES = 2,
};

static const float DROP_SHADOW_ALPHA = .5;

void
panic(const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "FATAL: ");

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fputc('\n', stderr);

	exit(1);
}

static void
init_glyph(struct glyph *g, int code, FT_Face face)
{
	unsigned *p, *q, *rgba[2];
	const unsigned char *r;
	FT_GlyphSlot slot;
	int n, i, j, src, dest;
	int bg_red, bg_green, bg_blue;

	if ((FT_Load_Char(face, code, FT_LOAD_RENDER)) != 0)
		panic("FT_Load_Char");

	slot = face->glyph;

	g->code = code;
	g->width = slot->bitmap.width + 2*glyph_border_size;
	g->height = slot->bitmap.rows + 2*glyph_border_size;
	g->left = slot->bitmap_left;
	g->top = slot->bitmap_top;
	g->advance_x = slot->advance.x/64;
	g->advance_y = slot->advance.y/64;
	g->texture_x = g->texture_y = -1;
	g->transposed = 0;

	rgba[0] = calloc(sizeof *rgba[0], g->width*g->height);
	rgba[1] = calloc(sizeof *rgba[1], g->width*g->height);

	for (i = 0; i < g->width*g->height; i++)
		rgba[0][i] = bg_color;

	bg_red = bg_color & 0xff;
	bg_green = (bg_color >> 8) & 0xff;
	bg_blue = (bg_color >> 16) & 0xff;

	p = &rgba[0][glyph_border_size*g->width + glyph_border_size];
	r = slot->bitmap.buffer;

	for (i = 0; i < slot->bitmap.rows; i++) {
		int fg_red;
		int fg_green;
		int fg_blue;

		if (!use_gradient) {
			fg_red = fg_color & 0xff;
			fg_green = (fg_color >> 8) & 0xff;
			fg_blue = (fg_color >> 16) & 0xff;
		} else {
#define LERP(name, byte) \
			name = \
			  ((int)(gradient_from >> byte*8) & 0xff) + \
			  (((int)((gradient_to >> byte*8) & 0xff) -  \
			   ((int)(gradient_from >> byte*8) & 0xff))*i)/ \
			   slot->bitmap.rows;
			LERP(fg_red, 0)
			LERP(fg_green, 1)
			LERP(fg_blue, 2)
#undef LERP
		}

#define MIX_COMPONENT(t, name) (((fg_ ## name * (t)) + (bg_ ## name * (255 - (t)))) / 255)
#define MIX_COLOR(t) \
	MIX_COMPONENT(t, red) | \
	(MIX_COMPONENT(t, green) << 8) | \
	(MIX_COMPONENT(t, blue) << 16) | \
	t << 24

		if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
			for (j = 0; j < slot->bitmap.pitch; j++)
				p[j] = MIX_COLOR(r[j]);
		} else if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
			unsigned *q = p;
			int j;

			for (j = 0; j < slot->bitmap.pitch; j++) {
				int k;
				unsigned ch = r[j];

				for (k = 0; k < 8; k++) {
					if (ch & 0x80)
						*q = fg_red | (fg_green << 8) | (fg_blue << 16) | 0xff000000;
					ch <<= 1;
					++q;
				}
			}
		} else {
			printf("unknown pixel mode: %d\n",
			  slot->bitmap.pixel_mode);
			assert(0);
		}

		p += g->width;
		r += slot->bitmap.pitch;
	}

	/* low pass filter on alpha channel */

	src = 0;
	dest = 1;

	for (n = 0; n < NUM_LOW_PASS_FILTER_PASSES; n++) {
		p = rgba[dest];
		q = rgba[src];

		for (i = 0; i < g->height; i++) {
			for (j = 0; j < g->width; j++) {
				unsigned v = 0;
				unsigned *t = q - g->width - 1;

#define ACC_ROW { if (j > 0) v += (t[0] >> 24); \
  v += (t[1] >> 24); if (j < g->width - 1) v += (t[2] >> 24); }
				if (i > 0)
					ACC_ROW
				t += g->width;

				ACC_ROW
				t += g->width;

				if (i < g->height - 1)
					ACC_ROW
#undef ACC_ROW

				v /= 9;

				*p++ = (*q++ & 0xffffff) | (v << 24);
			}
		}

		src ^= 1;
		dest ^= 1;
	}

	free(rgba[dest]);

	/* enhance */

	for (p = rgba[src]; p != &rgba[src][g->width*g->height]; p++) {
		unsigned v = (*p >> 24) << 3;
		if (v > 0xff)
			v = 0xff;
		*p = (*p & 0xffffff) | (v << 24);
	}

	if (drop_shadows) {
		r = slot->bitmap.buffer;

		for (i = 0; i < slot->bitmap.rows; i++) {
			for (j = 0; j < slot->bitmap.pitch; j++) {
				unsigned *p = &rgba
				  [src]
				  [(glyph_border_size + i + drop_shadow_dist)*g->width + glyph_border_size + j + drop_shadow_dist];
				unsigned v = *p;
				float alpha, s;
				
				s = 0;

				if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
					s = (float)r[j]/255;
				} else if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
					/* TODO: IMPLEMENT ME */
				}

				alpha = (float)(v >> 24)/255 + .25*s;
				if (alpha > 1)
					alpha = 1;

				*p = (v & 0xffffff) | ((int)(alpha*255) << 24);
			}

			r += slot->bitmap.pitch;
		}
	}

	g->bitmap = rgba[src];
}

static void
gen_glyphs(void)
{
	FT_Library library;
	FT_Face face;
	struct glyph_range *p;
	int glyph_index;

	if ((FT_Init_FreeType(&library)) != 0)
		panic("FT_Init_FreeType");

	if ((FT_New_Face(library, ttf_filename, 0, &face)) != 0)
		panic("FT_New_Face");

	if ((FT_Set_Char_Size(face, font_size*64, 0, 100, 0)) != 0)
		panic("FT_Set_Char_Size");

	num_glyphs = 0;

	for (p = glyph_range_list; p; p = p->next)
		num_glyphs += p->end_code - p->start_code + 1;

	glyphs = calloc(num_glyphs, sizeof *glyphs);

	glyph_index = 0;

	for (p = glyph_range_list; p; p = p->next) {
		int i;

		for (i = p->start_code; i <= p->end_code; i++)
			init_glyph(&glyphs[glyph_index++], i, face);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

static struct node *
node_make(int x, int y, int width, int height)
{
	struct node *p = malloc(sizeof *p);

	p->x = x;
	p->y = y;
	p->width = width;
	p->height = height;
	p->used = 0;
	p->left = p->right = NULL;

	return p;
}

void
node_free(struct node *root)
{
	if (root->left)
		node_free(root->left);

	if (root->right)
		node_free(root->right);

	free(root);
}

/* find smallest node in which box fits */
struct node *
node_find(struct node *root, int width, int height)
{
	struct node *p, *q;

	if (root->left) {
		/* not a leaf */

		p = node_find(root->left, width, height);

		q = node_find(root->right, width, height);

		if (p == NULL)
			return q;

		if (q == NULL)
			return p;

		return p->width*p->height < q->width*q->height ? p : q;
	} else {
		/* leaf node */

		/* already used? */
		if (root->used)
			return NULL;

		/* box fits? */
		if (root->width < width || root->height < height)
			return NULL;

		return root;
	}
}

struct node *
node_insert(struct node *root, int width, int height)
{
	struct node *p;

	if (root->left) {
		/* not a leaf */

		if ((p = node_insert(root->left, width, height)) != NULL)
			return p;

		return node_insert(root->right, width, height);
	} else {
		/* leaf */

		int dw, dh;

		if (root->used)
			return NULL;

		if (root->width < width || root->height < height)
			return NULL;

		if (root->width == width && root->height == height)
			return root;

		dw = root->width - width;
		dh = root->height - height;

		if (dw > dh) {
			root->left = node_make(root->x, root->y,
			  width, root->height);

			root->right = node_make(root->x + width,
			  root->y, root->width - width, root->height);
		} else {
			root->left = node_make(root->x, root->y,
			  root->width, height);

			root->right = node_make(root->x,
			  root->y + height, root->width,
			  root->height - height);
		}

		return node_insert(root->left, width, height);
	}
}

static int 
compare_glyph(const void *pa, const void *pb)
{
	const struct glyph *a = (const struct glyph *)pa;
	const struct glyph *b = (const struct glyph *)pb;

	return b->width*b->height - a->width*a->height;
}

static void
glyph_write_to_texture(const struct glyph *glyph)
{
	int i, j;
	unsigned *p;
	const unsigned *q;

	p = &texture[texture_width*glyph->texture_y + glyph->texture_x];
	q = glyph->bitmap;

	if (!glyph->transposed) {
		for (i = 0; i < glyph->height; i++) {
			memcpy(p, q, glyph->width*sizeof *p);
			p += texture_width;
			q += glyph->width;
		}
	} else {
		for (i = 0; i < glyph->height; i++) {
			for (j = 0; j < glyph->width; j++) {
				p[j*texture_width + i] = q[i*glyph->width + j];
			}
		}
	}
}

void
pack_glyphs(void)
{
	struct node *root;
	struct glyph *g;
	int max_glyph_width, max_glyph_height;

	texture = calloc(sizeof *texture, texture_width*texture_height);

	if (!can_pack) {
		assert(num_glyphs > 0);

		max_glyph_width = glyphs[0].width;
		max_glyph_height = glyphs[0].height;

		for (g = &glyphs[1]; g != &glyphs[num_glyphs]; g++) {
			if (g->width > max_glyph_width)
				max_glyph_width = g->width;

			if (g->height > max_glyph_height)
				max_glyph_height = g->height;
		}
	}

	/* sort glyphs by area */

	qsort(glyphs, num_glyphs, sizeof *glyphs, compare_glyph);

	/* pack glyphs in texture */

	root = node_make(0, 0, texture_width, texture_height);

	for (g = glyphs; g != &glyphs[num_glyphs]; g++) {
		struct node *n, *nt;
		int g_width, g_height;

		if (can_pack) {
			g_width = g->width;
			g_height = g->height;
		} else {
			g_width = max_glyph_width;
			g_height = max_glyph_height;
		}

		/* smallest node in which glyph fits */
		n = node_find(root, g_width, g_height);

		/* smallest node in which transposed glyph fits */
		nt = can_transpose ? node_find(root, g_height, g_width) : NULL;

		if (n == NULL && nt == NULL)
			panic("texture too small?");

		if (nt == NULL || n->width*n->height <= nt->width*nt->height) {
			/* allocate node for glyph */

			n = node_insert(n, g_width, g_height);
			assert(n != NULL);
		} else {
			g->transposed = 1;

			/* allocate node for transposed glyph */

			n = node_insert(nt, g_height, g_width);
			assert(n != NULL);
		}

		n->used = 1;

		g->texture_x = n->x;
		g->texture_y = n->y;

		if (!can_pack)
			g->texture_y += max_glyph_width - g->top;

		glyph_write_to_texture(g);
	}

	node_free(root);
}


#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

static void
write_texture(void)
{
	png_structp png_ptr;
	png_infop info_ptr;
	FILE *fp;
	int i;

	if ((fp = fopen(texture_filename, "wb")) == NULL)
		panic("fopen");

	if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	  (png_voidp)NULL, NULL, NULL)) == NULL)
		panic("png_create_write_struct");

	if ((info_ptr = png_create_info_struct(png_ptr)) == NULL)
		panic("png_create_info_struct");

	if (setjmp(png_jmpbuf(png_ptr)))
		panic("png error");

	png_init_io(png_ptr, fp);

	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	png_set_IHDR(png_ptr, info_ptr, texture_width, texture_height, 8,
	  PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
	  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);

	for (i = 0; i < texture_height; i++)
		png_write_row(png_ptr,
		  (unsigned char *)&texture[i*texture_width]);

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
}

static int
glyph_compare(const void *p1, const void *p2)
{
	return (*(const struct glyph **)p1)->code - (*(const struct glyph **)p2)->code;
}

static void
write_fontdef(void)
{
	FILE *fp;
	struct glyph **glyph_ptrs;
	int i;

	glyph_ptrs = malloc(num_glyphs*sizeof *glyph_ptrs);

	for (i = 0; i < num_glyphs; i++)
		glyph_ptrs[i] = &glyphs[i];

	qsort(glyph_ptrs, num_glyphs, sizeof *glyph_ptrs, glyph_compare);

	if ((fp = fopen(fontdef_filename, "w")) == NULL)
		panic("fopen");

	const float ds = 1./texture_width;
	const float dt = 1./texture_height;

	for (i = 0; i < num_glyphs; i++) {
		const struct glyph *g = glyph_ptrs[i];

		float t0x, t0y, t1x, t1y, t2x, t2y, t3x, t3y;

		t0x = ds*g->texture_x;
		t0y = dt*g->texture_y;

		if (!g->transposed) {
			t1x = ds*(g->texture_x + g->width); 
			t1y = dt*g->texture_y;

			t2x = ds*(g->texture_x + g->width); 
			t2y = dt*(g->texture_y + g->height);

			t3x = ds*g->texture_x;
			t3y = dt*(g->texture_y + g->height);
		} else {
			t1x = ds*g->texture_x;
			t1y = dt*(g->texture_y + g->width);

			t2x = ds*(g->texture_x + g->height);
			t2y = dt*(g->texture_y + g->width);

			t3x = ds*(g->texture_x + g->height); 
			t3y = dt*g->texture_y;
		}

		fprintf(fp, "%d %d %d %d %d %d %d %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f\n",
			g->code, g->width, g->height, g->left, g->top,
			g->advance_x, g->advance_y,
			t0x, t0y, t1x, t1y, t2x, t2y, t3x, t3y);
	}

	fclose(fp);

	free(glyph_ptrs);
}

static void
usage(char *argv0)
{
	fprintf(stderr, "Usage: %s [options] <font file> <start-code>-<end-code> ...\n", argv0);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options are:\n");
	fprintf(stderr, "  -S  font size\n");
	fprintf(stderr, "  -W  texture width\n");
	fprintf(stderr, "  -H  texture height\n");
	fprintf(stderr, "  -h  show usage\n");
	fprintf(stderr, "  -I  font base name\n");
	fprintf(stderr, "  -t  don't transpose glyphs\n");
	fprintf(stderr, "  -d  don't pack glyphs\n");
	fprintf(stderr, "  -s  drop shadow distance\n");
	fprintf(stderr, "  -g  color gradient ([from]-[to] colors, in hex)\n");
	fprintf(stderr, "  -f  foreground color, in hex\n");
	fprintf(stderr, "  -b  background color, in hex\n");

	exit(1);
}

int
main(int argc, char *argv[])
{
	int i, c;

	font_size = 26;
	texture_width = 256;
	texture_height = 256;
	can_transpose = 1;
	can_pack = 1;
	drop_shadows = 0;
	bg_color = 0x00000000;
	fg_color = 0x00ffffff;

	while ((c = getopt(argc, argv, "S:s:e:I:W:H:g:htdb:f:")) != EOF) {
		char *after;

		switch (c) {
			case 'S':
				font_size = strtol(optarg, &after, 10);
				if (after == optarg)
					usage(*argv);
				break;

			case 'I':
				strncpy(font_basename, optarg,
				  sizeof font_basename);
				break;

			case 'W':
				texture_width = strtol(optarg, &after, 10);
				if (after == optarg)
					usage(*argv);
				break;

			case 'H':
				texture_height = strtol(optarg, &after, 10);
				if (after == optarg)
					usage(*argv);
				break;

			case 'g':
				if (sscanf(optarg, "%x-%x", &gradient_from, &gradient_to) != 2)
					usage(*argv);
				use_gradient = 1;
				break;

			case 't':
				can_transpose = 0;
				break;

			case 'd':
				can_pack = 0;
				break;

			case 's':
				drop_shadow_dist = strtol(optarg, &after, 10);
				if (after == optarg)
					usage(*argv);
				drop_shadows = 1;
				glyph_border_size += drop_shadow_dist;
				break;

			case 'b':
				if (sscanf(optarg, "%x", &bg_color) != 1)
					usage(*argv);
				break;

			case 'f':
				if (sscanf(optarg, "%x", &fg_color) != 1)
					usage(*argv);
				break;

			case 'h':
				usage(*argv);
				break;
		}
	}

	if (optind >= argc - 1)
		usage(*argv);

	strncpy(ttf_filename, argv[optind], sizeof ttf_filename - 1);

	for (i = optind + 1; i < argc; i++) {
		int start_code, end_code;
		char *dash, *after;
		struct glyph_range *range;
		
		dash = strchr(argv[i], '-');

		start_code = strtol(argv[i], &after, 10);
		if (after == argv[i])
			usage(*argv);

		if (dash == NULL) {
			end_code = start_code;
		} else {
			end_code = strtol(dash + 1, &after, 10);
			if (after == dash + 1)
				usage(*argv);
		}

		range = malloc(sizeof *range);

		range->start_code = start_code;
		range->end_code = end_code;
		range->next = glyph_range_list;

		glyph_range_list = range;
	}

	if (!*font_basename) {
		char temp[PATH_MAX + 1], *dot;

		strncpy(temp, ttf_filename, sizeof temp - 1);

		strncpy(font_basename, basename(temp),
		  sizeof font_basename - 1);
	
		dot = strrchr(font_basename, '.');
		if (dot != NULL)
			*dot = '\0';
	}

	snprintf(texture_filename, sizeof texture_filename,
	  "%s_font.png", font_basename);

	snprintf(fontdef_filename, sizeof fontdef_filename,
	  "%s_font.fnt", font_basename);

	gen_glyphs();
	pack_glyphs();
	write_texture();
	write_fontdef();

	return 0;
}
