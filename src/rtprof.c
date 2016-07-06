/*
 *
 *   Copyright (c) 2011 Samuel Degrande
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 */
/**
 * \file probe.c
 * \brief Real-time in-game profiler's implementations
 */

#define _probe_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

// The Real-time profiler is a dev tool that will be rarely used. Then better to
// avoid compiling the code if it is not used.

#ifdef WITH_RTPROF

#include "rtprof.h"
#include <limits.h>
#include <float.h>

static LIST_HEAD(probe_list); // List of all registerd probe
static int probe_active = FALSE; // Active/Inactive global flag

// Reports' components size
static const int title_height = 20;
static const int bar_width = 200;
static const int bar_size = 10;
static const int stats_width = 200;
static const int stats1D_height = 50;
static const int stats2D_height = 200;
static const int report_width = 270;
static const int report_padding = 5;

/*==================== Generic functions ====================*/

/**
 * Initialize probe's base properties, and add it to the probes' list
 *
 * \param probe Pointer to the probe base struct
 * \param title Title of the probe, as displayed on the profiler report screen
 * \param width Report's width
 * \param height Report's height
 */
static void probe_register(struct probe *probe, char *title, int width, int height)
{
	probe->title = strdup(title);
	probe->triggered = FALSE;
	probe->report_width = width;
	probe->report_height = height;
	probe->display = NULL;
	probe->clear = NULL;
	list_add(&probe->node, &probe_list);
}

/*==================== Accum related functions ====================*/

/**
 * Clear accumulation struct
 *
 * \param accum Pointer to the accum struct to clear
 */
static void probe_accum_clear(struct probe_accum *accum)
{
	accum->val = 0;
	accum->cntr = 0;
}

/**
 * Add a value into the accumulator
 *
 * \param accum Pointer to the accum struct to use
 * \param value Value to be accumulated
 */
static void probe_accum_add(struct probe_accum *accum, int value)
{
	accum->val += value;
	accum->cntr++;
}

/**
 * Get the mean value
 *
 * Get the mean value of the accumulated data.
 * The mean value is equal to the accumulated value divided by the number of
 * accumulations.
 *
 * \param accum Pointer to the accum struct to use
 * \return mean value, or 0 if the accumulator is empty
 */
static int probe_accum_get_mean(struct probe_accum *accum)
{
	return ((accum->cntr != 0) ? (int)(accum->val / accum->cntr) : 0);
}

/*==================== 1D stats related functions ====================*/

/**
 * Clear 1D statistic struct
 *
 * \param stats Pointer to the stats struct to clear
 */
static void probe_stats_clear(struct probe_stats *stats)
{
	memset(stats->store, 0, stats->max_expected * sizeof(int));
	stats->min_value = INT_MAX;
	stats->max_value = -1;
	stats->overflow_value = 0;
	stats->overflow_cntr = 0;
	stats->scale = 1.0;
}

/**
 * Initialize a 1D statistic struct
 *
 * \param stats Pointer to the stats struct to clear
 * \param max Maximum expected value
 */
static void probe_stats_init(struct probe_stats *stats, int max)
{
	stats->store = (int *)MyMalloc(max * sizeof(int));
	stats->max_expected = max;
	probe_stats_clear(stats);
}

/**
 * Add a value into the 1D statistic
 *
 * Add a value into the 1D statistic store. If the value is higher than the
 * expected one, it is recorded as an overflow.
 * The highest overflowing value is kept, as well as the number of overflows.
 *
 * \param stats Pointer to the stats struct to clear
 * \param value The value to record
 * \return FALSE is the value generated an overflow, TRUE otherwise
 */
static int probe_stats_add(struct probe_stats *stats, int value)
{
	// Is there an overflow ?
	if (value >= stats->max_expected) {
		stats->overflow_cntr++;
		// Record the highest overflow
		if (value > stats->overflow_value) {
			stats->overflow_value = value;
			return FALSE;
		}
		return TRUE;
	}

	// Increment the number of occurrence of the given value
	stats->store[value]++;

	// Compute the min and max values
	if (value < stats->min_value)
		stats->min_value = value;
	if (value > stats->max_value)
		stats->max_value = value;

	return TRUE;
}

/*==================== 2D stats related functions ====================*/

/**
 * Clear 2D statistic struct
 *
 * \param stats Pointer to the array_stats struct to clear
 */
static void probe_array_stats_clear(struct probe_array_stats *stats)
{
	memset(stats->store, 0, stats->max_expected[1] * stats->max_expected[0] * sizeof(int));
	stats->min_value[0] = stats->min_value[1] = INT_MAX;
	stats->max_value[0] = stats->max_value[1] = -1;
	stats->overflow_value[0] = stats->overflow_value[1] = 0;
	stats->overflow_cntr[0] = stats->overflow_cntr[1] = 0;
	stats->scale[0] = stats->scale[1] = 1.0;
}

/**
 * Initialize a 2D statistic struct
 *
 * \param stats Pointer to the stats struct to clear
 * \param max_x Maximum expected value on X axis
 * \param max_y Maximum expected value on Y axis
 */
