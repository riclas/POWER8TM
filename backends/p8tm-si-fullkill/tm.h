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
//  texasr_t texasr = *_TEXASR_PTR (TM_buff);
  texasr_t texasr = __builtin_get_texasr ();
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

#  define TM_STARTUP(numThread, bId) /*running = 1; hc_start();*/
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
	for(int i=0; i < 6; i++) \
		printf("type %d length %d\n",i,tx_length[i]); \
} \

#  define TM_THREAD_ENTER()
#  define TM_THREAD_EXIT()

# define IS_LOCKED(lock)        *((volatile long*)(&lock)) != 0

# define IS_GLOBAL_LOCKED(lock)        *((volatile long*)(&lock)) == 1

# define TM_BEGIN(ro) TM_BEGIN_EXT(0,ro)

# define READ_TIMESTAMP(dest) __asm__ volatile("0:                  \n\tmfspr   %0,268           \n": "=r"(dest));

# define FINISHED 0

# define INACTIVE -1

# define USE_ROT(){ \
	int rot_budget = ROT_RETRIES; \
	while(IS_LOCKED(single_global_lock)){ \
        	cpu_relax(); \
        } \
	while(rot_budget > 0){ \
		rot_status = 1; \
		TM_buff_type TM_buff; \
/*		memset(counters_snapshot, 0, sizeof(counters_snapshot)); \
                memset(actions, 0, sizeof(actions)); \
                memset(to_save, 0, sizeof(to_save)); */\
                padded_scalar start_time; \
                READ_TIMESTAMP(start_time.value); \
                counters[local_thread_id].value = start_time.value + tx_length[b_type].value; \
		memset(counters_snapshot, 0, sizeof(counters_snapshot)); \
                memset(actions, 0, sizeof(actions)); \
                memset(to_save, 0, sizeof(to_save)); \
/*printf("tid %d b_type %d length %d\n",local_thread_id, b_type, tx_length[b_type].value);*/ \
		rmb(); \
		if(IS_LOCKED(single_global_lock)){ \
			counters[local_thread_id].value = FINISHED; \
			rmb(); \
			while(IS_LOCKED(single_global_lock)) cpu_relax(); \
			continue; \
		} \
		unsigned char tx_status = __TM_begin_rot(&TM_buff); \
		if (tx_status == _HTM_TBEGIN_STARTED) { \
                        if(/*b_type > 2 && */rot_budget > 2) \
			triggers[local_thread_id].value = 1; \
                        break; \
                } \
		else if(__TM_conflict(&TM_buff)){ \
                        stats_array[local_thread_id].rot_conflict_aborts++; \
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
/*		printf("%x\n", __builtin_get_texasr());*/ \
	} \
};

