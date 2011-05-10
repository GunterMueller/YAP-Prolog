/************************************************************************
**                                                                     **
**                   The YapTab/YapOr/OPTYap systems                   **
**                                                                     **
** YapTab extends the Yap Prolog engine to support sequential tabling  **
** YapOr extends the Yap Prolog engine to support or-parallelism       **
** OPTYap extends the Yap Prolog engine to support or-parallel tabling **
**                                                                     **
**                                                                     **
**      Yap Prolog was developed at University of Porto, Portugal      **
**                                                                     **
************************************************************************/

/***********************
**      Includes      **
***********************/

#include "Yap.h"
#if defined(YAPOR) || defined(TABLING)
#define OPT_MAVAR_STATIC
#include "Yatom.h"
#include "YapHeap.h"
#include <unistd.h>
#include <signal.h>
#ifdef YAPOR
#include "or.macros.h"
#endif	/* YAPOR */
#ifdef TABLING
#include "tab.macros.h"
#elif !defined(YAPOR_COW)
#include "opt.mavar.h"
#endif /* TABLING */
#ifdef YAPOR_COW
#include "sys/wait.h"
#endif /* YAPOR_COW */



/*********************
**      Macros      **
*********************/

#ifdef USE_PAGES_MALLOC
#define STRUCTS_PER_PAGE(STR_TYPE)  ((Yap_page_size - STRUCT_SIZE(struct page_header)) / STRUCT_SIZE(STR_TYPE))

#define INIT_PAGES(PG, STR_TYPE)                         \
        INIT_LOCK(Pg_lock(PG));                          \
        Pg_pg_alloc(PG) = 0;                             \
        Pg_str_in_use(PG) = 0;                           \
        Pg_str_per_pg(PG) = STRUCTS_PER_PAGE(STR_TYPE);  \
        Pg_free_pg(PG) = NULL
#else
#define INIT_PAGES(PG, STR_TYPE)  Pg_str_in_use(PG) = 0
#endif /* USE_PAGES_MALLOC */



/*******************************
**      Global functions      **
*******************************/

void Yap_init_global_optyap_data(int max_table_size, int n_workers, int sch_loop, int delay_load) {
  int i;

  /* global data related to memory management */
#ifdef LIMIT_TABLING
  if (max_table_size)
    GLOBAL_max_pages = ((max_table_size - 1) * 1024 * 1024 / SHMMAX + 1) * SHMMAX / Yap_page_size;
  else
    GLOBAL_max_pages = -1;
#endif /* LIMIT_TABLING */
  INIT_PAGES(GLOBAL_pages_void, void *);      
#ifdef YAPOR
  INIT_PAGES(GLOBAL_pages_or_fr , struct or_frame);
  INIT_PAGES(GLOBAL_pages_qg_sol_fr , struct query_goal_solution_frame);
  INIT_PAGES(GLOBAL_pages_qg_ans_fr, struct query_goal_answer_frame);
#endif /* YAPOR */
#ifdef TABLING_INNER_CUTS
  INIT_PAGES(GLOBAL_pages_tg_sol_fr, struct table_subgoal_solution_frame);
  INIT_PAGES(GLOBAL_pages_tg_ans_fr, struct table_subgoal_answer_frame);
#endif /* TABLING_INNER_CUTS */
#ifdef TABLING
  INIT_PAGES(GLOBAL_pages_tab_ent, struct table_entry);
  INIT_PAGES(GLOBAL_pages_sg_fr, struct subgoal_frame);
  INIT_PAGES(GLOBAL_pages_dep_fr, struct dependency_frame);
  INIT_PAGES(GLOBAL_pages_sg_node, struct subgoal_trie_node);
  INIT_PAGES(GLOBAL_pages_ans_node, struct answer_trie_node);
  INIT_PAGES(GLOBAL_pages_gt_node, struct global_trie_node);
  INIT_PAGES(GLOBAL_pages_sg_hash, struct subgoal_trie_hash);
  INIT_PAGES(GLOBAL_pages_ans_hash, struct answer_trie_hash);
  INIT_PAGES(GLOBAL_pages_gt_hash, struct global_trie_hash);
#endif /* TABLING */
#if defined(YAPOR) && defined(TABLING)
  INIT_PAGES(GLOBAL_pages_susp_fr, struct suspension_frame);
#endif /* YAPOR && TABLING */

#ifdef YAPOR
  /* global static data */
  GLOBAL_number_workers= n_workers;
  GLOBAL_worker_pid(0) = getpid();
  for (i = 1; i < GLOBAL_number_workers; i++) GLOBAL_worker_pid(i) = 0;
  GLOBAL_scheduler_loop = sch_loop;
  GLOBAL_delayed_release_load = delay_load;

  /* global data related to or-performance */
  GLOBAL_number_goals = 0;
  GLOBAL_best_times(0) = 0;
  GLOBAL_performance_mode = PERFORMANCE_OFF;

  /* global data related to or-parallelism */
  ALLOC_OR_FRAME(GLOBAL_root_or_fr);
  BITMAP_clear(GLOBAL_bm_present_workers);
  for (i = 0; i < GLOBAL_number_workers; i++) 
    BITMAP_insert(GLOBAL_bm_present_workers, i);
  BITMAP_copy(GLOBAL_bm_idle_workers, GLOBAL_bm_present_workers);
  BITMAP_clear(GLOBAL_bm_root_cp_workers);
  BITMAP_clear(GLOBAL_bm_invisible_workers);
  BITMAP_clear(GLOBAL_bm_requestable_workers);
  BITMAP_clear(GLOBAL_bm_executing_workers);
  BITMAP_copy(GLOBAL_bm_finished_workers, GLOBAL_bm_present_workers);
  INIT_LOCK(GLOBAL_locks_bm_idle_workers);
  INIT_LOCK(GLOBAL_locks_bm_root_cp_workers);
  INIT_LOCK(GLOBAL_locks_bm_invisible_workers);
  INIT_LOCK(GLOBAL_locks_bm_requestable_workers);
  INIT_LOCK(GLOBAL_locks_bm_executing_workers);
  INIT_LOCK(GLOBAL_locks_bm_finished_workers);
#ifdef TABLING_INNER_CUTS
  INIT_LOCK(GLOBAL_locks_bm_pruning_workers);
#endif /* TABLING_INNER_CUTS */
  GLOBAL_locks_who_locked_heap = MAX_WORKERS;
  INIT_LOCK(GLOBAL_locks_heap_access);
  INIT_LOCK(GLOBAL_locks_alloc_block);
  if (GLOBAL_number_workers== 1)
    GLOBAL_parallel_execution_mode = FALSE;
  else
    GLOBAL_parallel_execution_mode = TRUE;
#endif /* YAPOR */

#ifdef TABLING
  /* global data related to tabling */
  new_global_trie_node(GLOBAL_root_gt, 0, NULL, NULL, NULL);
  GLOBAL_root_tab_ent = NULL;
#ifdef LIMIT_TABLING
  GLOBAL_first_sg_fr = NULL;
  GLOBAL_last_sg_fr = NULL;
  GLOBAL_check_sg_fr = NULL;
#endif /* LIMIT_TABLING */
  new_dependency_frame(GLOBAL_root_dep_fr, FALSE, NULL, NULL, NULL, NULL, NULL);
  for (i = 0; i < MAX_TABLE_VARS; i++) {
    CELL *pt = GLOBAL_table_var_enumerator_addr(i);
    RESET_VARIABLE(pt);
  }
#ifdef TABLE_LOCK_AT_WRITE_LEVEL
  for (i = 0; i < TABLE_LOCK_BUCKETS; i++)
    INIT_LOCK(GLOBAL_table_lock(i));
#endif /* TABLE_LOCK_AT_WRITE_LEVEL */
#endif /* TABLING */

  return;
}


