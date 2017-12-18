#ifndef TM_H
#define TM_H 1

#  include <stdio.h>


#ifndef REDUCED_TM_API

#  define MAIN(argc, argv)              int main (int argc, char** argv)
#  define MAIN_RETURN(val)              return val

#  define GOTO_SIM()                    /* nothing */
#  define GOTO_REAL()                   /* nothing */
#  define IS_IN_SIM()                   (0)

#  define SIM_GET_NUM_CPU(var)          /* nothing */

#  define TM_PRINTF                     printf
#  define TM_PRINT0                     printf
#  define TM_PRINT1                     printf
#  define TM_PRINT2                     printf
#  define TM_PRINT3                     printf

#  define P_MEMORY_STARTUP(numThread)   /* nothing */
#  define P_MEMORY_SHUTDOWN()           /* nothing */

#  include <assert.h>
#  include "memory.h"
#  include "thread.h"
#  include "types.h"
#  include <math.h>

#  define TM_ARG                        /* nothing */
#  define TM_ARG_ALONE                  /* nothing */
#  define TM_ARGDECL                    /* nothing */
#  define TM_ARGDECL_ALONE              /* nothing */
#  define TM_CALLABLE                   /* nothing */

#  define TM_BEGIN_WAIVER()
#  define TM_END_WAIVER()

#  define P_MALLOC(size)                malloc(size)
#  define P_FREE(ptr)                   free(ptr)
#  define TM_MALLOC(size)               malloc(size)
#  define FAST_PATH_FREE(ptr)           free(ptr)
#  define SLOW_PATH_FREE(ptr)           free(ptr)

# define SETUP_NUMBER_TASKS(n)
# define SETUP_NUMBER_THREADS(n)
# define PRINT_STATS()
# define AL_LOCK(idx)

#endif

#include <asm/unistd.h>
#define rmb()           asm volatile ("sync" ::: "memory")
#define cpu_relax()     asm volatile ("" ::: "memory");
//#define cpu_relax() asm volatile ("or 31,31,31")
#ifdef REDUCED_TM_API
#    define SPECIAL_THREAD_ID()         get_tid()
#else
#    define SPECIAL_THREAD_ID()         thread_getId()
#endif


