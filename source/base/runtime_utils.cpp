#include "runtime_utils.hpp"
#include <execinfo.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/ucontext.h>
#include <dirent.h>
#include <fcntl.h>
#include <sstream>
#include <cxxabi.h>

using namespace cfx;

#define ERROR_FILENAME "./logs/[GLSM]_CallstackTrace.log"

void RuntimeUtils::initialize_monitoring(void)
 {
     char proc_task_path[64];

     check_systemCpu();	// system total time tick is required for thread and process cpu check
     update_taskCpuInfo(last_process_cputick, process_cputick_diff, "/proc", false);
     snprintf(proc_task_path, 64, "/proc/%d/task", pid);
     update_taskCpuInfo(last_thread_cputick, thread_cputick_diff, proc_task_path, false);

//     if (UtilityWrapper::system_cmd("grep stack: /proc/%d/maps -q", getpid()) == 0)
//     {
//         stack_check_type = STACK_CHECK_FROM_MAPS;
//     }
//     else
//     {
//         stack_check_type = STACK_CHECK_FROM_SUBSCHEDULER;
//     }
//     const TaskDirectory::ThreadList& thread_list = TaskDirectory::get_threadlist();
//     TaskDirectory::ThreadList::const_iterator itr;
//     for (itr = thread_list.begin(); itr != thread_list.end(); ++itr)
//     {
//         ComponentSubScheduler* sub = *itr;
//         ThreadStackInfo info;
//         strlcpy(info.name, sub->get_id().c_str(), sizeof(info.name));
//         info.stack_size = sub->get_stackSize();
//         stack_info.insert(std::make_pair(sub->get_osThreadId(), info));
//     }
 }

 /**
 @brief check if all subschedulers are working fine
 @todo if the blocked/killed thread is for log or database, system log will not be recorded
 */
 void RuntimeUtils::monitor_threadHealth(void)
 {
//     typedef TaskDirectory::ThreadList::const_iterator Iterator;
//     const TaskDirectory::ThreadList& thread_list = TaskDirectory::get_threadlist();
/*
     int now = UtilityWrapper::get_milisecond();

     for (Iterator it = thread_list.begin(), end = thread_list.end(); it != end; ++it)
     {
         ComponentSubScheduler* sub = *it;
         if (pthread_kill(sub->get_threadId(), 0) == ESRCH)
         {
             LOG_ERR("thread stopped");
             std::string logmsg = "STOPPED ";
             logmsg += get_subSchedulerRunningInfo(sub);
             // TODO: leave a system log (SYS_POWREBOOT, logmsg.c_str())
             sleep(1);
             raise_segFault();
         }

         int block_time = now - sub->get_lastRunningTime();

         if (block_time > 10000)
         {
             int block_limit = sub->get_blockTimeoutSeconds();
             if(block_limit != 0)
             {
                 LOG_ERR("thread %s blocked for %dms (limit %dsec)", sub->get_id().c_str(), block_time, block_limit);
                 if (block_time > block_limit * 1000 - THREAD_MONITORING_INTERVAL * 2)
                 {
                     pthread_kill(sub->get_threadId(), SIGUSR1);
                     if (block_time > block_limit * 1000)
                     {
                         LOG_ERR("WATCHDOG TIMEOUT...");

                         std::string logmsg = "BLOCKED ";
                         logmsg += get_subSchedulerRunningInfo(sub);
                         for (Iterator it2 = thread_list.begin(); it2 != end; ++it2)
                         {
                             if (it != it2 && now - (*it2)->get_lastRunningTime() > 30000)
                             {
                                 logmsg += get_subSchedulerRunningInfo(*it2);
                                 UtilityWrapper::wait_miliSecond(50);
                                 pthread_kill((*it2)->get_threadId(), SIGUSR1);
                             }
                         }
                         // TODO: leave a system log (SYS_POWREBOOT, logmsg.c_str())
                         raise_segFault();
                     }
                 }
             }
             else
             {
 //                LOG_ERR("%s is being Skipped watchdog timeout!!", sub->get_id().c_str());
             }
         }
     }*/
 }

 /**
 @brief run monitoring with current configuration
 */
 void RuntimeUtils::monitor_systemResource(void)
 {
     if (config.monitor_system_cpu || config.monitor_thread_cpu || config.monitor_all_process_cpu)
     {
         check_cpu();
     }
     if (config.monitor_system_memory)
     {
         check_systemMemory();
     }
     if (config.monitor_process_memory)
     {
         check_thisProcessMemory();
     }
     if (config.monitor_all_process_memory)
     {
         check_otherProcessMemory(config.process_memory_display_threshold_kb);
     }
     if (config.monitor_stack)
     {
         check_stack();
     }
 }

 /**
 @brief check cpu usage of all processes or threads from /proc
 @param[in,out] last_info - map of pid/tid and tickinfo. this value will be updated after calculating \a diff
 @param[out] diff - tick changes from last call
 @param proc_path - directory path which contains task info ("/proc" or "/proc/<pid>/task")
 @param trace_add_delete - if true, print the change of process list
 @param display_threshold - print threads whose cpu usage is over this value(%)
 */
 void RuntimeUtils::update_taskCpuInfo(std::map<int, TaskCpuTickInfo>& last_info, std::vector<TaskCpuTickInfo>& diff, const char* proc_path, bool trace_add_delete)
 {
     TaskCpuTickInfo info;
     char stat_filename[64];
     FILE* fp;
     DIR* dir;
     struct dirent* entry;
     const char* type = strcmp(proc_path, "/proc")? "thread" : "process";

     typedef std::map<int, TaskCpuTickInfo>::iterator Iterator;

     diff.clear();

     dir = opendir(proc_path);
     if (dir == NULL)
     {
         return;
     }
     while ((entry = readdir(dir)) != NULL)
     {
         if (entry->d_type != DT_DIR || entry->d_name[0] < '0' || entry->d_name[0] > '9')
         {
             continue;
         }

         snprintf(stat_filename, 64, "%s/%s/stat", proc_path, entry->d_name);
         fp = fopen(stat_filename, "r");
         if (fp == NULL)
         {
             continue;
         }

         cfx::fscanf(fp, "%*d %32s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %lu %lu",
                                info.name, &info.utime, &info.stime, &info.cutime, &info.cstime);
         fclose(fp);

         info.id = atoi(entry->d_name);
         info.total = info.utime + info.stime + info.cutime + info.cstime;
         info.system_timetick = last_system_cputick.total;

         Iterator it = last_info.find(info.id);
         if (it == last_info.end())
         {
             if (trace_add_delete)
             {
                 LOG_NOTI("new %s %d%s", type, info.id, info.name);
             }
             diff.push_back(info);
             last_info.insert(std::make_pair(info.id, info));
         }
         else
         {
             diff.push_back(info - it->second);
             memcpy(&it->second, &info, sizeof(TaskCpuTickInfo));
         }
     }
     closedir(dir);

     for (Iterator it = last_info.begin(); it != last_info.end(); )
     {
         TaskCpuTickInfo& threadinfo = it->second;
         if (threadinfo.system_timetick != last_system_cputick.total)
         {
             if (trace_add_delete)
             {
                 LOG_ERR("%s %d%s terminated", type, threadinfo.id, threadinfo.name);
             }
             last_info.erase(it++);
         }
         else
         {
             ++it;
         }
     }
 }

 /**
 @brief check cpu usage.
        because process time tick increases faster than system time tick, process/thread cpu is calculated as:
        (task_cpu_time / sum_of_all_task_cpu_time) * (busy_system_cpu_time / total_system_cpu_time)
 */
 void RuntimeUtils::check_cpu(void)
 {
     check_systemCpu();	// system total time tick is required for thread and process cpu check

     if (config.monitor_all_process_cpu == false && config.monitor_thread_cpu == false)
     {
         return;
     }
     if (system_cputick_diff.total == 0)
     {
         LOG_ERR("crazy system clock diff 0");
         return;
     }

     typedef std::vector<TaskCpuTickInfo>::iterator Itr;
     unsigned long all_task_cpu = 0;

     update_taskCpuInfo(last_process_cputick, process_cputick_diff, "/proc", false);
     for (Itr it = process_cputick_diff.begin(); it != process_cputick_diff.end(); ++it)
     {
         all_task_cpu += it->total;
     }
     if (all_task_cpu == 0)
     {
         LOG_ERR("no task in /proc?");
         return;
     }
     unsigned long busy_cpu = system_cputick_diff.total - system_cputick_diff.idle - system_cputick_diff.iowait;
     float ratio = 100.0 / all_task_cpu * busy_cpu / system_cputick_diff.total;	// usage(%) = ratio * time tick
     unsigned long threshold = config.cpu_display_threshold_percent / ratio;

     if (config.monitor_all_process_cpu)
     {
         for (Itr it = process_cputick_diff.begin(); it != process_cputick_diff.end(); ++it)
         {
             if (it->total > threshold)
             {
                 LOG_NOTI("process %d%s CPU %.1f%% (usr %.1f sys %.1f)",
                        it->id, it->name, it->total * ratio, it->utime * ratio, it->stime * ratio);
             }
         }
     }
     if (config.monitor_thread_cpu)
     {
         char proc_task_path[64];
         snprintf(proc_task_path, 64, "/proc/%d/task", pid);
         update_taskCpuInfo(last_thread_cputick, thread_cputick_diff, proc_task_path, false);

         for (Itr it = thread_cputick_diff.begin(); it != thread_cputick_diff.end(); ++it)
         {
             if (it->total > threshold)
             {
                 LOG_NOTI("thread %d%s CPU %.1f%% (usr %.1f sys %.1f)",
                        it->id, it->name, it->total * ratio, it->utime * ratio, it->stime * ratio);
             }
         }
     }
 }

 void RuntimeUtils::check_systemCpu(void)
 {
     SystemCpuTickInfo info;

     FILE* fp = fopen("/proc/stat", "r");
     if (fp)
     {
     	cfx::fscanf(fp, "%*s %lu %lu %lu %lu %lu %lu %lu",
                                &info.user, &info.nice, &info.sys, &info.idle,
                                &info.iowait, &info.irq, &info.sirq);
         fclose(fp);

         info.total = info.user + info.nice + info.sys + info.idle + info.iowait + info.irq + info.sirq;

         system_cputick_diff = info - last_system_cputick;
         last_system_cputick = info;

         float base = system_cputick_diff.total / 100.0;

#define PERCENT(field) (system_cputick_diff.field / base)
                if (config.monitor_system_cpu)
                {
             LOG_NOTI("Total CPU %.1f%% (usr %.1f nice %.1f sys %.1f iowait %.1f irq %.1f sirq %.1f)",
                    100 - PERCENT(idle), PERCENT(user), PERCENT(nice), PERCENT(sys),
                    PERCENT(iowait), PERCENT(irq), PERCENT(sirq));
                }
#undef PERCENT
     }
 }

 /**
 @brief check and show memory status of system
 */
 void RuntimeUtils::check_systemMemory(void)
 {
     int total, free, buffer, cached;

     FILE *fp = fopen("/proc/meminfo", "r");
     if (fp == NULL)
     {
         return;
     }
     cfx::fscanf(fp, "%*s %d %*s", &total);
     cfx::fscanf(fp, "%*s %d %*s", &free);
     cfx::fscanf(fp, "%*s %d %*s", &buffer);
     cfx::fscanf(fp, "%*s %d %*s", &cached);
     fclose(fp);

     LOG_NOTI("System Memory: Total %dk, Free %dk, Cached %dk, Buffer %dk", total, free, cached, buffer);
 }

 /**
 @brief check and show memory status of this process
 */
 void RuntimeUtils::check_thisProcessMemory(void)
 {
     char filename[64];
     snprintf(filename, 64, "/proc/%d/stat", getpid());

     FILE* fp = fopen(filename, "r");
     if (fp == NULL)
     {
         return;
     }
     cfx::fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*u %lu %ld",
                            &my_memory_info.vsize, &my_memory_info.rss);
     fclose(fp);

     LOG_NOTI("mainServer VM: size %luk, RSS %ldk", my_memory_info.vsize / 1024, my_memory_info.rss * 4);

     // TODO: need to check region malloc usage?
 }

 /**
 @brief check and show memory status of all processes
 @param display_threshold
 @todo remove redundancy: this function reads exactly same files with check_processCpu()
 */
 void RuntimeUtils::check_otherProcessMemory(int display_threshold)
 {
     char stat_filename[64];

     FILE* fp;
     DIR* dir;
     struct dirent* entry;

     ProcessMemoryInfo info;

     system_memory_info.clear();

     dir = opendir("/proc");
     if (dir == NULL)
     {
         return;
     }
     while ((entry = readdir(dir)) != NULL)
     {
         if (entry->d_type != DT_DIR || entry->d_name[0] < '0' || entry->d_name[0] > '9')
         {
             continue;
         }

         snprintf(stat_filename, 64, "/proc/%s/stat", entry->d_name);
         fp = fopen(stat_filename, "r");
         if (fp == NULL)
         {
             continue;
         }

         info.pid = atoi(entry->d_name);
         cfx::fscanf(fp, "%*d %32s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*u %lu %ld",
                                info.name, &info.vsize, &info.rss);
         fclose(fp);


         if ((int) (info.vsize / 1024) > display_threshold)
         {
             LOG_NOTI("%s(%d) VM: size %luk, RSS %ldk", info.name, info.pid, info.vsize / 1024, info.rss * 4);
         }
         system_memory_info.push_back(info);
     }
     closedir(dir);
 }
 /**
 @brief check stack usage of each thread
 */
 void RuntimeUtils::check_stack(void)
 {
     if (stack_check_type == STACK_CHECK_FROM_MAPS)
     {
         check_stack_from_maps();
     }
 }

 /**
 @brief check thread stack usage from /proc/maps file. avaliable since Linux 3.4
 */
 void RuntimeUtils::check_stack_from_maps(void)
 {
     const int PAGE_SIZE = 4096;
     const int PAGE_INFO_SIZE = 8;

     char pagemap_filename[64];
     snprintf(pagemap_filename, 64, "/proc/%d/pagemap", getpid());
     FILE* pagemap_fp = fopen(pagemap_filename, "rb");
     if (pagemap_fp == NULL)
     {
         return;
     }

     char maps_filename[64];
     snprintf(maps_filename, 64, "/proc/%d/maps", getpid());
     FILE* maps_fp = fopen(maps_filename, "r");
     if (maps_fp == NULL)
     {
         fclose(pagemap_fp);
         return;
     }

     char line[128];
     while (fgets(line, sizeof(line), maps_fp))
     {
         unsigned long start;
         unsigned long end;
         int tid;
         if (cfx::sscanf(line, "%lx-%lx %*[^\[]\[stack:%d", &start, &end, &tid) == 3)
         {
             int untouched_page_count = 0;
             if (fseek(pagemap_fp, start / PAGE_SIZE * PAGE_INFO_SIZE, SEEK_SET) < 0)
             {
                 LOG_ERR("%s fseek %lu error", pagemap_filename, start / PAGE_SIZE * PAGE_INFO_SIZE);
                 break;
             }
             for (long long s = 0; s == 0; untouched_page_count++)
             {
                 if (fread(&s, PAGE_INFO_SIZE, 1, pagemap_fp) != 1)
                 {
                     LOG_ERR("%s fread error", pagemap_filename);
                     break;
                 }
             }
             int untouched_stack_size = untouched_page_count * 4096;
             ThreadStackInfo& info = stack_info[tid];
             if (config.stack_display == STACK_DISPLAY_ALL)
             {
                 LOG_NOTI("%d(%s) stack margin %d / %d", tid, info.name, untouched_stack_size, info.stack_size);
             }
             else if (config.stack_display == STACK_DISPLAY_DIFF &&
                      info.untouched_stack_size != untouched_stack_size)
             {
                 LOG_NOTI("%d(%s) stack margin %d -> %d / %d", tid, info.name,
                        info.untouched_stack_size, untouched_stack_size, info.stack_size);
             }
             info.untouched_stack_size = untouched_stack_size;
         }
     }

     fclose(pagemap_fp);
     fclose(maps_fp);
 }


 /**
 @brief Print stack backtrace information.
        Usually this function will show instruction addresses,
        which can be translated using addr2line command or map file from nm
 @param msg - Description of the backtrace (can be NULL)
 @note backtrace_symbols() may fail when the heap memory is corrupted. backtrace_symbols_fd() can be an alternative
       recommended addr2line option is: $(CROSS_COMPILE)-addr2line -Cifspe <NOT_STRIPPED_EXEC_FILE> <ADDRESSES>
 @see http://www.linuxjournal.com/article/6391
 */
 void RuntimeUtils::print_callStackBacktrace(const char* msg)
 {
     const int STACKBUF_SIZE = 128;
     void* stackbuf[STACKBUF_SIZE];
     int stackcnt;
     char** strs;
     int fd = open(ERROR_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);

     stackcnt = backtrace(stackbuf, STACKBUF_SIZE);
     if(stackcnt == 0)
     {
         LOG_ERR( "Backtrace Stack is empty");
         return;
     }
     strs = backtrace_symbols(stackbuf, stackcnt);
     backtrace_symbols_fd(stackbuf, stackcnt, fd);
     close(fd);

     size_t funcnamesize = 256;
     char* funcname = new char[funcnamesize];

//     LOG_ERR( "Backtrace %s:\n", msg? msg : "");
     std::stringstream ss;
     for (int i = 1; i < stackcnt; i++)	// skip the first backtrace, which will show this function
     {
         char* begin_name = 0;
         char* begin_offset = 0;
         char* end_offset = 0;
         char* begin_address = 0;
         char* end_address = 0;
         for(char* p = strs[i]; *p; ++p)
         {
             if(*p == '(')
             {
                 begin_name = p;
             }
             else if(*p == '+')
             {
                 begin_offset = p;
             }
             else if(*p == ')' && begin_offset)
             {
                 end_offset = p;
             }
             else if(*p == '[')
             {
                 begin_address = p;
             }
             else if(*p == ']' && begin_address)
             {
                 end_address = p;
                 break;
             }
         }

         if(begin_name && begin_offset && end_offset && begin_name < begin_offset
                 && begin_address && end_address)
         {
             *begin_name++ = '\0';
             *begin_offset++ = '\0';
             *end_offset = '\0';
             *begin_address++ = '\0';
             *end_address = '\0';

             // mangled name is now in [begin_name, begin_offset) and caller
             // offset in [begin_offset, end_offset). now apply
             // __cxa_demangle():

             int status;
//             std::stringstream ss;
             char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
             if(status == 0)
             {
 //        		funcname = ret;
                 ss << "[backtrace]: (" << i << ") " << strs[i] << " : "
                    << ret << "+" << begin_offset <<end_offset << " [" << begin_address << "]"
                    << std::endl;
             }
             else
             {
                 // demangling failed. Output function name as a C function with
                 // no arguments.
                 ss << "[backtrace]: (" << i << ") " << strs[i] << " : "
                    << begin_name << "+" << begin_offset << end_offset << " [" << begin_address << "]"
                    << std::endl;
             }
//             LOG_ERR( "%s", ss.str().c_str());
         }
         else
         {
             // couldn't parse the line? print the whole line.
             LOG_ERR( "%s\n", strs[i]);
         }
     }
     LOG_ERR( "%s", ss.str().c_str());
     delete [] funcname;
     free(strs);
 }