# define ACQUIRE_GLOBAL_LOCK(){ \
	counters[local_thread_id].value = FINISHED; \
        rmb(); \
	/*while(1){\
		while(IS_LOCKED(single_global_lock)){ \
			__asm__ ("nop"); \
		} \
		if( __sync_val_compare_and_swap(&single_global_lock, 0, 1) == 0) { \
			break; \
		} \
	}*/ \
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
/* 	padded_scalar num_threads; \
	num_threads.value = global_numThread; \
	padded_scalar kill_index; \
	padded_scalar state; \
        long actions[81][81]={0}; \
        long to_save[81]={0};*/ \
        /*long state = 0; \
        long x = 0; \
        padded_scalar kill_ignored;*/ \
	kill_ignored=0; \
        padded_scalar start_wait_time; \
        READ_TIMESTAMP(start_wait_time.value); \
	/*for(kill_index=0; kill_index < num_threads; kill_index++){ \
		counters_snapshot[kill_index] = counters[kill_index].value; \
	} */\
	for(kill_index=0; kill_index < num_threads; kill_index++){ \
	/*	padded_scalar temp; \
		if(counters_snapshot[kill_index] > FINISHED){ \
			if(b_type < 3){ \
                        	temp.value = triggers[kill_index].value; /*kill large transactions*/ \
        /*        	} \
			while(counters[kill_index].value == counters_snapshot[kill_index]){ \
				cpu_relax(); \
			} \
		} \
	} */\
	    padded_scalar temp; \
            temp.value = counters[kill_index].value; \
	    if(temp.value > FINISHED) { \
                padded_scalar wait_needed; \
		wait_needed.value = temp.value - end_time.value; \
                /*printf("needed %d\n",wait_needed.value);*/ \
                if(wait_needed.value > 0) { \
                    kill_index2 = wait_needed.value/100000; /*(tx_length[0].value*2);/*alpha*/ \
/*printf("index2 %d\n",kill_index2);*/\
                    if(kill_index2 > num_threads - 1){ \
                        actions[num_threads][to_save[num_threads]++] = kill_index; \
/*printf("kill\n");*/\
                    } else { \
                        actions[kill_index2][to_save[kill_index2]++] = kill_index; \
                        counters_snapshot[kill_index] = temp.value; \
                    } \
                } else { \
                    actions[0][to_save[0]++] = kill_index; \
		    counters_snapshot[kill_index] = temp.value; \
                } \
            } else { \
                kill_ignored++; \
            } \
        } \
        kill_cansave = 0; \
        kill_acc = 0; \
	for(kill_index=0; kill_index < num_threads; kill_index++){ \
            kill_acc = kill_acc + to_save[kill_index]; \
            if(kill_acc >= kill_index) \
                kill_cansave = kill_index; \
            if(num_threads - 1 - kill_ignored - kill_acc - to_save[num_threads] < kill_index){ \
                break; \
            } \
        } \
	for(kill_index=num_threads; kill_index >= 0; kill_index--){ \
            if(kill_index > kill_cansave){ \
                for(kill_index2=0; kill_index2 < to_save[kill_index]; kill_index2++){ \
/*printf("killing\n");*/ \
                    padded_scalar temp; \
		    temp.value = triggers[actions[kill_index][kill_index2]].value; /*kill the transaction*/ \
                } \
            } else { \
                for(kill_index2=0; kill_index2 < to_save[kill_index]; kill_index2++){ \
			/*printf("%llu %llu \n",counters_snapshot[actions[kill_index][kill_index2]], counters[actions[kill_index][kill_index2]].value); \
                    /*printf("commits %d kill_index %d x %d to_save %d aa %d\n",stats_array[local_thread_id].rot_commits,kill_index, kill_index2, to_save[kill_index], actions[kill_index][kill_index2]);*/ \
                    if(counters_snapshot[actions[kill_index][kill_index2]] > FINISHED){ \
		        while(counters_snapshot[actions[kill_index][kill_index2]] == counters[actions[kill_index][kill_index2]].value){ \
                            cpu_relax(); \
			} \
                    } \
                } \
            } \
        } \
        padded_scalar end_wait_time; \
       	READ_TIMESTAMP(end_wait_time.value); \
       	stats_array[local_thread_id].wait_time += end_wait_time.value - start_wait_time.value; \
/*int i=0; while(i++ < 100000000);*/ \
};

# define QUIESCENCE_CALL_GL(){ \
        for(int kill_index=0; kill_index < global_numThread; kill_index++){ \
	    while(counters[kill_index].value != FINISHED){ \
                cpu_relax(); \
            } \
        } \
};

# define RELEASE_WRITE_LOCK(){ \
	if(local_exec_mode == 1){ \
	        __TM_suspend(); \
                padded_scalar end_time; \
                READ_TIMESTAMP(end_time.value); \
		if(local_thread_id == 0){ \
                    tx_length[b_type].value = tx_length[b_type].value*0.5 + (end_time.value - counters[local_thread_id].value + tx_length[b_type].value)*0.5; \
/*printf("after tid %d b_type %d length %d\n",local_thread_id, b_type, tx_length[b_type].value);*/ \
                } \
                counters[local_thread_id].value = INACTIVE; \
		rmb(); \
                QUIESCENCE_CALL_ROT(); \
	        __TM_resume(); \
/*		 QUIESCENCE_CALL_ROT();*/ \
		__TM_end(); \
		counters[local_thread_id].value = FINISHED; \
		/*printf("thread %d committed in ROT with counters %lu and rot_counters %d\n",local_thread_id,counters[local_thread_id].value,rot_counters[local_thread_id].value); */\
                if(ro) stats_array[local_thread_id].read_commits++; \
		else stats_array[local_thread_id].rot_commits++; \
	} \
	else{ \
		pthread_spin_unlock(&single_global_lock); \
		/*single_global_lock = 0;*/ \
		stats_array[local_thread_id].gl_commits++; \
		/*printf("thread %d committed in GL with counters %lu\n",local_thread_id,counters[local_thread_id].value); */\
	} \
	/*printf("thread %d committed\n",local_thread_id); */\
};

# define TM_BEGIN_EXT(b,ro) {  \
        num_threads = global_numThread; \
	local_exec_mode = 0; \
        b_type = b; \
	local_thread_id = SPECIAL_THREAD_ID(); \
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