static void probe_array_stats_init(struct probe_array_stats *stats, int max_x, int max_y)
{
	stats->store = (int *)MyMalloc(max_y * max_x * sizeof(int));
	stats->max_expected[0] = max_x;
	stats->max_expected[1] = max_y;
	probe_array_stats_clear(stats);
}

/**
 * Add a value pair into the 2D statistic
 *
 * Add a value pair into the 2D statistic store. If the value (on one axis) is
 * higher than the expected one, it is recorded as an overflow.
 * The highest overflowing value is kept, as well as the number of overflows.
 *
 * \param stats Pointer to the stats struct to clear
 * \param val_x The value to record on X axis
 * \param val_y The value to record on Y axis
 * \return FALSE is the value generated an overflow, TRUE otherwise
 */
static int probe_array_stats_add(struct probe_array_stats *stats, int val_x, int val_y)
{
	// Is there an overflow on X axis ?
	if (val_x >= stats->max_expected[0]) {
		stats->overflow_cntr[0]++;
		// Record the highest overflow
		if (val_x > stats->overflow_value[0]) {
			stats->overflow_value[0] = val_x;
			return FALSE;
		}
		return TRUE;
	}

	// Is there an overflow on Y axis ?
	if (val_y >= stats->max_expected[1]) {
		stats->overflow_cntr[1]++;
		// Record the highest overflow
		if (val_y > stats->overflow_value[1]) {
			stats->overflow_value[1] = val_y;
			return FALSE;
		}
		return TRUE;
	}

	// Increment the number of occurrence of the given value pair
	stats->store[val_y * stats->max_expected[0] + val_x]++;

	// Compute the min and max values
	if (val_x < stats->min_value[0])
		stats->min_value[0] = val_x;
	if (val_x > stats->max_value[0])
		stats->max_value[0] = val_x;
	if (val_y < stats->min_value[1])
		stats->min_value[1] = val_y;
	if (val_y > stats->max_value[1])
		stats->max_value[1] = val_y;

	return TRUE;
}

/*==================== Report display related functions ====================*/

/**
 * Display a container around a probe report
 *
 * Display a container around a probe report: a black semi-transparent
 * background + the probe's title
 *
 * \param x left position of the container on the screen
 * \param y top position of the container on the screen
 * \param width width of the container on the screen
 * \param height height of the container on the screen
 * \param title title of the probe
 */
static void probe_display_container(int x, int y, int width, int height, char *title)
{
	SDL_Rect background_rect = { x, y, width, height };
	ShadowingRectangle(Screen, background_rect);

	char text[200];
	sprintf(text, "%s%s", font_switchto_msgstat, title);
	put_string(get_current_font(), x + report_padding, y + report_padding, text);
}

/**
 * Display a bar graph
 *
 * Display a green rectangle whose width is defined by 'val'.
 * Also print the textual representation of 'val', at the right of the bar.
 * The actual width of the bar graph is 'val * scale', while the actual
 * textual representation is 'val / divider'.
 * If the bar graph is wider than 'width', then a yellow rectangle is displayed, to denote an overflow.
 *
 * \param x left position of the bar graph on the screen
 * \param y top position of the bar graph on the screen
 * \param width maximum width of the bar graph
 * \param height height of the bar
 * \param val value to display
 * \param scale multiplier used to compute the actual bar's width
 * \param divider divider used to print the textual value
 */
static void probe_display_bargraph(int x, int y, int width, int height, int val, float scale, int divider)
{
	int bar_width = val * scale;
	int overflow = FALSE;

	// Is there an overflow ?
	if (bar_width > width) {
		overflow = TRUE;
		bar_width = width;
	}

	SDL_Rect fill_rect = { x, y, bar_width, height };
	if (!overflow)
		gl_draw_rectangle(&fill_rect, 0x00, 0xff, 0x00, 0x88); // green bar
	else
		gl_draw_rectangle(&fill_rect, 0xff, 0xff, 0x00, 0x88); // yellow bar on overflow

	// Textual representation at the right of the bar
	char text[16];
	sprintf(text, "%g", (float)val / divider);
	put_string(get_current_font(), x + bar_width, y, text);
}

/**
 * Display an horizontal 'tribar' graph
 *
 * A 'tribar' graph is used to display 3 values (minimum, mean, maximum) on one
 * single bar graph.
 * It is composed of three parts:
 * - a grey rectangle from 0 to 'min_val'
 * - a blue rectangle between 'min_val' and 'mean_val'
 * - a red rectangle between 'mean_val' and 'max_val'
 * Also print the textual representation of the 3 values.
 * The actual width of the bar graph is 'val * scale', while the actual
 * textual representation is 'val / divider'.
 * If the bar graph is wider than 'width', then a yellow rectangle is displayed, to denote an overflow.
 *
 * \param x left position of the bar graph on the screen
 * \param y top position of the bar graph on the screen
 * \param width maximum width of the bar graph
 * \param height height of the bar graph
 * \param min_val minimum value to display
 * \param mean_val mean value to display
 * \param max_val maximum value to display
 * \param scale multiplier used to compute the actual bar's widths
 * \param divider divider used to print the textual values
 */
