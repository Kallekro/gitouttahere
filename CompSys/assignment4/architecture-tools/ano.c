//
// (C) Finn Schiermer Andersen, 2016
//
// ano.c
//
// Annotation support.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ano.h"
#include "ano_predictor.h"
#include "ano_cache.h"
#include "ano_resource.h"
#include "ano_options.h"

#define NO_REG 16

struct inst_profile {
    unsigned long executions;
    unsigned long wait_for_ops;
    unsigned long wait_for_exe;
    unsigned long exe_latency;
    unsigned long imiss;
    unsigned long dmiss;
    unsigned long pmiss;
    int waits[8];
    const char *stages;
    char disass[30];
};

struct annotation_t {
    unsigned long ins_count;
    unsigned long long pc;

    // pretty-printing
    int trace;
    int cycles;
    char* pp;
    unsigned long last_pagebreak;
    unsigned long first_cycle;
    unsigned long last_cycle;

    // timestamps (timing model)
    int model;
    unsigned long t_regs[17]; // 16 + one dummy "no register"
    unsigned long t_cc; // condition code register
    unsigned long t_fetch;
    unsigned long t_store_visible;
    const char* stages;
    unsigned long t_stages[128];

    // configuration
    int fetch_latency;
    int predictor_taken_latency;
    int predictor_not_taken_latency;
    int decode_latency;
    int load_latency;
    int pipe_width;
    int ooo;
    int ddep_only;

    // resource model
    predictor_ptr pred;
    predictor_ptr ret_pred;
    cache_ptr icache;
    cache_ptr dcache;
    cache_ptr l2cache;
    resource_ptr ifetch;
    resource_ptr decode;
    resource_ptr rob;
    resource_ptr decode_queue;
    resource_ptr scheduler;
    resource_ptr cache_queue;
    resource_ptr retire;
    resource_ptr cache_exe;
    resource_ptr agen_exe;
    resource_ptr alu_exe;
    resource_ptr mul_exe;

    // profiling array
    struct inst_profile* profiles;
    struct inst_profile* curr_profile;
    unsigned int profile_size;
};

void profile_inst(annotation_ptr, unsigned long pc, const char* pp);
void add_stages(annotation_ptr);
void add_dmiss(annotation_ptr);
void add_imiss(annotation_ptr);
void add_pmiss(annotation_ptr);

void profile_inst(annotation_ptr ano, unsigned long pc, const char* pp) {
    if (ano->profiles) {
	while (pc >= ano->profile_size) { // grow profile size
	    struct inst_profile empty;
	    empty.executions = empty.wait_for_ops = empty.wait_for_exe = 0;
	    empty.exe_latency = empty.dmiss = empty.imiss = empty.pmiss = 0;
	    for (int i = 0; i < 8; ++i) empty.waits[i] = 0;
	    empty.disass[0] = 0;
	    unsigned int new_profile_size = ano->profile_size * 3/2;
	    struct inst_profile* new_profiles = calloc(new_profile_size, sizeof(struct inst_profile));
	    for (unsigned int i = 0; i < ano->profile_size; ++i) {
		new_profiles[i] = ano->profiles[i];
	    }
	    free(ano->profiles);
	    ano->profiles = new_profiles;
	    for (unsigned int i = ano->profile_size; i < new_profile_size; ++i) {
		ano->profiles[i] = empty;
	    }
	    ano->profile_size = new_profile_size;
	}
	ano->curr_profile = ano->profiles + pc;
	if (ano->curr_profile->executions == 0) { // copy diassembly at first use
	    strcpy(ano->curr_profile->disass, pp);
	}
	ano->curr_profile->executions++;
    }
}

void add_stages(annotation_ptr ano) {
    if (ano->profiles == NULL) return;
    const char* stage = ano->stages;
    ano->curr_profile->stages = ano->stages; // capture for printing
    if (*stage == 0) return;
    unsigned int entry_time = ano->t_stages[(int)*stage];
    int stage_number = 0;
    while (*++stage) {
	ano->curr_profile->waits[stage_number] += ano->t_stages[(int)*stage] - entry_time;
	++stage_number;
	entry_time = ano->t_stages[(int)*stage];
    }
}

void add_dmiss(annotation_ptr ano) {
    if (ano->profiles == NULL) return;
    ano->curr_profile->dmiss++;
}

void add_imiss(annotation_ptr ano) {
    if (ano->profiles == NULL) return;
    ano->curr_profile->imiss++;
}

void add_pmiss(annotation_ptr ano) {
    if (ano->profiles == NULL) return;
    ano->curr_profile->pmiss++;
}


// nice to have:
static unsigned long max(unsigned long a, unsigned long b) {
    if (a > b) return a;
    else return b;
}