void Yap_init_local_optyap_data(int wid) {
#ifdef YAPOR
  CACHE_REGS
  /* local data related to or-parallelism */
  Set_REMOTE_top_cp(wid, (choiceptr) Yap_LocalBase);
  REMOTE_top_or_fr(wid) = GLOBAL_root_or_fr;
  REMOTE_load(wid) = 0;
  REMOTE_share_request(wid) = MAX_WORKERS;
  REMOTE_reply_signal(wid) = worker_ready;
#ifdef YAPOR_COPY
  INIT_LOCK(REMOTE_lock_signals(wid));
#endif /* YAPOR_COPY */
  Set_REMOTE_prune_request(wid, NULL);
#endif /* YAPOR */
  INIT_LOCK(REMOTE_lock(wid));
#ifdef TABLING
  /* local data related to tabling */
  REMOTE_next_free_ans_node(wid) = NULL;
  REMOTE_top_sg_fr(wid) = NULL; 
  REMOTE_top_dep_fr(wid) = GLOBAL_root_dep_fr; 
#ifdef YAPOR
  Set_REMOTE_top_cp_on_stack(wid, (choiceptr) Yap_LocalBase); /* ??? */
  REMOTE_top_susp_or_fr(wid) = GLOBAL_root_or_fr;
#endif /* YAPOR */
#endif /* TABLING */
  return;
}


void Yap_init_root_frames(void) {
  CACHE_REGS

#ifdef YAPOR
  /* root or frame */
  or_fr_ptr or_fr = GLOBAL_root_or_fr;
  INIT_LOCK(OrFr_lock(or_fr));
  OrFr_alternative(or_fr) = NULL;
  BITMAP_copy(OrFr_members(or_fr), GLOBAL_bm_present_workers);
  SetOrFr_node(or_fr, (choiceptr) Yap_LocalBase);
  OrFr_nearest_livenode(or_fr) = NULL;
  OrFr_depth(or_fr) = 0;
  Set_OrFr_pend_prune_cp(or_fr, NULL);
  OrFr_nearest_leftnode(or_fr) = or_fr;
  OrFr_qg_solutions(or_fr) = NULL;
#ifdef TABLING_INNER_CUTS
  OrFr_tg_solutions(or_fr) = NULL;
#endif /* TABLING_INNER_CUTS */
#ifdef TABLING
  OrFr_owners(or_fr) = GLOBAL_number_workers;
  OrFr_next_on_stack(or_fr) = NULL;
  OrFr_suspensions(or_fr) = NULL;
  OrFr_nearest_suspnode(or_fr) = or_fr;
#endif /* TABLING */
  OrFr_next(or_fr) = NULL;
#endif /* YAPOR */

#ifdef TABLING
  /* root dependency frame */
  DepFr_cons_cp(GLOBAL_root_dep_fr) = B;
#endif /* TABLING */
}


void itos(int i, char *s) {
  int n,r,j;
  n = 10;
  while (n <= i) n *= 10;
  j = 0;
  while (n > 1) {
    n = n / 10;   
    r = i / n;
    i = i - r * n;
    s[j++] = r + '0';
  }
  s[j] = 0;
  return;
}
#endif /* YAPOR || TABLING */