static void probe_display_htribargraph(int x, int y, int width, int height, int min_val, int mean_val, int max_val, float scale, int divider)
{
	// No value yet recorded
	if (max_val == -1)
		return;

	int bar_min = min_val * scale;
	int bar_mean = mean_val * scale;
	int bar_max = max_val * scale;
	int overflow = FALSE;

	// Is there an overflow ?
	if (bar_max > width) {
		overflow = TRUE;
		bar_max = width;
	}

	SDL_Rect fill_rect = { x, y, bar_min, height };
	gl_draw_rectangle(&fill_rect, 0x88, 0x88, 0x88, 0x88); // grey part

	fill_rect.x += fill_rect.w;
	fill_rect.w = bar_mean - bar_min;
	gl_draw_rectangle(&fill_rect, 0x00, 0x00, 0xff, 0x88); // blue part

	fill_rect.x += fill_rect.w;
	fill_rect.w = bar_max - bar_mean;
	if (!overflow)
		gl_draw_rectangle(&fill_rect, 0xff, 0x00, 0x00, 0x88); // red part
	else
		gl_draw_rectangle(&fill_rect, 0xff, 0xff, 0x00, 0x88); // yellow part if overflow

	// Textual representations
	char text[16];
	sprintf(text, "%g", (float)min_val / divider);
	put_string(get_current_font(), x + bar_min - text_width(get_current_font(), text), y, text);

	sprintf(text, "%g", (float)mean_val / divider);
	put_string(get_current_font(), x + bar_mean - text_width(get_current_font(), text)/2, y, text);

	sprintf(text, "%g", (float)max_val / divider);
	put_string(get_current_font(), x + bar_max, y, text);
}

/**
 * Display a vertical 'tribar' graph
 *
 * A 'tribar' graph is used to display 3 values (minimum, mean, maximum) on one
 * single bar graph.
 * It is composed of three parts:
 * - a grey rectangle from 0 to 'min_val'
 * - a blue rectangle between 'min_val' and 'mean_val'
 * - a red rectangle between 'mean_val' and 'max_val'
 * Also print the textual representation of the 3 values.
 * The actual width of the bar graph is 'val * scale', while the actual
 * textual representation is 'val / divider'.
 * If the bar graph is higher than 'height', then a yellow rectangle is displayed, to denote an overflow.
 *
 * \param x left position of the bar graph on the screen
 * \param y top position of the bar graph on the screen
 * \param width width of the bar graph
 * \param height maximum height of the bar graph
 * \param min_val minimum value to display
 * \param mean_val mean value to display
 * \param max_val maximum value to display
 * \param scale multiplier used to compute the actual bar's widths
 * \param divider divider used to print the textual values
 */
static void probe_display_vtribargraph(int x, int y, int width, int height, int min_val, int mean_val, int max_val, float scale, int divider)
{
	// No value yet recorded
	if (max_val == -1)
		return;

	int bar_min = min_val * scale;
	int bar_mean = mean_val * scale;
	int bar_max = max_val * scale;
	int overflow = FALSE;

	// Is there an overflow ?
	if (bar_max > height) {
		overflow = TRUE;
		bar_max = height;
	}

	SDL_Rect fill_rect = { x, y + height - bar_min, width, bar_min };
	gl_draw_rectangle(&fill_rect, 0x88, 0x88, 0x88, 0x88); // grey part

	fill_rect.y = y + height - bar_mean;
	fill_rect.h = bar_mean - bar_min;
	gl_draw_rectangle(&fill_rect, 0x00, 0x00, 0xff, 0x88); // blue part

	fill_rect.y = y + height - bar_max;
	fill_rect.h = bar_max - bar_mean;
	if (!overflow)
		gl_draw_rectangle(&fill_rect, 0xff, 0x00, 0x00, 0x88); // red part
	else
		gl_draw_rectangle(&fill_rect, 0xff, 0xff, 0x00, 0x88); // yellow part if overflow

	// Textual representations
	char text[16];
	sprintf(text, "%g", (float)min_val / divider);
	put_string(get_current_font(), x - text_width(get_current_font(), text), y + height - bar_min, text);

	sprintf(text, "%g", (float)mean_val / divider);
	put_string(get_current_font(), x - text_width(get_current_font(), text), y + height - bar_mean, text);

	sprintf(text, "%g", (float)max_val / divider);
	put_string(get_current_font(), x - text_width(get_current_font(), text), y + height - bar_max, text);
}

/**
 * Display a 1 dimensional statistic
 *
 * Display a 1D array of vertical bars. The height of each bar depends
 * on the relative number of occurrence of each value.
 * The display size of a bar is adapted to best fit the graph's width and
 * height.
 *
 * \param x left position of the graph on the screen
 * \param y top position of the graph on the screen
 * \param width maximum width of the graph
 * \param height maximum height of the graph
 * \param stats pointer to the statistics
 */
