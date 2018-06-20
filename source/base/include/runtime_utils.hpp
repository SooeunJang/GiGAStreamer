#pragma once

#include <signal.h>
#include <map>
#include <vector>
#include <fcntl.h>
#include "log.hpp"

#define DAFAULT_CONNECTION_TIMEOUT 1 //sec

namespace cfx {

	struct MyMemoryInfo
	{
		unsigned long vsize;	/**< byte */
		long rss;	/**< page count */
		// TODO: region malloc?
	};
	struct ProcessMemoryInfo
	{
	    int pid;
	    char name[32];
	    unsigned long vsize;	/**< byte */
	    long rss;	/**< page count */
	};
	enum STACK_DISPLAY_OPTION
	{
	    STACK_DISPLAY_DIFF,
	    STACK_DISPLAY_ALL,
	    STACK_DISPLAY_NONE,
	};
	struct SystemMonitorConfiguration
	{
	    int resource_monitoring_interval_sec;	/**< if 0, stop monitoring */
	    bool monitor_system_cpu;
	    bool monitor_thread_cpu;
	    bool monitor_all_process_cpu;	/**< not implmented */
	    bool monitor_system_memory;
	    bool monitor_process_memory;
	    bool monitor_all_process_memory; /**< not implemented */
	    bool monitor_stack;
	    float cpu_display_threshold_percent; /**< print threads or other processes whose cpu usage is over this value */
	    int process_memory_display_threshold_kb; /**< print other processes whose memory usage(vsize) is over this value */
	    STACK_DISPLAY_OPTION stack_display;

	    /**
	    @brief constructor with default options
	    */
	    SystemMonitorConfiguration(void) :
	        resource_monitoring_interval_sec(30), monitor_system_cpu(true), monitor_thread_cpu(true),
	        monitor_all_process_cpu(true), monitor_system_memory(true), monitor_process_memory(true),
	        monitor_all_process_memory(false), monitor_stack(false), cpu_display_threshold_percent(1.0),
	        process_memory_display_threshold_kb(15*1024), stack_display(STACK_DISPLAY_DIFF)
	    {
	    }
	};

	/**
	 *
	 * @struct
	 * @brief
	 */
	struct ThreadStackInfo
	{
	    char name[32];
	    int stack_size;	/**< 0 means unknown */
	    int untouched_stack_size;


	    /**
	     *
	     */
	    ThreadStackInfo(void)
	    {
	        memset(this, 0, sizeof(ThreadStackInfo));
	    }
	};
	enum STACK_CHECK_TYPE
	{
	    STACK_CHECK_UNKNOWN,
	    STACK_CHECK_FROM_MAPS,
	    STACK_CHECK_FROM_SUBSCHEDULER,
	};
	struct SystemCpuTickInfo
	{
		unsigned long user;
		unsigned long nice;
		unsigned long sys;
		unsigned long idle;
		unsigned long iowait;
		unsigned long irq;
		unsigned long sirq;
		//unsigned long steal;
		//unsigned long guest;
		//unsigned long guest_nice;
		unsigned long total;

		SystemCpuTickInfo operator-(const SystemCpuTickInfo& other)
		{
		    SystemCpuTickInfo answer;

		    answer.user = user - other.user;
		    answer.nice = nice - other.nice;
		    answer.sys = sys - other.sys;
		    answer.idle = idle - other.idle;
		    answer.iowait = iowait - other.iowait;
		    answer.irq = irq - other.irq;
		    answer.sirq = sirq - other.sirq;
		    answer.total = total - other.total;

		    return answer;
		}
	};
	struct TaskCpuTickInfo
	{
	    int id;	/**< tid or pid */
	    char name[32];
	    unsigned long stime;
	    unsigned long utime;
	    unsigned long cstime;
	    unsigned long cutime;
	    unsigned long total;
	    unsigned long system_timetick; /**< system time from /proc/stat */

	    TaskCpuTickInfo operator-(const TaskCpuTickInfo& other)
	    {
	        TaskCpuTickInfo answer;

	        answer.id = id;
	        memcpy(answer.name, name, sizeof(name));
	        answer.stime = stime - other.stime;
	        answer.utime = utime - other.utime;
	        answer.cstime = cstime - other.cstime;
	        answer.cutime = cutime - other.cutime;
	        answer.total = total - other.total;
	        answer.system_timetick = system_timetick - other.system_timetick;

	        return answer;
	    }
	};

