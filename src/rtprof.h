/*
 *
 *   Copyright (c) 2011,2014 Samuel Degrande
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
 * \file probe.h
 * \brief Real-time in-game profiler's definitions
 */

#ifndef _PROBE_H_
#define _PROBE_H_

//#include "system.h"
//#include "struct.h"
//#include "lists.h"
#include <time.h>
#include "config.h"

#ifdef WITH_RTPROF

/// \defgroup rtprof Real-time in-game profiler
///
/// Probes can be placed inside the code, in order to retrieve and display
/// counters, execution times or statistics on any variables.\n
/// This is a developer tool.\n
/// In order to enable its compilation, run configure with --enable-rtprof.
///
/// In game, and if probes were added in the code, use LAlt-p to switch on/off
/// the profiler, and LAlt-o to clear the graphs.

/**
 * Base 'class' of all probes
 *
 * Inherited by actual probe definitions.
 */
struct probe {
	char *title;                                                //!< Displayed on the profiler report screen
	int triggered;                                              //!< Set each time the probe is triggered during execution
	int report_width;                                           //!< Report window's width
	int report_height;                                          //!< Report window's height
	void (*display)(struct probe *this_probe, int x, int y);    //!< virtual display function
	void (*clear)(struct probe *this_probe);                    //!< virtual clear function
	struct list_head node;                                      //!< linked list
};

/**
 * Used to accumulate a data's value
 */
struct probe_accum {
	long val;    //!< Accumulator
	int cntr;    //!< Number of accumulated values
};

/**
 * Used to create a statistic graph of a data
 */
struct probe_stats {
	int *store;           //!< Pointer to an array containing the number of occurrences of each value of the data
	int max_expected;     //!< Expected maximum value of the data (and so size of the store)
	int min_value;        //!< Minimum encountered value of the data
	int max_value;        //!< Maximum encountered value of the data (less than max_expected)
	int overflow_value;   //!< Highest value of the data, if the data overflows what was expected (max_expected)
	int overflow_cntr;    //!< Number of occurrences of overflows
	float scale;          //!< Scale needed to fit max_value to graph's width (computed during graph display)
};

/**
 * Used to create a statistic graph of a 2 dimensional data
 */
struct probe_array_stats {
	int *store;             //!< Pointer to a 2D array containing the number of occurrences of each value of the data
	int max_expected[2];    //!< Expected maximum value of the data on each axis (and so size of the store)
	int min_value[2];       //!< Minimum encountered value of the data on each axis
	int max_value[2];       //!< Maximum encountered value of the data on each axis (less than max_expected)
	int overflow_value[2];  //!< Highest value of the data on each axis, if the data overflows what was expected (max_expected)
	int overflow_cntr[2];   //!< Number of occurrences of overflows
	float scale[2];         //!< Scale needed to fit max_value on each axis to graph's size (computed during graph display)
};

/// \defgroup counter_probe Counter probe
/// \ingroup rtprof
///
/// A counter probe accumulates a value each time the probe is triggered, during one
/// display frame.\n
/// Statistics of the counter's accumulated values is generated.
///
/// Usage example:
/// \code
/// #include "probe.h"
///
/// void foo()
/// {
///   int val = some_value;
///   probe_counter_set(my_probe, "My probe", 1000, val);
///   ...
/// }
/// \endcode
/// Each time foo() is called, \a val is added to the accumulated value.
/// The maximum expected accumulated value is \a 1000.\n
/// If, during one frame, the accumulated value exceeds \a 1000, an overflow is
/// denoted on the report, and in the console.\n
/// The accumulated value is reseted at each frame.
///
/// Result:
/// \image html rtprof_counter.png "Graph of a counter probe"

/**
 * Counter probe structure
 */
struct probe_counter {
	struct probe base;         //!< Inheritance
	int counter;               //!< Current counter's value
	struct probe_stats stats;  //!< Statistics of counter's values
	struct probe_accum accum;  //!< Accumulate the counter's values at the end of each frame, to compute the counter's mean value
};