void probe_display_stats(int x, int y, int width, int height, struct probe_stats *stats)
{
	// No value recorder -> nothing to display
	if (stats->max_value == -1)
		return;

	// Adapt the bars width to fit the graph's width.
	// However, the bars width can not be lower than 2 (so that the graph is 'readable')
	int bar_width = max(2, (int)floor((float)width / (float)stats->max_value));

	// Maximum number of bars that can be displayed on the graph
	int nb_bars = width / bar_width;

	// If the number of bars is lower than the number of values to display,
	// then we have to aggregate several values into one bar (this is due to
	// the bars' minimum width limitation)
	int aggregate = (int)ceilf((float)stats->max_value / (float)nb_bars);

	// We end up with a number of bars that is possibly different than the number
	// of values to display (when aggregation is needed). So, we have to
	// 're-sample' the data in a new array. We also record the highest value.
	int highest = 0;
	struct probe_accum *aggregated_bars = (struct probe_accum *)MyMalloc(nb_bars * sizeof(struct probe_accum));
	memset(aggregated_bars, 0, nb_bars * sizeof(struct probe_accum));

	int i;
	for (i = 0; i < stats->max_value; i++) {
		int bar_nb = i / aggregate;
		struct probe_accum *bar = &aggregated_bars[bar_nb];
		probe_accum_add(bar, stats->store[i]);
	}

	for (i = 0; i < nb_bars; i++) {
		struct probe_accum *bar = &aggregated_bars[i];
		bar->val = probe_accum_get_mean(bar);
		if (highest < bar->val)
			highest = bar->val;
	}

	// Draw the statistic graph
	if (highest != 0) {
		float scale = (float)height / (float)highest;
		for (i = 0; i < nb_bars; i++) {
			if (aggregated_bars[i].val == 0)
				continue;
			SDL_Rect fill_rect = { x + i*bar_width, y + height - (int)(aggregated_bars[i].val*scale), bar_width, (int)(aggregated_bars[i].val*scale) };
			gl_draw_rectangle(&fill_rect, 0xff, 0x00, 0x00, 0x88);
		}
		SDL_Rect fill_rect = { x + nb_bars*bar_width, y + height - (int)(stats->overflow_cntr*scale), bar_width, (int)(stats->overflow_cntr*scale) };
		gl_draw_rectangle(&fill_rect, 0xff, 0xff, 0x00, 0x88);
	}

	free(aggregated_bars);

	// Compute an horizontal scale. This scale will be used to adapt a
	// bargraph's size to the actual size of the statistic graph.
	stats->scale = (float)width / (float)(nb_bars * aggregate);
}

/**
 * Display a 2 dimensional statistic graph
 *
 * Display a 2D array of cells using a color scale. The color of each cell
 * depends on its value divided by maximum value found in the array.
 * The display size of a cell is adapted to best fit the graph's width and
 * height.
 *
 * \param x left position of the graph on the screen
 * \param y top position of the graph on the screen
 * \param width maximum width of the graph
 * \param height maximum height of the graph
 * \param stats pointer to the statistics
 */
void probe_display_array_stats(int x, int y, int width, int height, struct probe_array_stats *stats)
{
	const int color_scale_max = 12;
	const int color_scale[][3] = {
			{0x00, 0x00, 0xff},
			{0x00, 0x80, 0xff},
			{0x00, 0xc0, 0xff},
			{0x00, 0xff, 0xff},
			{0x00, 0xff, 0x90},
			{0x00, 0xff, 0x00},
			{0x80, 0xe0, 0x00},
			{0xc0, 0xe0, 0x00},
			{0xff, 0xff, 0x00},
			{0xff, 0xc0, 0x00},
			{0xff, 0x80, 0x00},
			{0xff, 0x00, 0x00}
	};

	// No value recorder -> nothing to display
	if (stats->max_value[0] == -1 || stats->max_value[1] == -1)
		return;

	// Adapt the cells size to fit the graph's width and height.
	// However, the cell size can not be lower than 2 (so that the graph is 'readable')
	int cell_size_x = max(2, (int)floor((float)width / (float)stats->max_value[0]));
	int cell_size_y = max(2, (int)floor((float)height / (float)stats->max_value[1]));

	// Maximum number of cells that can be displayed on the graph
	int nb_cell_x = width / cell_size_x;
	int nb_cell_y = height / cell_size_y;

	// If the number of cells is lower than the number of values to display,
	// then we have to aggregate several values into one cell (this is due to
	// the cell's minimum size limitation)
	int aggregate_x = (int)ceilf((float)stats->max_value[0] / (float)nb_cell_x);
	int aggregate_y = (int)ceilf((float)stats->max_value[1] / (float)nb_cell_y);

	// We end up with a number of cells that is possibly different than the number
	// of values to display (when aggregation is needed). So, we have to
	// 're-sample' the data in a new array. We also record the highest value.
	int highest = 0;
	struct probe_accum *aggregated_cells = (struct probe_accum *)MyMalloc(nb_cell_y * nb_cell_x * sizeof(struct probe_accum));
	memset(aggregated_cells, 0, nb_cell_y * nb_cell_x * sizeof(struct probe_accum));

	int i, j;
	for (j = 0; j < stats->max_value[1]; j++) {
		for (i = 0; i < stats->max_value[0]; i++) {
			int cell_x = i / aggregate_x;
			int cell_y = j / aggregate_y;
			struct probe_accum *cell = &aggregated_cells[cell_y*nb_cell_x + cell_x];
			probe_accum_add(cell, stats->store[j*stats->max_expected[0] + i]);
		}
	}

	for (j = 0; j < nb_cell_y; j++) {
		for (i = 0; i < nb_cell_x; i++) {
			struct probe_accum *cell = &aggregated_cells[j*nb_cell_x + i];
			cell->val = probe_accum_get_mean(cell);
			if (highest < cell->val)
				highest = cell->val;
		}
	}

	// Draw the statistic graph
	if (highest != 0) {
		float scale = (float)color_scale_max / (float)highest;
		for (j = 0; j < nb_cell_y; j++) {
			for (i = 0; i < nb_cell_x; i++) {
				if (aggregated_cells[j*nb_cell_x + i].val == 0)
					continue;
				SDL_Rect fill_rect = { x + i*cell_size_x, y + height - j*cell_size_y, cell_size_x, cell_size_y };
				int color_idx = min(color_scale_max - 1, floor(aggregated_cells[j*nb_cell_x + i].val * scale));
				gl_draw_rectangle(&fill_rect, color_scale[color_idx][0], color_scale[color_idx][1], color_scale[color_idx][2], 0x88);
			}
		}
	}

	free(aggregated_cells);

	// Compute an horizontal and a vertical scale. Those scales will be
	// used to adapt a bargraph's size to the actual size of the statistic graph.
	stats->scale[0] = (float)width / (float)(nb_cell_x * aggregate_x);
	stats->scale[1] = (float)height / (float)(nb_cell_y * aggregate_y);
}

