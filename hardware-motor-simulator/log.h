/*
 * Defines the log
 */

/*
 * NOTE: code assume that LOG_SIZE+1 fits in an 8-bit integer
 */
#define	LOG_SIZE	150	// no more than 150 entries are stored
#define	LOG_MAX_NORMAL	LOG_SIZE - 20
#define	LOG_MAX_DETAIL	LOG_MAX_NORMAL - 20

// Log time is managed to ensure that timestamps can always fit in 16 bits
#define	LOG_ROLLOVER_TIME	10000	// after this many milliseconds, rebase.

extern bool log_enabled;	// set for full runs, clear for tests and such

struct log_entry_s {
	unsigned char log_op;		// see various #defines
	unsigned char log_param;	// some op codes take a numeric parameter
	unsigned int timestamp;	// Time when the logged event happened.
};

/*
 * Notes on log time stamps
 *
 * The first log entry always happens at time t=0.
 * All other log events happen at some number of milliseconds since t=0.
 * If the log runs more than 65 seconds, a log rollover event is recorded.
 */

/*
 * Log Levels:
 * 	Three levels of logging allows for users to see only the
 * 	important subsets.  Also, last few entries in the log are
 * 	reserved for important log entries.
 */

#define	LOG_LEVEL_SHIFT	6
#define	LOG_CRITICAL	(0 << LOG_LEVEL_SHIFT)
#define	LOG_NORMAL	(1 << LOG_LEVEL_SHIFT)
#define	LOG_DETAIL	(2 << LOG_LEVEL_SHIFT)
#define	LOG_LEVEL_MASK	(3 << LOG_LEVEL_SHIFT)

#define	LOG_LEVEL(op)	((unsigned)((op) & LOG_LEVEL_MASK))

/*
 * Log Op Codes
 * 	NOTE: Codes must be consecutive integers
 * These are the events that can be logged.
 */

#define	LOG_START		( 0 | LOG_CRITICAL)	// always the first log entry.  Entered automatically, not by caller.
#define	LOG_IG_IPA_OPEN		( 1 | LOG_CRITICAL)
#define	LOG_IG_IPA_CLOSE	( 2 | LOG_NORMAL)
#define	LOG_IG_N2O_OPEN		( 3 | LOG_CRITICAL)
#define	LOG_IG_N2O_CLOSE	( 4 | LOG_NORMAL)
#define	LOG_SPARK_FIRST		( 5 | LOG_CRITICAL)
#define	LOG_SPARK_LAST		( 6 | LOG_NORMAL)
#define	LOG_IG_PRESSURE_GOOD_1	( 7 | LOG_CRITICAL)	// first time ig pressure is good
#define	LOG_IG_PRESSURE_GOOD	( 8 | LOG_NORMAL)		// second thru n'th time ig pressure is good
#define	LOG_IG_PRESSURE_CHANGE	( 9 | LOG_DETAIL)		// any ig pressure change.  Param is 8 msb of pressure
#define	LOG_MAIN_N2O_CHANGE	(10 | LOG_CRITICAL)	// N2O valve commanded to move; param is 8 msb of servo commanded position
#define	LOG_MAIN_IPA_CHANGE	(11 | LOG_CRITICAL)
#define	LOG_MAIN_DONE		(12 | LOG_CRITICAL)	// simulation done
#define	LOG_MAIN_PCT		(13 | LOG_NORMAL)	// pct of full chamber pressure
#define LOG_FUEL_GONE		(14 | LOG_CRITICAL)
#define	LOG_TIME_ROLLOVER	(15 | LOG_CRITICAL)

/*
 * Entry points into log.cpp
 */

void log_init();
void log_commit();
void log_reset();
void log(unsigned char op, unsigned char param);
char *log_tos_seqn();
char *log_tos_short(unsigned char entry);
char *log_tos_long(unsigned char entry);