void RuntimeUtils::on_badSignal(int signo, siginfo_t *info, void *secret)
{

#if 0
    LOG_ERR("Signal %d received(%s, si_code %d, si_ptr %p)", signo, strsignal(signo), info->si_code, info->si_ptr);
#else

    /**
     *
     * @struct
     * @brief
     */
    typedef struct _sig_ucontext
    {
        unsigned long uc_flags;
        struct ucontext * uc_link;
        stack_t uc_stack;
        struct sigcontext uc_mcontext;
        sigset_t uc_sigmask;
    } sig_ucontext_t;

    void* caller_address = 0;
    sig_ucontext_t* uc = 0;
    uc = (sig_ucontext_t*) secret;

    /* Get the address at the time the signal was raised */
#if defined(__i386__) // gcc specific
    caller_address = (void *) uc->uc_mcontext.eip; // EIP: x86 specific
#elif defined(__x86_64__) // gcc specific
    caller_address = (void *) uc->uc_mcontext.rip; // RIP: x86_64 specific
#elif defined(__arm__)
    caller_address = (void *) uc->uc_mcontext.arm_pc; // PC : arm specific
#else
    //#error Unsupported architecture. // TODO: Add support for other arch.
#endif
    LOG_ERR("signal %d (%s), address is %p from %p\n", signo, strsignal(signo), info->si_addr, (void*)caller_address);

#endif

//    print_ARMSignalContext((struct ucontext*) secret);

    print_callStackBacktrace(NULL);

    if (signo != SIGUSR1)
    {
        // for core dump (I'm not sure, but someone said that you can get a better core dump file without this signal handler, by disabling backtrace in the constructor)
        raise_segFault();
    }
}