/*==================== Counter probe related functions ====================*/

static void probe_counter_clear(struct probe *probe_base);
static void probe_counter_display(struct probe *probe_base, int x, int y);

/**
 * Create a counter probe
 *
 * \param title Title of the probe, as displayed on the report screen
 * \param max Maximum excepted value of the counter (used to report overflows)
 * \return Pointer to the created probe
 */
struct probe_counter *probe_counter_create(char *title, int max)
{
	struct probe_counter *probe = (struct probe_counter *)MyMalloc(sizeof(struct probe_counter));
	int report_height = report_padding + title_height + stats1D_height + 2 * bar_size + report_padding;

	probe_register((struct probe *)probe, title, report_width, report_height);

	probe->counter = 0;
	probe_accum_clear(&probe->accum);
	probe_stats_init(&probe->stats, max);

	((struct probe *)probe)->display = probe_counter_display;
	((struct probe *)probe)->clear = probe_counter_clear;

	return probe;
}

/**
 * Add a value to a counter probe
 *
 * Add a value to a counter probe, and set the probe as triggered
 *
 * \param probe Pointer to a probe
 * \param value Value to be added to the counter
 */
void probe_counter_add(struct probe_counter *probe, int value)
{
	static int negative_reported = FALSE;

	if (!probe_active)
		return;

	// The value must be positive
	if (value < 0) {
		if (!negative_reported) {
			DebugPrintf(-1, "PROBE error: Graph1D value can not be negative\n");
			negative_reported = TRUE;
		}
	}

	((struct probe *)probe)->triggered = TRUE;
	probe->counter += value;
}

/**
 * Clear the content of a counter probe (reset to 'zero')
 *
 * \param probe_base Pointer to the base's struct of a probe
 */
void probe_counter_clear(struct probe *probe_base)
{
	struct probe_counter *probe = (struct probe_counter *)probe_base;
	probe_accum_clear(&probe->accum);
	probe_stats_clear(&probe->stats);
}

/**
 * Display a report of a counter probe
 *
 * Display a report of a counter probe, composed of 3 components:
 * - a 1 dimensional statistic of the counter's values, since last cleared
 * - an horizontal tribar graph, with the min, mean and max values of the counter
 * - an horizontal bar graph to report the current counter's value
 *
 * \param probe_base Pointer to the base's struct of a probe
 * \param x Left screen position of the report
 * \param y Top screen position of the report
 */
void probe_counter_display(struct probe *probe_base, int x, int y)
{
	struct probe_counter *probe = (struct probe_counter *)probe_base;

	// If the probe was triggered during the current frame, add the counter's
	// value to the statistic and to the accumulator.
	// If a new overflow is detected, report it.
	if (probe_base->triggered) {
		if (!probe_stats_add(&probe->stats, probe->counter)) {
			DebugPrintf(-1, "PROBE error: Counter \"%s\" value is too high (%d)\n", probe_base->title, probe->counter);
		}
		probe_accum_add(&probe->accum, probe->counter);
	}

	// Display the report

	probe_display_container(x, y, probe_base->report_width, probe_base->report_height, probe_base->title);
	x += 20;
	y += title_height;

	probe_display_stats(x, y, stats_width, stats1D_height, &probe->stats);
	y += stats1D_height;

	probe_display_htribargraph(x, y, bar_width,  bar_size,
	                          probe->stats.min_value, probe_accum_get_mean(&probe->accum), probe->stats.max_value,
	                          probe->stats.scale, 1);
	y += bar_size;

	probe_display_bargraph(x, y, bar_width, bar_size, probe->counter, probe->stats.scale, 1);

	// Prepare the probe for the next frame's report
	probe->counter = 0;
	probe_base->triggered = FALSE;
}

/*==================== Timer probe related functions ====================*/