annotation_ptr create_annotation(int argc, char const * argv [])
{
    int trace = has_flag("-t", argc, argv);
    int model = has_flag("-m", argc, argv);
    int cycles = has_flag("-c", argc, argv);
    int profile = has_flag("-prof", argc, argv);
    int ddep_only = has_flag("-ddep-only", argc, argv);
    model |= ddep_only;
    int annotate = model || trace || cycles || profile;
    if (!annotate) 
        return NULL;

    if (cycles) {
	model = 1; // display of cycle/time implies modelling and tracing
	trace = 1;
    }
    annotation_ptr res = malloc(sizeof(struct annotation_t));
    res->ddep_only = ddep_only;
    res->last_pagebreak = 0;
    res->first_cycle = 0;
    res->last_cycle = 0;
    res->ins_count = 0;
    res->t_fetch = 0;
    res->t_store_visible = 0;
    res->trace = trace;
    res->model = model;
    res->cycles = cycles;
    for (int j=0; j<128; ++j) res->t_stages[j] = 0;
    for (int j=0; j<16; ++j) res->t_regs[j] = 0;
    res->t_cc = 0;

    // setup configuration:
    int pipeline_width = option_with_default("-pw=%d", 1, argc, argv);
    int ooo = has_flag("-ooo", argc, argv);
    int clat = option_with_default("-clat=%d", 3, argc, argv);
    int l2lat = option_with_default("-l2lat=%d", 9, argc, argv);
    int dlat = 1;
    int mlat = 100;
    mlat = option_with_default("-mlat=%d", mlat, argc, argv);
    // default assumption: more than one inst/clock requires additional decode stage
    if (pipeline_width > 1) dlat++;
    // default assumption: same for ooo to do renaming
    if (ooo) dlat++;
    dlat = option_with_default("-dlat=%d", dlat, argc, argv);
    res->ooo = ooo;
    int cache_ports = option_with_default("-cp=%d", 1, argc, argv);
    int exe_engines = option_with_default("-xp=%d", pipeline_width, argc, argv);
    int mul_engines = option_with_default("-mp=%d", 1, argc, argv);
    res->fetch_latency = clat;
    res->predictor_taken_latency = option_with_default("-ptlat=%d", 2, argc, argv);
    res->predictor_not_taken_latency = option_with_default("-pntlat=%d", 0, argc, argv);
    res->decode_latency = dlat;
    res->load_latency = clat;
    res->pipe_width = pipeline_width;
    if (has_flag("-bpred=oracle", argc, argv))
	res->pred = create_oracle_predictor();
    else if (has_flag("-bpred=taken", argc, argv))
	res->pred = create_taken_predictor();
    else if (has_flag("-bpred=nt", argc, argv))
	res->pred = create_not_taken_predictor();
    else if (has_flag("-bpred=btfnt", argc, argv))
	res->pred = create_btfnt_predictor();
    else {
	int num = option_with_default("-bpred=local:%d", 0, argc, argv);
	if (num) res->pred = create_local_predictor(num);
	else {
	    num = option_with_default("-bpred=gshare:%d", 0, argc, argv);
	    if (num) res->pred = create_gshare_predictor(num);
	    else res->pred = create_gshare_predictor(12);
	}
    }
    res->ret_pred = create_return_predictor(4);
    // cache config
    int block_sz = option_with_default("-l2:blksz=%d", 5, argc, argv);
    int index_sz = option_with_default("-l2:idxsz=%d", 11, argc, argv);
    int assoc = option_with_default("-l2:assoc=%d", 4, argc, argv);
    res->l2cache = create_cache("L2-Cache", block_sz, index_sz, assoc, l2lat, NULL, mlat);

    block_sz = option_with_default("-dc:blksz=%d", 5, argc, argv);
    index_sz = option_with_default("-dc:idxsz=%d", 7, argc, argv);
    assoc = option_with_default("-dc:assoc=%d", 4, argc, argv);
    res->dcache = create_cache("DCache", block_sz, index_sz, assoc, res->fetch_latency, res->l2cache, 0);
    if (res->ddep_only) return res;

    // Config of more advanced models:
    block_sz = option_with_default("-ic:blksz=%d", 5, argc, argv);
    index_sz = option_with_default("-ic:idxsz=%d", 7, argc, argv);
    assoc = option_with_default("-ic:assoc=%d", 4, argc, argv);
    res->icache = create_cache("ICache", block_sz, index_sz, assoc, res->load_latency, res->l2cache, 0);
    // resource config
    int rob_size = option_with_default("-rob=%d", 128, argc, argv);
    int scheduler_size = option_with_default("-xq=%d", 64, argc, argv);
    int cache_q_size = option_with_default("-cq=%d", 32, argc, argv);
    res->rob = create_inorder_resource("ROB", rob_size, 5000);
    if (ooo) {
	res->scheduler = create_inorder_resource("scheduler", scheduler_size, 5000);
	res->cache_queue = create_inorder_resource("cache-q", cache_q_size, 5000);
	res->agen_exe = create_unordered_resource("Agen-exe", cache_ports, 5000);
	res->cache_exe = create_unordered_resource("Cache-exe", cache_ports, 5000);
	res->alu_exe = create_unordered_resource("Alu-Exe", exe_engines, 5000);
	res->mul_exe = create_unordered_resource("Mul-Exe", mul_engines, 5000);
    } else {
	res->scheduler = create_inorder_resource("scheduler", pipeline_width, 500);
	res->cache_queue = create_inorder_resource("cache-q", pipeline_width * (1+clat), 500);
	res->cache_exe = create_inorder_resource("Cache-exe", cache_ports, 500);
	res->alu_exe = create_inorder_resource("Alu-Exe", exe_engines, 500);
	res->mul_exe = create_inorder_resource("Mul-Exe", mul_engines, 500);
	res->agen_exe = res->alu_exe; // alu and agen is the same exe resource - really??
    }
    res->ifetch = create_inorder_resource("IFetch", res->pipe_width, 10);
    res->decode = create_inorder_resource("Decode", res->pipe_width, 10);
    res->retire = create_inorder_resource("Retire", res->pipe_width, 10);
    res->decode_queue = create_inorder_resource("decode-q", res->pipe_width * res->fetch_latency, 500);

    // profiling
    if (profile) {
	res->profiles = calloc(3, sizeof(struct inst_profile));
	res->profile_size = 3;
    } else {
	res->profiles = NULL;
	res->profile_size = 0;
    }
    return res;
}