/**
@brief print the arm cpu context when on_systemCrash() was called
@param ctx - context information
@todo ctx->uc_regspace contains the information of coprocessor registers
@see linux/arch/arm/kernel/signal.c
*/
void RuntimeUtils::print_ARMSignalContext(struct ucontext* ctx)
{
	/*
    LOG_ERR("trap_no: %08lx, error_code: %08lx, oldmask: %08lx",
           ctx->uc_mcontext.trap_no, ctx->uc_mcontext.error_code, ctx->uc_mcontext.oldmask);
    LOG_ERR("r0: %08lx, r1: %08lx, r2: %08lx, r3: %08lx, r4: %08lx, r5: %08lx",
           ctx->uc_mcontext.arm_r0, ctx->uc_mcontext.arm_r1, ctx->uc_mcontext.arm_r2,
           ctx->uc_mcontext.arm_r3, ctx->uc_mcontext.arm_r4, ctx->uc_mcontext.arm_r5);
    LOG_ERR("r6: %08lx, r7: %08lx, r8: %08lx, r9: %08lx, r10: %08lx",
           ctx->uc_mcontext.arm_r6, ctx->uc_mcontext.arm_r7, ctx->uc_mcontext.arm_r8,
           ctx->uc_mcontext.arm_r9, ctx->uc_mcontext.arm_r10);
    LOG_ERR("fp: %08lx, ip: %08lx, sp: %08lx",
           ctx->uc_mcontext.arm_fp, ctx->uc_mcontext.arm_ip, ctx->uc_mcontext.arm_sp);
    LOG_ERR("cpsr: %08lx, fault address: %08lx",
           ctx->uc_mcontext.arm_cpsr, ctx->uc_mcontext.fault_address);

    void* stack[2];
    stack[0] = (void*) ctx->uc_mcontext.arm_lr;
    stack[1] = (void*) ctx->uc_mcontext.arm_pc;
    char** strs = backtrace_symbols(stack, 2);
    LOG_ERR("lr: %s, pc: %s\n", strs[0], strs[1]);
    free(strs);
    */
}

/**
@brief raise a segmentation fault so that a core dump file can be generated
*/
void RuntimeUtils::raise_segFault(void)
{
    sleep(1);	// separate application error messages from kernel/driver messages
    signal(SIGSEGV, SIG_DFL);
    *(int*)0x03 = 0x03;
    sleep(1);
    LOG_ERR("still alive?");
    exit(-1);
}