static void probe_timer_clear(struct probe *probe_base);
static void probe_timer_display(struct probe *probe_base, int x, int y);

/**
 * Compute the diff between 2 timespecs (helper func)
 *
 * \param start Start time
 * \param end End time
 * \return A timespec struct containing 'end - start'
 */
static struct timespec probe_timer_diff(struct timespec *start, struct timespec *end)
{
	struct timespec temp;

	if (end->tv_nsec < start->tv_nsec) {
		temp.tv_sec = end->tv_sec - start->tv_sec - 1;
		temp.tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
	} else {
		temp.tv_sec = end->tv_sec - start->tv_sec;
		temp.tv_nsec = end->tv_nsec - start->tv_nsec;
	}

	return temp;
}

/**
 * Create a timer probe
 *
 * \param title Title of the probe, as displayed on the report screen
 * \param max_usecs Maximum excepted value of the timer, in microseconds (used to report overflows)
 * \return Pointer to the created probe
 */
struct probe_timer *probe_timer_create(char *title, int max_usecs)
{
	struct probe_timer *probe = (struct probe_timer *)MyMalloc(sizeof(struct probe_timer));
	int report_height = report_padding + title_height + stats1D_height + 2 * bar_size + report_padding;

	probe_register((struct probe *)probe, title, report_width, report_height);

	probe_accum_clear(&probe->per_frame_accum);
	probe_accum_clear(&probe->overall_accum);
	probe_stats_init(&probe->stats, max_usecs * 100); // Time is stored in hundredth of microseconds

	((struct probe *)probe)->display = probe_timer_display;
	((struct probe *)probe)->clear = probe_timer_clear;

	return probe;
}

/**
 * Store the timer's start time
 *
 * \param probe Pointer to a timer probe
 */
void probe_timer_add_in(struct probe_timer *probe)
{
	if (!probe_active)
		return;

	((struct probe *)probe)->triggered = TRUE;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &probe->timer_in);
}

/**
 * Store the time's stop time
 *
 * Store the time's stop time, computes the time diff, and add it to the statistic.
 * The probe is set as triggered.
 *
 * \param probe Pointer to a timer probe
 */
void probe_timer_add_out(struct probe_timer *probe)
{
	struct timespec timer_out, time_diff;

	if (!probe_active)
		return;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &timer_out);
	time_diff = probe_timer_diff(&probe->timer_in, &timer_out);

	// To avoid the need to store a huge amount of values, we only keep
	// hundredth of microseconds.
	int val = time_diff.tv_nsec / 10;

	// Add the timer into the statistic and the accumulator.
	// If a new overflow is detected, report it
	if (!probe_stats_add(&probe->stats, val)) {
		DebugPrintf(-1, "PROBE error: Timer \"%s\" value is too high (%f)\n",
				    ((struct probe *)probe)->title, val / 100.0);
	}
	probe_accum_add(&probe->per_frame_accum, val);

	((struct probe *)probe)->triggered = TRUE;
}

/**
 * Clear the content of a timer probe (reset to 'zero')
 *
 * \param probe_base Pointer to the base's struct of a timer probe
 */
void probe_timer_clear(struct probe *probe_base)
{
	struct probe_timer *probe = (struct probe_timer *)probe_base;
	probe_accum_clear(&probe->per_frame_accum);
	probe_accum_clear(&probe->overall_accum);
	probe_stats_clear(&probe->stats);
	probe_base->triggered = FALSE;
}

/**
 * Display a report of a timer probe
 *
 * Display a report of a timer probe, composed of 3 components:
 * - a 1 dimensional statistic of the timer's values, since last cleared
 * - an horizontal tribar graph, with the min, mean and max values of the timer
 * - an horizontal bar graph to report the mean value of the timer (on the current frame)
 *
 * \param probe_base Pointer to the base's struct of a probe
 * \param x Left screen position of the report
 * \param y Top screen position of the report
 */
void probe_timer_display(struct probe *probe_base, int x, int y)
{
	struct probe_timer *probe = (struct probe_timer *)probe_base;

	// Mean timer value on the current frame
	int mean_time = probe_accum_get_mean(&probe->per_frame_accum);

	// If the probe was triggered during the current frame, get it's mean value,
	// and add it to the overall accumulator (in order to have the mean value since
	// the last reset).
	if (probe_base->triggered && mean_time != 0) {
		probe_accum_add(&probe->overall_accum, mean_time);
	}

	// Display the report

	probe_display_container(x, y, probe_base->report_width, probe_base->report_height, probe_base->title);
	x += 20;
	y += title_height;

	probe_display_stats(x, y, stats_width, stats1D_height, &probe->stats);
	y += stats1D_height;

	const int value_divider = 100; // Value divider set to 100 to print time in usecs

	probe_display_htribargraph(x, y, bar_width,  bar_size,
	                          probe->stats.min_value, probe_accum_get_mean(&probe->overall_accum), probe->stats.max_value,
	                          probe->stats.scale, value_divider);
	y += bar_size;

	probe_display_bargraph(x, y, bar_width, bar_size, mean_time, probe->stats.scale, value_divider);

	// Prepare the probe for the next frame's report
	probe_accum_clear(&probe->per_frame_accum);
	probe_base->triggered = FALSE;
}