void print_annotation(annotation_ptr annotation) { 
    // if we have collected a profile, print if first:
    if (annotation->profiles) {
	printf("\nExecution profile:\n\n");
	printf("addr : count  : imiss dmiss pmiss : ");
	if (annotation->model)
	    printf("active states                            : ");
	printf("disassembly\n");
	unsigned int p;
	for (p = 0; p < annotation->profile_size; ++p) {
	    struct inst_profile* pp = annotation->profiles + p;
	    if (pp->executions) {
		char im_buf[10] = "";
		if (pp->imiss)
		    sprintf(im_buf, "%2.1f", pp->imiss*100.0/pp->executions);
		char dm_buf[10] = "";
		if (pp->dmiss)
		    sprintf(dm_buf, "%2.1f", pp->dmiss*100.0/pp->executions);
		char pm_buf[10] = "";
		if (pp->pmiss)
		    sprintf(pm_buf, "%2.1f", pp->pmiss*100.0/pp->executions);
		printf("%4x : %6lu : %5s %5s %5s ", p, pp->executions, im_buf, dm_buf, pm_buf);
		if (annotation->model) {
		    printf(": %-8s ", pp->stages);
		    bool print_stage = true;
		    for (int i=0; i<8; ++i) {
			if (print_stage && pp->stages[i+1] == 0) print_stage = false;
			if (print_stage)
			    printf("%3lu ", pp->waits[i] / pp->executions);
			else
			    printf("    ");
		    }
		}
		printf(": %-30s\n", pp->disass);
	    }
	}
    }

    printf("\nCaches and predictors:\n\n");
    if (annotation->ddep_only == false) {
	print_predictor("Branch predictions:", annotation->pred);
	print_predictor("Return predictions:", annotation->ret_pred);
	print_cache(annotation->icache);
    }
    print_cache(annotation->dcache);
    print_cache(annotation->l2cache);

    if (annotation->model) {

	if (annotation->ooo) {
	    printf("\nOOO-Resources:\n\n");
	    print_resource(annotation->rob);
	    print_resource(annotation->scheduler);
	    print_resource(annotation->cache_queue);
	}
	// find timing of last value
	unsigned long t_last = 0;
	t_last = max(t_last, annotation->t_cc);
	for (int j=0; j<15; ++j)
	    t_last = max(t_last, annotation->t_regs[j]);
	printf("\nTiming info:\n\n");
	printf("    Instructions: %ld   cycles (last value): %ld\n", 
	       annotation->ins_count, t_last);
	if (t_last && annotation->ins_count) {
	    printf("    IPC: %f   CPI: %f\n",
		   annotation->ins_count*1.0/t_last,
		   t_last*1.0/annotation->ins_count);
	}
    }
}

void free_annotation(annotation_ptr annotation) { 
    free_cache(annotation->dcache);
    free_cache(annotation->l2cache);
    if (annotation->ddep_only) return;

    // free resources for more advanced models
    free_cache(annotation->icache);
    free_predictor(annotation->pred);
    free_predictor(annotation->ret_pred);
    free_resource(annotation->ifetch);
    free_resource(annotation->decode_queue);
    free_resource(annotation->decode);
    free_resource(annotation->rob);
    free_resource(annotation->scheduler);
    free_resource(annotation->cache_queue);
    free_resource(annotation->retire);
    free_resource(annotation->cache_exe);
    free_resource(annotation->alu_exe);
    free_resource(annotation->mul_exe);
    if (annotation->ooo)
	free_resource(annotation->agen_exe);
    free(annotation);
    if (annotation->profiles)
	free(annotation->profiles);
}

/*
  Pretty - Printing

  ---
  Why is it that formatting output is always so ugly?
*/
static void print_gridline(annotation_ptr annotation, int force) {
    char buffer[1000];
    if (!annotation->cycles) return;
    // everty 8th instruction we need a grid line
    if (force || ((annotation->ins_count % 10) == 0)) {
	int v = sprintf(buffer, 
			"-----( %8ld / %8ld)---------------------------------------------------", 
			annotation->ins_count + force,
			annotation->first_cycle);
	if (annotation->first_cycle + 999 < annotation->last_cycle) {
	    // buffer isn't large enough, get us out of here:
	    if (annotation->trace) printf("<<out-of-range>>\n");
	    return;
	}
	int bars = 0;
	for (unsigned long j=annotation->first_cycle; j<=annotation->last_cycle; ++j) {
	    if ((j % 10)==0) {
		buffer[v+j+bars-annotation->first_cycle] = '|';
		++bars;
	    }
	    buffer[v+j+bars-annotation->first_cycle] = '-';
	}
	buffer[annotation->last_cycle - annotation->first_cycle + v + bars] = 0;
	printf("%s\n", buffer);
    }
}