	inline int fscanf(FILE *file, const char *fmt, ...)
	{
	    va_list ap;
	    int ret;
	    va_start(ap, fmt);
	    ret = vfscanf(file, fmt, ap);
	    va_end(ap);
	    return ret;
	}
	inline int sscanf(const char *buf, const char *fmt, ...)
	{
	    // TODO: buffer overflow check
	    va_list ap;
	    int ret;
	    va_start(ap, fmt);
	    ret = vsscanf(buf, fmt, ap);
	    va_end(ap);
	    return ret;
	}
    class RuntimeUtils {
    public:
    	RuntimeUtils(const SystemMonitorConfiguration* conf, bool backtrace_on_crash) : config(), stack_check_type(STACK_CHECK_UNKNOWN)
		{
    	    if (backtrace_on_crash)
    	    {
    	        struct sigaction sa;
    	        memset(&sa, 0, sizeof(struct sigaction));
    	        sigfillset(&sa.sa_mask);
    	        sa.sa_flags = SA_SIGINFO;
    	        sa.sa_sigaction = on_badSignal;
    	        sigaction(SIGILL, &sa, NULL);
    	        sigaction(SIGABRT, &sa, NULL);	// NOTE: stack trace is not working well after abort
    	        sigaction(SIGFPE, &sa, NULL);
    	        sigaction(SIGSEGV, &sa, NULL);
    	        sigaction(SIGBUS, &sa, NULL);
    	        sigaction(SIGUSR1, &sa, NULL);
    	    }
    	    if (conf != NULL)
    	    {
    	        memcpy(&config, conf, sizeof(SystemMonitorConfiguration));
    	    }
    	    memset(&last_system_cputick, 0, sizeof(SystemCpuTickInfo));
    	    memset(&system_cputick_diff, 0, sizeof(SystemCpuTickInfo));
    	    memset(&my_memory_info, 0, sizeof(MyMemoryInfo));
    	    pid = getpid();
		}

        ~RuntimeUtils(void)
        {

        }
        /**
        @brief initialize. this function will be called after creating threads
        */
        void initialize_monitoring(void);

        /**
        @brief check if all subschedulers are working fine
        @todo if the blocked/killed thread is for log or database, system log will not be recorded
        */
        void monitor_threadHealth(void);

        /**
        @brief run monitoring with current configuration
        */
        void monitor_systemResource(void);

		void check_cpu(void);
		void check_systemCpu(void);
		void update_taskCpuInfo(std::map<int, TaskCpuTickInfo>& last_info, std::vector<TaskCpuTickInfo>& diff, const char* proc_path, bool trace_add_delete);
		void check_systemMemory(void);
		void check_thisProcessMemory(void);
		void check_otherProcessMemory(int display_threshold);
		void check_stack(void);
		void check_stack_from_maps(void);
//		void check_stack_from_scheduler(void);

//		static void print_subSchedulerRunningInfo(pthread_t thread_id);
		static void on_badSignal(int signo, siginfo_t *info, void *secret);
		static void print_ARMSignalContext(struct ucontext* ctx);
		static void raise_segFault(void);
		static void print_callStackBacktrace(const char* msg);
    private:
        SystemMonitorConfiguration config;

        SystemCpuTickInfo last_system_cputick;
        SystemCpuTickInfo system_cputick_diff;

        std::map<int, TaskCpuTickInfo> last_thread_cputick; /**< key is tid */
        std::vector<TaskCpuTickInfo> thread_cputick_diff;

        std::map<int, TaskCpuTickInfo> last_process_cputick; /**< key is pid */
        std::vector<TaskCpuTickInfo> process_cputick_diff;

        MyMemoryInfo my_memory_info;

        std::vector<ProcessMemoryInfo> system_memory_info;

        std::map<int, ThreadStackInfo> stack_info;	/**< key is tid */

        STACK_CHECK_TYPE stack_check_type;
        int pid;
    };
}