#include <htmxlintrin.h>

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_self_conflict(void* const TM_buff)
{
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_SELF_INDUCED_CONFLICT (texasr);
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_trans_conflict(void* const TM_buff)
{
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_TRANSACTION_CONFLICT (texasr);
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_nontrans_conflict(void* const TM_buff)
{
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_NON_TRANSACTIONAL_CONFLICT (texasr);
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_persistent_abort(void* const TM_buff)
{
  texasr_t texasr = *_TEXASR_PTR (TM_buff);
  return _TEXASR_FAILURE_PERSISTENT (texasr);
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_conflict(void* const TM_buff)
{
  texasr_t texasr = *_TEXASR_PTR (TM_buff);
  /* Return TEXASR bits 11 (Self-Induced Conflict) through
     14 (Translation Invalidation Conflict).  */
  return (_TEXASR_EXTRACT_BITS (texasr, 14, 4)) ? 1 : 0;
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_user_abort (void* const TM_buff)
{
  texasr_t texasr = *_TEXASR_PTR (TM_buff);
  return _TEXASR_ABORT (texasr);
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_capacity_abort (void* const TM_buff)
{
  texasr_t texasr = *_TEXASR_PTR (TM_buff);
  return _TEXASR_FOOTPRINT_OVERFLOW (texasr);
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_begin_rot (void* const TM_buff)
{
  *_TEXASRL_PTR (TM_buff) = 0;
  if (__builtin_expect (__builtin_tbegin (1), 1)){
    return _HTM_TBEGIN_STARTED;
  }
#ifdef __powerpc64__
  *_TEXASR_PTR (TM_buff) = __builtin_get_texasr ();
#else
  *_TEXASRU_PTR (TM_buff) = __builtin_get_texasru ();
  *_TEXASRL_PTR (TM_buff) = __builtin_get_texasr ();
#endif
  *_TFIAR_PTR (TM_buff) = __builtin_get_tfiar ();
  return 0;
}

#  define TM_STARTUP(numThread, bId) running = 1; hc_start();
#  define TM_SHUTDOWN(){ \
    running = 0; \
    unsigned long wait_time = 0; \
    unsigned long total_time = 0; \
    unsigned long read_commits = 0; \
    unsigned long htm_commits = 0; \
    unsigned long htm_conflict_aborts = 0; \
    unsigned long htm_user_aborts = 0; \
    unsigned long htm_self_conflicts = 0; \
    unsigned long htm_trans_conflicts = 0; \
    unsigned long htm_nontrans_conflicts = 0; \
    unsigned long htm_persistent_aborts = 0; \
    unsigned long htm_capacity_aborts = 0; \
    unsigned long htm_other_aborts = 0; \
    unsigned long rot_commits = 0; \
    unsigned long rot_conflict_aborts = 0; \
    unsigned long rot_user_aborts = 0; \
    unsigned long rot_self_conflicts = 0; \
    unsigned long rot_trans_conflicts = 0; \
    unsigned long rot_nontrans_conflicts = 0; \
    unsigned long rot_persistent_aborts = 0; \
    unsigned long rot_capacity_aborts = 0; \
    unsigned long rot_other_aborts = 0; \
    unsigned long gl_commits = 0; \
    int i = 0; \
    for (; i < 80; i++) { \
       wait_time += stats_array[i].wait_time; \
       total_time += stats_array[i].total_time; \
       read_commits += stats_array[i].read_commits; \
       htm_commits += stats_array[i].htm_commits; \
       htm_conflict_aborts += stats_array[i].htm_conflict_aborts; \
       htm_user_aborts += stats_array[i].htm_user_aborts; \
       htm_self_conflicts += stats_array[i].htm_self_conflicts; \
       htm_trans_conflicts += stats_array[i].htm_trans_conflicts; \
       htm_nontrans_conflicts += stats_array[i].htm_nontrans_conflicts; \
       htm_persistent_aborts += stats_array[i].htm_persistent_aborts; \
       htm_capacity_aborts += stats_array[i].htm_capacity_aborts; \
       htm_other_aborts += stats_array[i].htm_other_aborts; \
       rot_commits += stats_array[i].rot_commits; \
       rot_conflict_aborts += stats_array[i].rot_conflict_aborts; \
       rot_user_aborts += stats_array[i].rot_user_aborts; \
       rot_self_conflicts += stats_array[i].rot_self_conflicts; \
       rot_trans_conflicts += stats_array[i].rot_trans_conflicts; \
       rot_nontrans_conflicts += stats_array[i].rot_nontrans_conflicts; \
       rot_persistent_aborts += stats_array[i].rot_persistent_aborts; \
       rot_capacity_aborts += stats_array[i].rot_capacity_aborts; \
       rot_other_aborts += stats_array[i].rot_other_aborts; \
       gl_commits += stats_array[i].gl_commits; \
    } \
    printf("Total time: %lu\n \
    Total wait time: %lu\n \
    Total commits: %lu\n \
       \tRead commits: %lu\n \
       \tHTM commits:  %lu\n \
       \tROT commits:  %lu\n \
       \tGL commits: %lu\n \
    Total aborts: %lu\n \
       \tHTM conflict aborts:  %lu\n \
          \t\tHTM self aborts:  %lu\n \
          \t\tHTM trans aborts:  %lu\n \
          \t\tHTM non-trans aborts:  %lu\n \
       \tHTM user aborts :  %lu\n \
       \tHTM capacity aborts:  %lu\n \
          \t\tHTM persistent aborts:  %lu\n \
       \tHTM other aborts:  %lu\n \
       \tROT conflict aborts:  %lu\n \
          \t\tROT self aborts:  %lu\n \
          \t\tROT trans aborts:  %lu\n \
          \t\tROT non-trans aborts:  %lu\n \
       \tROT user aborts:  %lu\n \
       \tROT capacity aborts:  %lu\n \
          \t\tROT persistent aborts:  %lu\n \
       \tROT other aborts:  %lu\n", total_time, wait_time, read_commits+htm_commits+rot_commits+gl_commits, read_commits, htm_commits, rot_commits, gl_commits,htm_conflict_aborts+htm_user_aborts+htm_capacity_aborts+htm_other_aborts+rot_conflict_aborts+rot_user_aborts+rot_capacity_aborts+rot_other_aborts,htm_conflict_aborts,htm_self_conflicts,htm_trans_conflicts,htm_nontrans_conflicts,htm_user_aborts,htm_capacity_aborts,htm_persistent_aborts,htm_other_aborts,rot_conflict_aborts,rot_self_conflicts,rot_trans_conflicts,rot_nontrans_conflicts,rot_user_aborts,rot_capacity_aborts,rot_persistent_aborts,rot_other_aborts); \
} \

#  define TM_THREAD_ENTER()
#  define TM_THREAD_EXIT()

# define IS_LOCKED(lock)        *((volatile int*)(&lock)) != 0

# define IS_GLOBAL_LOCKED(lock)        *((volatile int*)(&lock)) == 2

# define TM_BEGIN(ro) TM_BEGIN_EXT(0,ro)

# define READ_TIMESTAMP(dest) __asm__ volatile("0:                  \n\tmfspr   %0,268           \n": "=r"(dest));

#define INACTIVE 0

# define USE_ROT(){ \
	int rot_budget = ROT_RETRIES; \
	while(IS_LOCKED(single_global_lock)){ \
        	cpu_relax(); \
        } \
	while(rot_budget > 0){ \
		rot_status = 1; \
		TM_buff_type TM_buff; \
                long start_time; \
		READ_TIMESTAMP(start_time); \
                counters[local_thread_id].value = start_time + tx_length[b_type].value; \
		rmb(); \
		if(IS_LOCKED(single_global_lock)){ \
			counters[local_thread_id].value = INACTIVE; \
			rmb(); \
			while(IS_LOCKED(single_global_lock)) cpu_relax(); \
			continue; \
		} \
		unsigned char tx_status = __TM_begin_rot(&TM_buff); \
		if (tx_status == _HTM_TBEGIN_STARTED) { \
                        triggers[local_thread_id].value = 1; \
                        break; \
                } \
		else if(__TM_conflict(&TM_buff)){ \
                        stats_array[local_thread_id].rot_conflict_aborts ++; \
			if(__TM_is_self_conflict(&TM_buff)) stats_array[local_thread_id].rot_self_conflicts++; \
			else if(__TM_is_trans_conflict(&TM_buff)) stats_array[local_thread_id].rot_trans_conflicts++; \
			else if(__TM_is_nontrans_conflict(&TM_buff)) stats_array[local_thread_id].rot_nontrans_conflicts++; \
                        rot_status = 0; \
                        rot_budget--; \
			counters[local_thread_id].value = INACTIVE; \
                	rmb(); \
                } \
                else if (__TM_user_abort(&TM_buff)) { \
                        stats_array[local_thread_id].rot_user_aborts ++; \
                        rot_status = 0; \
                        rot_budget--; \
                } \
                else if(__TM_capacity_abort(&TM_buff)){ \
			rot_status = 0; \
			stats_array[local_thread_id].rot_capacity_aborts ++; \
			if(__TM_is_persistent_abort(&TM_buff)) stats_array[local_thread_id].rot_persistent_aborts ++; \
                        break; \
		} \
                else{ \
			rot_status = 0; \
                        rot_budget--; \
			stats_array[local_thread_id].rot_other_aborts ++; \
		} \
	} \
};

# define ACQUIRE_GLOBAL_LOCK(){ \
	counters[local_thread_id].value = INACTIVE; \
        rmb(); \
	while (pthread_spin_trylock(&single_global_lock) != 0) { \
                    __asm volatile ("" : : : "memory"); \
        } \
	QUIESCENCE_CALL_GL(); \
};

# define ACQUIRE_WRITE_LOCK() { \
	local_exec_mode = 1; \
	int rot_status = 0; \
	USE_ROT(); \
	if(!rot_status){ \
		local_exec_mode = 2; \
		ACQUIRE_GLOBAL_LOCK(); \
	} \
};\

# define QUIESCENCE_CALL_ROT(){ \
 	long num_threads = global_numThread; \
	long index;\
	int state;\
	volatile long temp; \
	long counters_snapshot[80]={0}; \
        long actions[81][81]={0}; \
        long toSave[81]={0}; \
        long x = 0; \
        long ignored = 0; \
        long start_wait_time; \
        READ_TIMESTAMP(start_wait_time); \
	for(index=0; index < num_threads; index++){ \
            if(counters[index].value != INACTIVE) { \
                long wait_needed = counters[index].value - end_time; \
                if(wait_needed > 0) { \
                    x = wait_needed/alpha; \
                    if(x > num_threads - 1){ \
                        actions[num_threads][toSave[num_threads]++] = index; \
                    } else { \
                        actions[x][toSave[x]++] = index; \
                        counters_snapshot[index] = counters[index].value; \
                    } \
                } else { \
                    actions[0][toSave[0]++] = index; \
                } \
            } else { \
                ignored++; \
            } \
        } \
        long canSave = 0; \
        long acc = 0; \
	for(index=0; index < num_threads; index++){ \
            acc = acc + toSave[index]; \
            if(acc >= index) \
                canSave = index; \
            if(num_threads - 1 - ignored - acc - toSave[num_threads] < index){ \
                break; \
            } \
        } \
	for(index=num_threads-1; index >= 0; index--){ \
            if(index > canSave){ \
                for(x=0; x < toSave[index]; x++){ \
                    void* temp = triggers[actions[index][x]].value; /*kill the transaction */ \
                } \
            } else { \
                for(x=0; x < toSave[index]; x++){ \
/*printf("commits %d index %d x %d tosave %d aa %d\n",stats_array[local_thread_id].rot_commits,index, x, toSave[index], actions[index][x]);   */                 while(counters_snapshot[actions[index][x]] == counters[actions[index][x]].value){ \
                        cpu_relax(); \
                    } \
                } \
            } \
        } \
        long end_wait_time; \
       	READ_TIMESTAMP(end_wait_time); \
       	stats_array[local_thread_id].wait_time += end_wait_time - start_wait_time; \
};

# define QUIESCENCE_CALL_GL(){ \
        for(int index=0; index < global_numThread; index++){ \
            while(counters[index].value){ \
                cpu_relax(); \
            } \
        } \
};

# define RELEASE_WRITE_LOCK(){ \
	if(local_exec_mode == 1){ \
	        __TM_suspend(); \
                counters[local_thread_id].value = INACTIVE; \
		long end_time; \
                READ_TIMESTAMP(end_time); \
                if(local_thread_id == 0){ \
                    tx_length[b_type].value = tx_length[b_type].value*0.8 + (counters[local_thread_id].value - tx_length[b_type].value - end_time)*0.2; \
                } \
                QUIESCENCE_CALL_ROT(); \
	        __TM_resume(); \
		__TM_end(); \
		/*printf("thread %d committed in ROT with counters %lu and rot_counters %d\n",local_thread_id,counters[local_thread_id].value,rot_counters[local_thread_id].value); */\
                if(ro) stats_array[local_thread_id].read_commits++; \
		else stats_array[local_thread_id].rot_commits++; \
	} \
	else{ \
		pthread_spin_unlock(&single_global_lock); \
		stats_array[local_thread_id].gl_commits++; \
		/*printf("thread %d committed in GL with counters %lu\n",local_thread_id,counters[local_thread_id].value); */\
	} \
	/*printf("thread %d committed\n",local_thread_id); */\
};