static void print_stages(annotation_ptr annotation) {

    unsigned long first_stage = -1;
    unsigned long last_stage = 0;
    if (!annotation->cycles) {
	if (annotation->trace) putchar('\n');
	annotation->ins_count++;
	return;
    }
    const char* p = annotation->stages;
    while (*p) {
	unsigned long time = annotation->t_stages[(int)*p];
	if (time < first_stage) first_stage = time;
	if (time > last_stage) last_stage = time;
	// printf("%c: %ld    ",*p, timing->t_stages[(int)*p]);
	++p;
    }
    if (first_stage == (unsigned long)-1)
	first_stage = 0;
    if (first_stage < annotation->first_cycle)
	first_stage = annotation->first_cycle;

    // last cycle printed must align to vertical grid and must be after last stage
    if (last_stage +7 > annotation->last_cycle)
	annotation->last_cycle = (last_stage + 15) & ~7;
    if (annotation->first_cycle + 999 < annotation->last_cycle) {
	// buffer isn't large enough, get us out of here:
	if (annotation->trace) putchar('\n');
	annotation->ins_count++;
	return;
    }
    // write all activity into a buffer:
    char buffer[1000];
    for (unsigned long j = annotation->first_cycle; j <= annotation->last_cycle; ++j) {
	buffer[j - annotation->first_cycle] = ' ';
    }
    buffer[annotation->last_cycle - annotation->first_cycle + 1] = 0;
    for (unsigned long j = first_stage; j <= last_stage; ++j) {
	buffer[j - annotation->first_cycle] = '.';
    }
    p = annotation->stages;
    while (*p) {
	unsigned long time = annotation->t_stages[(int)*p];
	if (time >= annotation->first_cycle)
	    buffer[time - annotation->first_cycle] = *p;
	++p;
    }	
    for (unsigned long j = annotation->first_cycle; j <= annotation->last_cycle; ++j) {
	if ((j % 10) == 0) putchar('|');
	putchar(buffer[j - annotation->first_cycle]);
    }
    putchar('\n');
    // look for a pagebreak:
    if ((annotation->ins_count % 10) == 9) {
	if ((annotation->ins_count >= 127 + annotation->last_pagebreak)
	    || ((annotation->ins_count >= 9 + annotation->last_pagebreak)
		&& (first_stage >= 10 + annotation->first_cycle)
		&& (last_stage > 60 + annotation->first_cycle))) {
	    print_gridline(annotation, 1);
	    annotation->last_pagebreak = annotation->ins_count + 1;
	    annotation->first_cycle = first_stage - (first_stage % 10); // align to vertical grid
	    printf("\n\n\n");
	}
    }
    annotation->ins_count++;
}


/*
  Capture information of relevance to pretty printing
*/

void note_insn(annotation_ptr annotation, char* pp, word_t pc) {
    annotation->pc = pc;
    annotation->pp = pp;
    profile_inst(annotation, pc, pp);
}

void note_done(annotation_ptr annotation) {
    print_gridline(annotation, 0);
    if (annotation->trace)
	printf("%-4llx %-22s    : %-45s", annotation->pc, annotation->pp, "");
    add_stages(annotation);
    print_stages(annotation);
}

void note_done_reg(annotation_ptr annotation, char* reg) {
    print_gridline(annotation, 0);
    if (annotation->trace)
	printf("%-4llx %-22s    : %-23s  %-18s  ", annotation->pc, annotation->pp, reg, "");
    add_stages(annotation);
    print_stages(annotation);
}

void note_done_cc(annotation_ptr annotation, char* cc) {
    print_gridline(annotation, 0);
    if (annotation->trace)
	printf("%-4llx %-22s    : %-23s  %-18s  ", annotation->pc, annotation->pp, cc, "");
    add_stages(annotation);
    print_stages(annotation);
}

void note_done_reg_cc(annotation_ptr annotation, char* reg, char* cc) {
    print_gridline(annotation, 0);
    if (annotation->trace)
	printf("%-4llx %-22s    : %-23s  %-18s  ", annotation->pc, annotation->pp, reg, cc);
    add_stages(annotation);
    print_stages(annotation);
}

void note_done_mem(annotation_ptr annotation, char* mem) {
    print_gridline(annotation, 0);
    if (annotation->trace)
	printf("%-4llx %-22s    : %-23s  %-18s  ", annotation->pc, annotation->pp, mem, "");
    add_stages(annotation);
    print_stages(annotation);
}

void note_done_reg_reg(annotation_ptr annotation, char* reg1, char* reg2) {
    print_gridline(annotation, 0);
    if (annotation->trace)
	printf("%-4llx %-22s    : %-23s  %-18s  ", annotation->pc, annotation->pp, reg1, reg2);
    add_stages(annotation);
    print_stages(annotation);
}

void note_done_reg_mem(annotation_ptr annotation, char* reg, char* mem) {
    print_gridline(annotation, 0);
    if (annotation->trace)
	printf("%-4llx %-22s    : %-23s  %-18s  ", annotation->pc, annotation->pp, reg, mem);
    add_stages(annotation);
    print_stages(annotation);
}