/*==================== 1D graph probe related functions ====================*/

static void probe_graph1D_clear(struct probe *probe_base);
static void probe_graph1D_display(struct probe *probe_base, int x, int y);

/**
 * Create a graph1D probe
 *
 * A graph1D probe is used to report some statistics about a 1 dimensional data
 *
 * \param title Title of the probe, as displayed on the report screen
 * \param max Maximum excepted value of the data
 * \param div Divider to apply when printing the values
 * \return Pointer to the created probe
 */
struct probe_graph1D *probe_graph1D_create(char *title, int max, int div)
{
	struct probe_graph1D *probe = (struct probe_graph1D *)MyMalloc(sizeof(struct probe_graph1D));
	const int report_height = report_padding + title_height + stats1D_height + 2 * bar_size + report_padding;

	probe_register((struct probe *)probe, title, report_width, report_height);
	probe_accum_clear(&probe->accum);
	probe_stats_init(&probe->stats, max);

	probe->divider = div;

	((struct probe *)probe)->display = probe_graph1D_display;
	((struct probe *)probe)->clear = probe_graph1D_clear;

	return probe;
}

/**
 * Add a new value in the 1 dimensional graph
 *
 * A value is added into the statistic store, and is accumulated to
 * compute a mean value on all the frames since the last reset.
 * The probe is set as triggered.
 *
 * \param probe Pointer to a graph1D probe
 * \param val Value to add
 */
void probe_graph1D_add(struct probe_graph1D *probe, int val)
{
	static int negative_reported = FALSE;

	if (!probe_active)
		return;

	// The value must be positive
	if (val < 0) {
		if (!negative_reported) {
			DebugPrintf(-1, "PROBE error: Graph1D value can not be negative\n");
			negative_reported = TRUE;
		}
	}

	// Add the value into the statistic and the accumulator.
	// If a new overflow is detected, report it
	if (!probe_stats_add(&probe->stats, val)) {
		DebugPrintf(-1, "PROBE error: Graph1D \"%s\" value is too high (%d)\n",
				    ((struct probe *)probe)->title, val);
	}
	probe_accum_add(&probe->accum, val);

	((struct probe *)probe)->triggered = TRUE;
}

/**
 * Clear the content of a 1D graph probe (reset to 'zero')
 *
 * \param probe_base Pointer to the base's struct of a 1D graph probe
 */
void probe_graph1D_clear(struct probe *probe_base)
{
	struct probe_graph1D *probe = (struct probe_graph1D *)probe_base;
	probe_accum_clear(&probe->accum);
	probe_stats_clear(&probe->stats);
	probe_base->triggered = FALSE;
}

/**
 * Display a report of a 1D graph probe
 *
 * Display a report of a 1D graph probe, composed of 2 components:
 * - a 1 dimensional statistic of the values, since last cleared
 * - an horizontal tribar graph, with the min, mean and max values
 *
 * \param probe_base Pointer to the base's struct of a probe
 * \param x Left screen position of the report
 * \param y Top screen position of the report
 */
void probe_graph1D_display(struct probe *probe_base, int x, int y)
{
	struct probe_graph1D *probe = (struct probe_graph1D *)probe_base;

	// Display the report

	probe_display_container(x, y, probe_base->report_width, probe_base->report_height, probe_base->title);
	x += 20;
	y += title_height;

	probe_display_stats(x, y, stats_width, stats1D_height, &probe->stats);
	y += stats1D_height;

	probe_display_htribargraph(x, y, bar_width,  bar_size,
	                           probe->stats.min_value, probe_accum_get_mean(&probe->accum), probe->stats.max_value,
	                           probe->stats.scale, probe->divider);

	// Prepare the probe for the next frame's report
	probe_base->triggered = FALSE;
}

/*==================== 2D graph probe related functions ====================*/

static void probe_graph2D_clear(struct probe *probe_base);
static void probe_graph2D_display(struct probe *probe_base, int x, int y);

/**
 * Create a graph2D probe
 *
 * A graph2D probe is used to report some statistics about a 2 dimensional data
 * (a data pair).
 *
 * \param title Title of the probe, as displayed on the report screen
 * \param max_x Maximum excepted value of the data pair along X axis
 * \param max_y Maximum excepted value of the data pair along Y axis
 * \return Pointer to the created probe
 */
struct probe_graph2D *probe_graph2D_create(char *title, int max_x, int max_y, int div_x, int div_y)
{
	struct probe_graph2D *probe = (struct probe_graph2D *)MyMalloc(sizeof(struct probe_graph2D));
	const int report_height = report_padding + title_height + stats2D_height + bar_size + report_padding;

	probe_register((struct probe *)probe, title, report_width, report_height);
	probe_accum_clear(&probe->accum[0]);
	probe_accum_clear(&probe->accum[1]);
	probe_array_stats_init(&probe->stats, max_x, max_y);

	probe->divider[0] = div_x;
	probe->divider[1] = div_y;

	((struct probe *)probe)->display = probe_graph2D_display;
	((struct probe *)probe)->clear = probe_graph2D_clear;

	return probe;
}