# define TM_BEGIN_EXT(b,ro) {  \
	local_exec_mode = 0; \
        b_type = b; \
	local_thread_id = SPECIAL_THREAD_ID();\
	ACQUIRE_WRITE_LOCK(); \
}

# define TM_END(){ \
	RELEASE_WRITE_LOCK(); \
};

#    define TM_BEGIN_RO()                 TM_BEGIN(1)
#    define TM_RESTART()                  __TM_abort();
#    define TM_EARLY_RELEASE(var)

# define FAST_PATH_RESTART() __TM_abort();

#define SLOW_PATH_SHARED_READ(var)             var;
#define SLOW_PATH_SHARED_READ_P(var)           var;
#define SLOW_PATH_SHARED_READ_D(var)           var;

#define FAST_PATH_SHARED_READ(var)                 var
#define FAST_PATH_SHARED_READ_P(var)               var
#define FAST_PATH_SHARED_READ_D(var)               var

# define FAST_PATH_SHARED_WRITE(var, val) ({var = val; var;})
# define FAST_PATH_SHARED_WRITE_P(var, val) ({var = val; var;})
# define FAST_PATH_SHARED_WRITE_D(var, val) ({var = val; var;})

# define SLOW_PATH_RESTART() FAST_PATH_RESTART()
# define SLOW_PATH_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define SLOW_PATH_SHARED_WRITE_P(var, val)   FAST_PATH_SHARED_WRITE_P(var, val)
# define SLOW_PATH_SHARED_WRITE_D(var, val)   FAST_PATH_SHARED_WRITE_D(var, val)


#  define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_D(var, val)    ({var = val; var;})


#endif