static void note_prepare(annotation_ptr annotation, const char* stages) {
    annotation->stages = stages;
    while (*stages) {
	annotation->t_stages[(int) *stages] = 0;
	++stages;
    }
}

static void note_event(annotation_ptr annotation, char event, unsigned long time) {
    annotation->t_stages[(int)event] = time;
}

static unsigned long get_event_time(annotation_ptr annotation, char event) {
    return annotation->t_stages[(int)event];
}

/*
  Timing model
*/


// pipeline frontend. Returns earliest start of exe-stage.
// model fetch + decode bw limitation.
// allocate one, two or three resource queues during decode.
// Only 'return' needs 3 allocation: one cache access and two
// ordinary exe-runs, 1 for sp modification and 1 for the actual
// change of control flow.
static unsigned long model_fetch(annotation_ptr annotation, resource_ptr q2, resource_ptr q3) {

    unsigned long start_fetch = resource_acquire(annotation->ifetch, annotation->t_fetch);
    start_fetch = resource_acquire(annotation->decode_queue, start_fetch);
    resource_use(annotation->ifetch, start_fetch, start_fetch + 1);
    note_event(annotation, 'F', start_fetch);
    annotation->t_fetch = start_fetch;
    bool triggered_miss;
    unsigned long fetch_done = cache_read(annotation->icache, start_fetch, annotation->pc, &triggered_miss);
    if (triggered_miss)
	add_imiss(annotation);
    unsigned long start_decode = resource_acquire(annotation->decode, fetch_done);
    start_decode = resource_acquire(annotation->rob, start_decode);
    start_decode = resource_acquire(annotation->scheduler, start_decode);
    if (q2)
	start_decode = resource_acquire(q2, start_decode);
    if (q3)
	start_decode = resource_acquire(q3, start_decode);
    resource_use(annotation->decode_queue, start_fetch, start_decode);
    resource_use(annotation->decode, start_decode, start_decode + 1);
    note_event(annotation, 'D', start_decode);
    unsigned long end_decode = start_decode + annotation->decode_latency;
    note_event(annotation, 'q', end_decode);
    return end_decode;
}


// some form of execution stage. Wait for earliest possible access
// to the execution resoruce, then release whichever queue was used for
// waiting on the resource
static unsigned long model_exe(char stage, annotation_ptr annotation, 
			resource_ptr queue, resource_ptr exe, 
			unsigned long t) {
    t = resource_acquire(exe, t);
    note_event(annotation, stage, t);
    // exe stages have a writeback the following cycle
    if (stage=='X')
	note_event(annotation, 'w', t+1);
    else if (stage=='M')
	note_event(annotation, 'w', t+4);
    resource_use(exe, t, t + 1);
    // We occupy a scheduler/queue/pipestage from decode til execute:
    resource_use(queue, get_event_time(annotation, 'D'), t);
    return t;
}


// pipeline backend. Model retire bw limitation. Also model
// queue usage for reorder buffer
static unsigned long model_retire(annotation_ptr annotation, unsigned long done) {
    unsigned long t_retire = resource_acquire(annotation->retire, done);
    resource_use(annotation->retire, t_retire, t_retire + 1);
    // we occupy a slot in the ROB from decode to completion of retire:
    resource_use(annotation->rob, get_event_time(annotation, 'D'), t_retire + 1);
    note_event(annotation, 'C', t_retire);
    return t_retire + 1;
}



// Timing of load and store operations - used by all instructions which
// reference memory

static unsigned long model_load(annotation_ptr annotation, reg_id_t base, 
			 unsigned long exe_start, word_t addr, int writeback_base) {

    exe_start = max(exe_start, annotation->t_regs[base]);
    note_event(annotation, 'r', exe_start);
    exe_start = max(exe_start, annotation->t_store_visible);
    exe_start = model_exe('A', annotation, annotation->scheduler, annotation->agen_exe, exe_start);
    unsigned long cache_access_start = 1 + exe_start;
    // handle side-effecting address computation
    if (writeback_base)
	annotation->t_regs[base] = 1 + exe_start;
    cache_access_start = model_exe('L', annotation, annotation->cache_queue, annotation->cache_exe,
				   cache_access_start);
    bool triggered_miss;
    unsigned long val_ready = cache_read(annotation->dcache, cache_access_start, addr, &triggered_miss);
    if (triggered_miss)
	add_dmiss(annotation);
    return val_ready;
}


static unsigned long model_store(annotation_ptr annotation, reg_id_t base, unsigned long exe_start,
			  unsigned long data_ready, word_t addr, int writeback_base) {
    exe_start = max(exe_start, annotation->t_regs[base]);
    note_event(annotation, 'r', exe_start);
    exe_start = model_exe('A', annotation, annotation->scheduler, annotation->agen_exe, exe_start);
    unsigned long cache_access_start = 1 + exe_start;
    // handle side-effecting address computation
    if (writeback_base)
	annotation->t_regs[base] = 1 + exe_start;
    annotation->t_store_visible = max(annotation->t_store_visible, cache_access_start);
    cache_access_start = max(cache_access_start, data_ready);
    cache_access_start = model_exe('S', annotation, annotation->cache_queue, annotation->cache_exe,
				   cache_access_start);
    bool triggered_miss;
    unsigned long done = cache_write(annotation->dcache, cache_access_start, addr, &triggered_miss);
    if (triggered_miss)
	add_dmiss(annotation);
    return done;
}