/**
 * Add a new value in the 2 dimensional graph
 *
 * A value pair is added into the statistic store, and is accumulated to
 * compute a mean value on all the frames since the last reset.
 * The probe is set as triggered.
 *
 * \param probe Pointer to a graph2D probe
 * \param val_x Value on the X axis
 * \param val_y Value on the Y axis
 */
void probe_graph2D_add(struct probe_graph2D *probe, int val_x, int val_y)
{
	static int negative_reported = FALSE;

	if (!probe_active)
		return;

	// The value pair must be positive
	if (val_x < 0 || val_y < 0) {
		if (!negative_reported) {
			DebugPrintf(-1, "PROBE error: Graph2D values can not be negative\n");
			negative_reported = TRUE;
		}
	}

	// Add the pair into the statistic and the accumulator.
	// If a new overflow is detected, report it
	if (!probe_array_stats_add(&probe->stats, val_x, val_y)) {
		DebugPrintf(-1, "PROBE error: Graph2D \"%s\" value is too high (%d / %d)\n",
				    ((struct probe *)probe)->title, val_x, val_y);
	}
	probe_accum_add(&probe->accum[0], val_x);
	probe_accum_add(&probe->accum[1], val_y);

	((struct probe *)probe)->triggered = TRUE;
}

/**
 * Clear the content of a 2D graph probe (reset to 'zero')
 *
 * \param probe_base Pointer to the base's struct of a 2D graph probe
 */
void probe_graph2D_clear(struct probe *probe_base)
{
	struct probe_graph2D *probe = (struct probe_graph2D *)probe_base;
	probe_accum_clear(&probe->accum[0]);
	probe_accum_clear(&probe->accum[1]);
	probe_array_stats_clear(&probe->stats);
	probe_base->triggered = FALSE;
}

/**
 * Display a report of a 2D graph probe
 *
 * Display a report of a 2D graph probe, composed of 3 components:
 * - a 2 dimensional statistic of the value pairs, since last cleared
 * - an horizontal tribar graph, with the min, mean and max values on the X axis
 * - a vertical tribar graph, with the min, mean and max values on the Y axis
 *
 * \param probe_base Pointer to the base's struct of a probe
 * \param x Pointer to the left screen position of the report
 * \param y Pointer to the right screen position of the report
 */
void probe_graph2D_display(struct probe *probe_base, int x, int y)
{
	struct probe_graph2D *probe = (struct probe_graph2D *)probe_base;

	// Display the report

	probe_display_container(x, y, probe_base->report_width, probe_base->report_height, probe_base->title);
	x += 20;
	y += title_height;

	probe_display_array_stats(x + bar_size, y, stats_width, stats2D_height, &probe->stats);

	probe_display_vtribargraph(x, y, bar_size,  stats2D_height,
	                           probe->stats.min_value[1], probe_accum_get_mean(&probe->accum[1]), probe->stats.max_value[1],
	                           probe->stats.scale[1], probe->divider[1]);
	y += stats2D_height;

	probe_display_htribargraph(x + bar_size, y, stats_width,  bar_size,
	                           probe->stats.min_value[0], probe_accum_get_mean(&probe->accum[0]), probe->stats.max_value[0],
	                           probe->stats.scale[0], probe->divider[0]);

	// Prepare the probe for the next frame's report
	probe_base->triggered = FALSE;
}

/*==================== External API ====================*/

/**
 * Activate/De-activate the real-time profiler system
 * \ingroup rtprof
 */
void rtprof_switch_activation()
{
	probe_active = !probe_active;
}

/**
 * Clear all probes' content
 * \ingroup rtprof
 */
void rtprof_clear_probes()
{
	struct probe *probe;
	list_for_each_entry(probe, &probe_list, node) {
		if (probe->clear) {
			probe->clear(probe);
		}
	}
}

/**
 * Display the profiler report screen
 * \ingroup rtprof
 *
 * If the real-time profiler is active, fill the screen with the graph of each
 * probe, from top to bottom, and from left to right.
 */
void rtprof_display()
{
	const int start_x = 50; // X screen position of the first report
	const int start_y = 50; // Y screen position of the first report
	const int margin = 20; // Margin between probe's report
	static int error_reported = FALSE;
	struct probe *probe;

	int x = start_x;
	int y = start_y;
	int max_y = start_y;

	// Only works in opengl mode
	if (!use_open_gl)
		return;

	// Nothing to display if the profiler is de-activated
	if (!probe_active)
		return;

	// Display each probe's report
	list_for_each_entry(probe, &probe_list, node) {
		if (!probe->display)
			continue;

		// If screen's width is exceeded, start on a new line
		if (x + probe->report_width > GameConfig.screen_width) {
			x = start_x;
			y = max_y;
			// If screen's height is exceeded, report the error (only once) and stop
			if (y > GameConfig.screen_height) {
				if (!error_reported) {
					DebugPrintf(-1, "PROBE error: Screen is too small to report all probes\n");
					error_reported = TRUE;
				}
				break;
			}
		}

		probe->display(probe, x, y);

		x += probe->report_width + margin;
		max_y = max(max_y, y + probe->report_height + margin);
	}
}

#endif // WITH_RTPROF

#undef _probe_c