struct probe_counter *probe_counter_create(char *title, int max);
void probe_counter_add(struct probe_counter *probe, int value);

/**
 * Set a counter probe
 * \ingroup counter_probe
 *
 * \param ref Probe's id
 * \param title Probe's title as displayed on the profiler report screen
 * \param max Maximum expected value of the counter
 * \param value Value to add to the counter each time the probe is triggered
 */
#define probe_counter_set(ref, title, max, value) \
	static struct probe_counter *probe_##ref = NULL; \
	if (!probe_##ref) probe_##ref = probe_counter_create(title, max); \
	probe_counter_add(probe_##ref, value)


/// \defgroup timer_probe Timer probe
/// \ingroup rtprof
///
/// Used to generate a statistics of the execution time of a code.
///
/// Usage example:
/// \code
/// #include "probe.h"
///
/// void foo()
/// {
///   probe_timer_set_in(my_probe, "My probe", 50);
///   ...
///   if (flag) {
///     probe_timer_set_out(my_probe);
///     return;
///   }
///   ...
///   probe_timer_set_out(my_probe);
///   return;
/// }
/// \endcode
/// In order to measure the elapsed time of the overall function,
/// \a probe_timer_out \b must be called before all return statements.
///
/// Result:
/// \image html rtprof_timer.png "Graph of a timer probe"

/**
 * Timer probe structure
 */
struct probe_timer {
	struct probe base;                  //!< Inheritance
	struct timespec timer_in;           //!< Starting time of the code
	struct probe_stats stats;           //!< Execution time statistics
	struct probe_accum per_frame_accum; //!< Accumulate all execution times during one frame. Used to compute the mean execution time during one frame
	struct probe_accum overall_accum;   //!< Accumulate all execution times since last profiler reset. Used to compute the overall mean execution time
};

struct probe_timer *probe_timer_create(char *title, int max_usecs);
void probe_timer_add_in(struct probe_timer *probe);
void probe_timer_add_out(struct probe_timer *probe);

/**
 * Set the in-point of a timer probe
 * \ingroup timer_probe
 *
 * Used to record the starting time. Must be put at the beginning of the profiled code.
 * \param ref Probe's id
 * \param title Probe's title as displayed on the profiler report screen
 * \param max_usecs Maximum expected value of the execution time (in microseconds)
 */
#define probe_timer_set_in(ref, title, max_usecs) \
	static struct probe_timer *probe_##ref = NULL; \
	if (!probe_##ref) probe_##ref = probe_timer_create(title, max_usecs); \
	probe_timer_add_in(probe_##ref)

/**
 * Set the out-point of the probe
 * \ingroup timer_probe
 *
 * Time difference between the in-point and the out-point defines the execution time.
 * Must be put at the end of the profiled code.
 * \param ref Probe's id (related to the ref defined with PROBE_timer_in)
 */
#define probe_timer_set_out(ref) \
	probe_timer_add_out(probe_##ref)


/// \defgroup onedgraph_probe 1 dimensional statistics probe
/// \ingroup rtprof
///
/// When the probe is triggered, a value is recorded, and a statistics graph of the value
/// is reported.\n
/// The recorded value must be a positive integral value.\n
/// To record a floating value:\n
/// - apply a scale to convert to an integral value, and use the scaled value in the probe's call
/// - define a divider that will be applied to any displayed value on the report screen
///
/// Usage example:
/// \code
/// #include "probe.h"
///
/// void foo()
/// {
///   float fval = some_value;
///   probe_graph1D_set(my_probe, "My probe", 500, 100, (int)(fval * 100.0));
/// }
/// \endcode
/// Since \a probe_graph1D can only record integral values, (fval * 100) is
/// recorded instead of \a fval. Here the excepted maximal value of \a fval is thus
/// 500/100 = 5.0.
///
/// Result:
/// \image html rtprof_1D_stats.png "Graph of a 1D-stats probe"

/**
 * A 1 dimensional statistics probe structure
 */
struct probe_graph1D
{
	struct probe base;           //!< Inheritance
	struct probe_stats stats;    //!< Statistics of the probed values
	struct probe_accum accum;    //!< Accumulate the values, to compute the mean value
	int divider;                 //!< Divider to apply when printing values
};

struct probe_graph1D *probe_graph1D_create(char *title, int max, int div);
void probe_graph1D_add(struct probe_graph1D *probe, int val);

/**
 * Set a 1 dimensional statistics probe
 * \ingroup onedgraph_probe
 *
 * Internally creates an array of 'max' values. So 'max' should be
 * kept as low as possible.
 * \param ref Probe's id
 * \param title Probe's title as displayed on the profiler report screen
 * \param max Maximum expected value of the data
 * \param div Divider to apply to a value when displaying the profiler report
 * \param val Value to record when the probe is triggered
 */
#define probe_graph1D_set(ref, title, max, div, val) \
	static struct probe_graph1D *probe_##ref = NULL; \
	if (!probe_##ref) probe_##ref = probe_graph1D_create(title, max, div); \
	probe_graph1D_add(probe_##ref, val)


/// \defgroup twodgraph_probe 2 dimensional statistics probe
/// \ingroup rtprof
///
/// When the probe is triggered, a 2 dimensional value is recorded, and a statistics graph of the value
/// is reported, using a 12 color scale (blue to red).\n
/// The recorded values must be integral values.\n
/// To record floating values:
/// - apply a scale to convert to integral values, and use the scaled values in the probe's call
/// - define a divider that will be applied to any displayed value on the report screen
///
/// Usage example:
/// \code
/// #include "probe.h"
///
/// void foo()
/// {
///   int xval = some_value;
///   int yval = some_other_value;
///   probe_graph2D_set(my_probe, "My probe", 100, 100, 1, 1, xval, yval);
/// }
/// \endcode
/// See the example of the 1D-stats probe, for comment on the use of floating values.
///
/// Result:
/// \image html rtprof_2D_stats.png "Graph of a 2D-stats probe"

/**
 * A 2 dimensional statistics probe structure
 */
struct probe_graph2D
{
	struct probe base;                 //!< Inheritance
	struct probe_array_stats stats;    //!< Statistics of the probed values
	struct probe_accum accum[2];       //!< Accumulate the values on each axis, to compute the mean values
	int divider[2];                    //!< Divider to apply when printing values
};

struct probe_graph2D *probe_graph2D_create(char *title, int max_x, int max_y, int div_x, int div_y);
void probe_graph2D_add(struct probe_graph2D *probe, int val_x, int val_y);

/**
 * Set a 2 dimensional statistics probe
 * \ingroup twodgraph_probe
 *
 * Internally creates an array of 'max_x' * 'max_y' values. So 'max_x' and
 * 'max_y' should be kept as low as possible.
 * \param ref Probe's id
 * \param title Probe's title as displayed on the profiler report screen
 * \param max_x Maximum expected value of the data, along X axis
 * \param max_y Maximum expected value of the data, along Y axis
 * \param div_x Divider to apply to a value on X axis when displaying the profiler report
 * \param div_y Divider to apply to a value on Y axis when displaying the profiler report
 * \param val_x X value to record when the probe is triggered
 * \param val_y Y value to record when the probe is triggered
 */
#define probe_graph2D_set(ref, title, max_x, max_y, div_x, div_y, val_x, val_y) \
	static struct probe_graph2D *probe_##ref = NULL; \
	if (!probe_##ref) probe_##ref = probe_graph2D_create(title, max_x, max_y, div_x, div_y); \
	probe_graph2D_add(probe_##ref, val_x, val_y)

#endif // WITH_RTPROF

#endif // _PROBE_H_