static unsigned long model_load_simple(annotation_ptr annotation, reg_id_t base, 
				word_t addr, int writeback_base) {

    unsigned long exe_start = annotation->t_regs[base];
    // no logically later store may access the cache before all 
    // earlier stores have computed their address:
    exe_start = max(exe_start, annotation->t_store_visible);
    note_event(annotation, 'A', exe_start);
    unsigned long cache_access_start = 1 + exe_start;
    // handle side-effecting address computation
    if (writeback_base)
	annotation->t_regs[base] = 1 + exe_start;
    note_event(annotation, 'L', cache_access_start);
    bool triggered_miss;
    unsigned long val_ready = cache_read(annotation->dcache, cache_access_start, addr, &triggered_miss);
    if (triggered_miss)
	add_dmiss(annotation);
    return val_ready;
}


static unsigned long model_store_simple(annotation_ptr annotation, reg_id_t base,
				 unsigned long data_ready, word_t addr, int writeback_base) {
    unsigned long exe_start = annotation->t_regs[base];
    note_event(annotation, 'A', exe_start);
    unsigned long cache_access_start = 1 + exe_start;
    // handle side-effecting address computation
    if (writeback_base)
	annotation->t_regs[base] = 1 + exe_start;
    // no logically later store may access the cache before all 
    // earlier stores have computed their address
    annotation->t_store_visible = max(annotation->t_store_visible, cache_access_start);
    cache_access_start = max(cache_access_start, data_ready);
    note_event(annotation, 'S', cache_access_start);
    bool triggered_miss;
    unsigned long done = cache_write(annotation->dcache, cache_access_start, addr, &triggered_miss);
    if (triggered_miss)
	add_dmiss(annotation);
    return done;
}




// Timing of individual instructions:

void model_hlt(annotation_ptr annotation) { 

    if (annotation->ddep_only) {
	note_prepare(annotation,"");
    } else {
	note_prepare(annotation,"FDqrC");
	unsigned long exe_start = model_fetch(annotation, 0, 0);
	note_event(annotation, 'r', exe_start);
	model_retire(annotation, exe_start + 1);
    }
}

void model_nop(annotation_ptr annotation) { 

    if (annotation->ddep_only) {
	note_prepare(annotation,"");
    } else {
	note_prepare(annotation,"FDqrC");
	unsigned long exe_start = model_fetch(annotation, 0, 0);
	note_event(annotation, 'r', exe_start);
	model_retire(annotation, exe_start + 1);
    }
}

void model_alu(annotation_ptr annotation, reg_id_t src, reg_id_t dest, bool writes) {

    if (annotation->ddep_only) {
	note_prepare(annotation,"Xw");
	unsigned long exe_start = max(annotation->t_regs[src], annotation->t_regs[dest]);
	note_event(annotation, 'X', exe_start);
	note_event(annotation, 'w', exe_start + 1);
	if (writes)
	    annotation->t_regs[dest] = exe_start + 1;
	annotation->t_cc = exe_start + 1;
    } else {
	note_prepare(annotation,"FDqrXwC");
	unsigned long exe_start = model_fetch(annotation, 0, 0);
	exe_start = max(exe_start, max(annotation->t_regs[src], annotation->t_regs[dest]));
	note_event(annotation, 'r', exe_start);
	exe_start = model_exe('X', annotation, annotation->scheduler, annotation->alu_exe, exe_start);
	if (writes)
	    annotation->t_regs[dest] = exe_start + 1;
	annotation->t_cc = exe_start + 1;
	model_retire(annotation, exe_start + 2);
    }
}


void model_mul(annotation_ptr annotation, reg_id_t src, reg_id_t dest) {

    if (annotation->ddep_only) {
	note_prepare(annotation,"Mw");
	unsigned long exe_start = max(annotation->t_regs[src], annotation->t_regs[dest]);
	note_event(annotation, 'M', exe_start);
	note_event(annotation, 'w', exe_start + 4);
	annotation->t_regs[dest] = exe_start + 4;
	annotation->t_cc = exe_start + 4;
    } else {
	note_prepare(annotation,"FDqrMwC");
	unsigned long exe_start = model_fetch(annotation, 0, 0);
	exe_start = max(exe_start, max(annotation->t_regs[src], annotation->t_regs[dest]));
	note_event(annotation, 'r', exe_start);
	exe_start = model_exe('M', annotation, annotation->scheduler, annotation->mul_exe, exe_start);
	annotation->t_regs[dest] = exe_start + 4;
	annotation->t_cc = exe_start + 4;
	model_retire(annotation, exe_start + 5);
    }
}


void model_ialu(annotation_ptr annotation, reg_id_t dest, bool writes) {

    model_alu(annotation, NO_REG, dest, writes);
}


void model_jmp(annotation_ptr annotation, word_t next_pc, int is_conditional, int is_taken) { 

    if (annotation->ddep_only) {
	note_prepare(annotation, "");
    } else {
	note_prepare(annotation, "FDqrJC");
	unsigned long exe_start = model_fetch(annotation, 0, 0);
	if (is_conditional)
	    exe_start = max(exe_start, annotation->t_cc);
	note_event(annotation, 'r', exe_start);
	exe_start = model_exe('J', annotation, annotation->scheduler, annotation->alu_exe, exe_start);
	int correct_prediction;
	if (is_conditional)
	    correct_prediction = train_predictor(annotation->pred, annotation->pc, next_pc, is_taken);
	else
	    correct_prediction = 1;
	if (is_taken)
	    resource_use_all(annotation->ifetch, 
			     annotation->t_fetch + annotation->predictor_taken_latency);
	else
	    resource_use_all(annotation->ifetch, 
			     annotation->t_fetch + annotation->predictor_not_taken_latency);
	if (!correct_prediction) {
	    add_pmiss(annotation);
	    annotation->t_fetch = exe_start + 1;
	}
	model_retire(annotation, exe_start + 1);
    }
}


void model_call(annotation_ptr annotation, word_t return_pc, word_t target, word_t push_addr) { 

    (void) target;
    
    if (annotation->ddep_only) {
	note_prepare(annotation,"AS");
	model_store_simple(annotation, REG_RSP, 0, push_addr, 1);
    } else {
	note_prepare(annotation, "FDqrASC");
	unsigned long exe_start = model_fetch(annotation, annotation->cache_queue, 0);
	note_call(annotation->ret_pred, return_pc);
	resource_use_all(annotation->ifetch, annotation->t_fetch + annotation->predictor_taken_latency);
	unsigned long done = model_store(annotation, REG_RSP, exe_start, exe_start, push_addr, 1);
	model_retire(annotation, done + 1);
    }
}


void model_ret(annotation_ptr annotation, word_t next_pc, word_t pop_addr) { 

    if (annotation->ddep_only) {
	note_prepare(annotation, "AL");
	model_load_simple(annotation, REG_RSP, pop_addr, 1);
    } else {
	note_prepare(annotation, "FDqrALJC");
	unsigned long exe_start = model_fetch(annotation, annotation->scheduler, annotation->cache_queue);
	unsigned long ret_start = model_load(annotation, REG_RSP, exe_start, pop_addr, 1);
	ret_start = model_exe('J', annotation, annotation->scheduler, annotation->alu_exe, ret_start);
	int predicted = predict_return(annotation->ret_pred, next_pc);
	resource_use_all(annotation->ifetch, annotation->t_fetch + annotation->predictor_taken_latency);
	if (!predicted) {
	    add_pmiss(annotation);
	    annotation->t_fetch = ret_start;
	}
	model_retire(annotation, ret_start + 1);
    }
}


void model_rrmov(annotation_ptr annotation, reg_id_t src, reg_id_t dest, int is_conditional) { 

    if (annotation->ddep_only) {
	note_prepare(annotation, "Xw");
	unsigned long exe_start = annotation->t_regs[src];
	if (is_conditional) {
	    exe_start = max(exe_start, annotation->t_cc);
	}
	note_event(annotation, 'X', exe_start);
	note_event(annotation, 'w', exe_start + 1);
	annotation->t_regs[dest] = exe_start + 1;
    } else {
	note_prepare(annotation, "FDqrXwC");
	unsigned long exe_start = model_fetch(annotation, 0, 0);
	exe_start = max(exe_start, annotation->t_regs[src]);
	if (is_conditional) {
	    exe_start = max(exe_start, annotation->t_cc);
	}
	note_event(annotation, 'r', exe_start);
	exe_start = model_exe('X', annotation, annotation->scheduler, annotation->alu_exe, exe_start);
	annotation->t_regs[dest] = exe_start + 1;
	model_retire(annotation, exe_start + 2);
    }
}


void model_irmov(annotation_ptr annotation, reg_id_t dest) { 

    if (annotation->ddep_only) {
	note_prepare(annotation, "Xw");
	unsigned long exe_start = 0;
	note_event(annotation, 'X', exe_start);
	note_event(annotation, 'w', exe_start + 1);
	annotation->t_regs[dest] = exe_start + 1;	
    } else {
	note_prepare(annotation, "FDqrXwC");
	unsigned long exe_start = model_fetch(annotation, 0, 0);
	note_event(annotation, 'r', exe_start);
	exe_start = model_exe('X', annotation, annotation->scheduler, annotation->alu_exe, exe_start);
	annotation->t_regs[dest] = exe_start + 1;
	model_retire(annotation, exe_start + 2);
    }
}


void model_mrmov(annotation_ptr annotation, reg_id_t base, reg_id_t dest, word_t addr) { 

    if (annotation->ddep_only) {
	note_prepare(annotation, "ALw");
	unsigned long val_ready = model_load_simple(annotation, base, addr, 0);
	note_event(annotation, 'w', val_ready);
	annotation->t_regs[dest] = val_ready;
    } else {
	note_prepare(annotation, "FDqrALwC");
	unsigned long exe_start = model_fetch(annotation, annotation->cache_queue, 0);
	unsigned long val_ready = model_load(annotation, base, exe_start, addr, 0);
	note_event(annotation, 'w', val_ready);
	annotation->t_regs[dest] = val_ready;
	model_retire(annotation, val_ready + 1);
    }
}


void model_rmmov(annotation_ptr annotation, reg_id_t base, reg_id_t src, word_t addr) { 

    if (annotation->ddep_only) {
	note_prepare(annotation, "AS");
	model_store_simple(annotation, base, annotation->t_regs[src], addr, 0);
    } else {
	note_prepare(annotation, "FDqrASC");
	unsigned long exe_start = model_fetch(annotation, annotation->cache_queue, 0);
	unsigned long done = model_store(annotation, base, exe_start, annotation->t_regs[src], addr, 0);
	model_retire(annotation, done + 1);
    }
}


void model_push(annotation_ptr annotation, reg_id_t src, word_t addr) { 

    if (annotation->ddep_only) {
	note_prepare(annotation, "AS");
	model_store_simple(annotation, REG_RSP, annotation->t_regs[src], addr, 1);
    } else {
	note_prepare(annotation, "FDqrASC");
	unsigned long exe_start = model_fetch(annotation, annotation->cache_queue, 0);
	unsigned long done = model_store(annotation, REG_RSP, exe_start, annotation->t_regs[src], addr, 1);
	model_retire(annotation, done + 1);
    }
}


void model_pop(annotation_ptr annotation, reg_id_t dest, word_t addr) { 

    if (annotation->ddep_only) {
	note_prepare(annotation, "ALw");
	unsigned long val_ready = model_load_simple(annotation, REG_RSP, addr, 1);
	note_event(annotation, 'w', val_ready);
	annotation->t_regs[dest] = val_ready;
    } else {
	note_prepare(annotation, "FDqrALwC");
	unsigned long exe_start = model_fetch(annotation, annotation->cache_queue, 0);
	unsigned long val_ready = model_load(annotation, REG_RSP, exe_start, addr, 1);
	note_event(annotation, 'w', val_ready);
	annotation->t_regs[dest] = val_ready;
	model_retire(annotation, val_ready + 1);
    }
}

void annotation_usage()
{
    printf("Modelling options\n");
    printf("  -prof                 collect and display profiling information for each instruction\n");
    printf("  -t                    trace: print disassembly and result of each instruction\n");
    printf("  -m                    model resource use and estimate timing\n");
    printf("  -c                    print cycle diagram for each instruction (implies -m and -t)\n");
    printf("  -ddep-only            model only data dependencies, ignore actual resource needs\n");
    printf("  -diff                 end simulation by printing registers and changed memory cells\n");

    printf("Pipeline delays\n");
    printf("  -dlat=<cycles>        latency of decoder (default 1, 2 or 3 dependent on other options)\n");
    printf("  -clat=<cycles>        latency of cache read (default 3)\n");
    printf("  -l2lat=<cycles>       additional latency of L2 (shared) cache access (default 9)\n");
    printf("  -mlat=<cycle>         additional latency of main memory access (default 100)\n");
    printf("  -ptlat=<cycles>       latency of prediction if flow change (default 2)\n");
    printf("  -pntlat=<cycles>      latency of prediction if no flow change (default 0)\n");

    printf("Cache configuration\n");
    printf("  -ic:blksz=<bits>      select number of bits for indexing i-cache block (default 5)\n");
    printf("  -ic:idxsz=<bits>      select number of bits in i-cache index (default 7)\n");
    printf("  -ic:assoc=<num>       select i-cache associativity (default 4)\n");
    printf("  -dc:blksz=<bits>      select number of bits for indexing d-cache block (default 5)\n");
    printf("  -dc:idxsz=<bits>      select number of bits in d-cache index (default 7)\n");
    printf("  -dc:assoc=<num>       select d-cache associativity (default 4)\n");
    printf("  -l2:blksz=<bits>      select number of bits for indexing L2-cache block (default 5)\n");
    printf("  -l2:idxsz=<bits>      select number of bits in L2-cache index (default 11)\n");
    printf("  -l2:assoc=<num>       select L2-cache associativity (default 4)\n");

    printf("Predictor configuration\n");
    printf("  -bpred=oracle         select oracle predictor (no mispredictions)\n");
    printf("  -bpred=taken          select taken predictor\n");
    printf("  -bpred=nt             select not taken predictor\n");
    printf("  -bpred=btfnt          select backward-taken, forward-not-taken predictor\n");
    printf("  -bpred=gshare:<size>  select gshare predictor of given size (default 12 bits)\n");
    printf("  -bpred=local:<size>   select local predictor of given size\n");

    printf("Superscalar resource configuration\n");
    printf("  -pw=<width>           width of pipeline (default 1)\n");
    printf("  -cp=<ports>           number of cache ports (default: 1)\n");
    printf("  -xp=<ports>           number of agen/alu ports (=units) (default: half of -pw +1)\n");
    printf("  -mp=<ports>           number of multiplier ports (=units) (default: 1)\n");

    printf("Out-of-order pipeline configuration\n");
    printf("  -ooo                  enable out-of-order scheduling\n");
    printf("  -rob=<size>           give size of reorder buffer (default 128)\n");
    printf("  -cq=<size>            give size of queue/scheduler for cache access (default 32)\n");
    printf("  -xq=<size>            give size of queue/scheduler for execute stage (default 64)\n");
}
